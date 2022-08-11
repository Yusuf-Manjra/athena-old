/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileRecUtils/TileCellBuilder.h"
#include "TileRecUtils/TileRawChannelBuilder.h"
#include "TileEvent/TileMutableRawChannelContainer.h"
#include "TileEvent/TileCell.h"
#include "TileEvent/TileCellCollection.h"
#include "TileEvent/TileRawChannel.h"
#include "TileEvent/TileDQstatus.h"
#include "CaloIdentifier/TileID.h"
#include "CaloIdentifier/TileTBID.h"
#include "TileIdentifier/TileHWID.h"
#include "CaloDetDescr/MbtsDetDescrManager.h"
#include "TileDetDescr/TileDetDescrManager.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileConditions/TileInfo.h"
 
// Calo includes
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloConditions/CaloAffectedRegionInfoVec.h"
#include "Identifier/IdentifierHash.h"

// Atlas includes
#include "AthAllocators/DataPool.h"
#include "EventContainers/SelectAllObject.h" 
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "CLHEP/Units/SystemOfUnits.h"
using CLHEP::MeV;
 
// uncomment line below for debug output
// #define ALLOW_DEBUG_COUT 1

static const InterfaceID IID_ITileCellBuilder("TileCellBuilder", 1, 0); 	 

const InterfaceID& TileCellBuilder::interfaceID( ) {
  return IID_ITileCellBuilder; 	 
}

//Constructor
TileCellBuilder::TileCellBuilder(const std::string& type, const std::string& name,
    const IInterface* parent)
  : base_class(type, name, parent)
  , m_eneForTimeCut(35. * MeV) // keep time only for cells above 70 MeV (more than 35 MeV in at least one PMT to be precise)
  , m_eneForTimeCutMBTS(0.03675) // the same cut for MBTS, but in pC, corresponds to 3 ADC counts or 35 MeV
  , m_qualityCut(254) // cut on overflow in quality (if quality is 255 - assume that channel is bad)
  , m_eThreshold(-100000.)
  , m_maxTimeDiff(100000.)
  , m_maxTime (100000.)
  , m_minTime(-100000.)
  , m_maxChi2(100000.)
  , m_minChi2(-100000.)
  , m_thresholdNotSet(true)
  , m_fullSizeCont(true)
  , m_maskBadChannels(true)
  , m_fakeCrackCells(false)
  , m_tileID(nullptr)
  , m_tileTBID(nullptr)
  , m_tileHWID(nullptr)
  , m_cabling(nullptr)
  , m_tileMgr(nullptr)
  , m_mbtsMgr(nullptr)
  , m_notUpgradeCabling(true)
  , m_run2(false)
  , m_tileInfo(0)
  , m_run2plus(false)
{
  declareInterface<TileCellBuilder>( this );

  //memset(m_drawerRunStatus, 0, sizeof(m_drawerRunStatus));
  //memset(m_eventErrorCounter, 0, sizeof(m_eventErrorCounter));

  // never set energy to zero, but set it to some small number
  // this will help TopoCluster to assign proper weight to the cell if needed
  m_zeroEnergy = 0.5 * MeV; // half a MeV in both PMTs i.e. one MeV in a cell

  //!< normal channel energy threshold for masking
  m_minEneChan[0] = -5000. * MeV;
  //!< gap channel energy threshold for masking
  m_minEneChan[1] = -10000. * MeV;
  //!< MBTS channel energy threshold for masking (not used currently(
  m_minEneChan[2] = -999999. * MeV;

  declareProperty( "MinEnergyChan", m_minEneChan[0]);
  declareProperty( "MinEnergyGap",  m_minEneChan[1]);
  declareProperty( "MinEnergyMBTS", m_minEneChan[2]);

  // Energy threshold in MeV that the Cell must exceed to be considered:
  declareProperty("EThreshold",m_eThreshold);

  // Maximum difference between times of two PMTs in cell:
  declareProperty("MaxTimeDiff", m_maxTimeDiff);

  // Maximum and minimum time for a cell to be included:
  declareProperty("MaxTime", m_maxTime);
  declareProperty("MinTime", m_minTime);

  // Maximum and Minimum fit quality for cell to be considered:
  declareProperty("MaxChi2", m_maxChi2);
  declareProperty("MinChi2", m_minChi2);

  declareProperty("fullSizeCont", m_fullSizeCont);

  // put zero energy in bad channels and recover from single-channel failure using second PMT is a cell
  declareProperty("maskBadChannels", m_maskBadChannels);

  // create fake E3/E4 crack scintillators with zero energy when they do not exist 
  declareProperty("fakeCrackCells", m_fakeCrackCells);

  // PMT energy will be set to this value if channel is bad
  declareProperty("BadChannelZeroEnergy", m_zeroEnergy);
  // PMT with energy above cut will preserve time info in ESD
  declareProperty("EneForTimeCut", m_eneForTimeCut);
  declareProperty("EneForTimeCutMBTS", m_eneForTimeCutMBTS);
  // PMT with quality greater than this cut will be masked
  declareProperty("QualityCut", m_qualityCut);

  // apply time correction taking numbers from CondDB (if not yet done in OF)
  declareProperty("correctTime", m_correctTime = false);

  // apply parabolic amplitude correction (if not yet done in OF without iterations)
  declareProperty("correctAmplitude", m_correctAmplitude = false);

  // use parabolic amplitude correction for OF2 or OF1 method
  declareProperty("OF2", m_of2 = true);

  // merge DSP results with offline reco results
  declareProperty("mergeChannels", m_mergeChannels = true);

  // thresholds for parabolic amplitude correction 
  declareProperty("AmpMinForAmpCorrection", m_ampMinThresh = 15.0);
  declareProperty("TimeMinForAmpCorrection", m_timeMinThresh = -12.5);
  declareProperty("TimeMaxForAmpCorrection", m_timeMaxThresh = 12.5);

  declareProperty("SkipGain", m_skipGain = -1); // never skip any gain by default

  declareProperty("UseDemoCabling", m_useDemoCabling = 0); // if set to 2015 - assume TB 2015 cabling

  declareProperty("TileInfoName", m_infoName = "TileInfo");

  declareProperty("CheckDCS", m_checkDCS = false);
}

/**
 * Destructor
 */
TileCellBuilder::~TileCellBuilder(){ 
}

/**
 * Initializer
 */
StatusCode TileCellBuilder::initialize() {

  // retrieve MBTS and Tile detector manager, TileID helper and TileIfno from det store
  if (m_MBTSContainerKey.key().empty()) {
    m_mbtsMgr = nullptr;
  } else {

    ATH_CHECK( m_MBTSContainerKey.initialize() );
    ATH_MSG_INFO( "Storing MBTS cells in " << m_MBTSContainerKey.key() );
    
    if (detStore()->retrieve(m_mbtsMgr).isFailure()) {
      ATH_MSG_WARNING( "Unable to retrieve MbtsDetDescrManager from DetectorStore" );
      m_mbtsMgr = nullptr;
    }
  }

  ATH_CHECK( m_eventInfoKey.initialize() );
  ATH_CHECK( m_DQstatusKey.initialize() );
  ATH_CHECK( m_EventInfoTileStatusKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_tileMgr) );
  ATH_CHECK( detStore()->retrieve(m_tileID) );
  ATH_CHECK( detStore()->retrieve(m_tileTBID) );
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  //=== get TileBadChanTool
  ATH_CHECK( m_tileBadChanTool.retrieve() );

  // access tools and store them
  ATH_CHECK( m_noiseFilterTools.retrieve() );

  //=== get TileCondToolEmscale
  ATH_CHECK( m_tileToolEmscale.retrieve() );

  //=== get TileCondToolTiming
  ATH_CHECK( m_tileToolTiming.retrieve() );

  CHECK( m_tileDCS.retrieve(EnableTool{m_checkDCS}) );

  ATH_CHECK( m_cablingSvc.retrieve() );
  m_cabling = m_cablingSvc->cablingService();

  reset(true, false);

  m_run2 = m_cabling->isRun2Cabling();
  m_run2plus = m_cabling->isRun2PlusCabling();

  if (m_run2 && !m_E4prContainerKey.key().empty()) {
    ATH_CHECK( m_E4prContainerKey.initialize() );
    ATH_MSG_INFO( "Storing E4'  cells in " << m_E4prContainerKey.key() );
  } else {
    m_E4prContainerKey = ""; // no E4' container for RUN1
  }

  if (m_cabling->getCablingType() == TileCablingService::UpgradeABC) {
    m_towerE1 = E1_TOWER_UPGRADE_ABC;
    m_notUpgradeCabling = false;
  }

  ATH_CHECK( m_rawChannelContainerKey.initialize() );

  if (m_mergeChannels && (m_dspRawChannelContainerKey.key() == m_rawChannelContainerKey.key()
                          || m_dspRawChannelContainerKey.key().empty())) {
      m_mergeChannels = false;
  }

  ATH_CHECK( m_dspRawChannelContainerKey.initialize(m_mergeChannels) );
  ATH_CHECK( m_eventInfoKey.initialize() );

  ATH_MSG_INFO( "TileCellBuilder initialization completed" );

  //=== get TileInfo
  ATH_CHECK( detStore()->retrieve(m_tileInfo, m_infoName) );
  m_ADCmaskValueMinusEps = m_tileInfo->ADCmaskValue() - 0.01;  // indicates channels which were masked in background dataset
  m_ADCmaskValuePlusEps  = m_tileInfo->ADCmaskValue() + 0.01;  // indicates channels which were masked in background dataset

  return StatusCode::SUCCESS;
}

void TileCellBuilder::reset(bool /* fullSizeCont */, bool printReset) {

  if (printReset) ATH_MSG_INFO( "Resetting options in " << name() );
  
  // check if any threshold was set in jobOptions
  m_thresholdNotSet = ((fabs(m_eThreshold + 100000.) < 1)
                       && (fabs(m_maxTimeDiff - 100000.) < 1)
                       && (fabs(m_maxTime - 100000.) < 1)
                       && (fabs(m_minTime + 100000.) < 1)
                       && (fabs(m_maxChi2 - 100000.) < 1)
                       && (fabs(m_minChi2 + 100000.) < 1));

  if (m_thresholdNotSet) {
    ATH_MSG_INFO( "none of thresholds set, all RawChannels will be converted to Cells");
  } else {
    ATH_MSG_INFO( "Ene threshold " << m_eThreshold << " MeV" );
    ATH_MSG_INFO( "max time diff " << m_maxTimeDiff << " ns" );
    ATH_MSG_INFO( "max time thr  " << m_maxTime << " ns" );
    ATH_MSG_INFO( "min time thr  " << m_minTime << " ns" );
    ATH_MSG_INFO( "max qual thr  " << m_maxChi2 );
    ATH_MSG_INFO( "min qual thr  " << m_minChi2 );
  }
  
  // prepare empty vector for all cell pointers
  m_fullSizeCont = true;
  
  ATH_MSG_INFO( "taking RawChannels from '" << m_rawChannelContainerKey.key() << "'" );
}

StatusCode TileCellBuilder::finalize() {

  ATH_MSG_INFO( "Finalizing" );

  return StatusCode::SUCCESS;
}

StatusCode TileCellBuilder::process (CaloCellContainer* theCellContainer,
                                     const EventContext& ctx) const
{
  //**
  //* Get TileRawChannels
  //**

  TileDrawerEvtStatusArray drawerEvtStatus;

  SG::ReadHandle<TileRawChannelContainer> rawChannelContainer(m_rawChannelContainerKey, ctx);

  if (!rawChannelContainer.isValid()) {
    ATH_MSG_WARNING( " Could not find container " << m_rawChannelContainerKey.key() );
    ATH_MSG_WARNING( " do not fill CaloCellContainer " );

  } else {
    
    ATH_MSG_DEBUG( "Container " << m_rawChannelContainerKey.key() << " with TileRawChannels found ");


    VecParams params;
    params.m_RChType = rawChannelContainer->get_type();
    params.m_RChUnit = rawChannelContainer->get_unit();
    params.m_correctAmplitude = m_correctAmplitude;
    params.m_correctTime = m_correctTime;
    params.m_of2 = m_of2;
    unsigned int bsflags = rawChannelContainer->get_bsflags();
    if (params.m_correctAmplitude || params.m_correctTime) {
      int DataType = (bsflags & 0x30000000) >> 28;
      if (DataType < 3) { // real data
        bool of2 = ((bsflags & 0x4000000) != 0);
        if (of2 != params.m_of2) {
          params.m_of2 = of2;
          ATH_MSG_WARNING( "OF2 flag in data is " << ((params.m_of2)?"True":"False"));
        }
        params.m_maxTimeCorr = 63.9375; // 64-1/16 ns is hard limit in DSP
        if (params.m_correctAmplitude && ((bsflags & 0x3000000) != 0)) {
          ATH_MSG_WARNING( "Using results of Opt filter with interations from DSP, disabling amplitude correction" );
          params.m_correctAmplitude = false;
        }
        if (params.m_correctTime && ((bsflags & 0x3000000) == 0)) {
          ATH_MSG_WARNING( "Using results of Opt filter without interations from DSP, disabling time correction" );
          params.m_correctTime = false;
        }
      } else {
        params.m_maxTimeCorr = ((bsflags >> 27) & 1) ? 100.0 : 75.0; // 100 or 75 ns is the limit for 9 or 7 samples
        if (params.m_correctAmplitude && ((bsflags & 0x6000) != 0)) {
          ATH_MSG_WARNING( "Amplitude correction was done already in optimal filter, disabling it here" );
          params.m_correctAmplitude = false;
        }
        if (params.m_correctTime && ((bsflags & 0x9000) != 0)) {
          ATH_MSG_WARNING( "Time correction was done already in optimal filter or best phase is used, disabling it here" );
          params.m_correctTime = false;
        }
      }
    }

    std::unique_ptr<TileCellContainer> MBTSCells;
    if (!m_MBTSContainerKey.key().empty()) {
      MBTSCells = std::make_unique<TileCellContainer>(SG::VIEW_ELEMENTS);
    }

    std::unique_ptr<TileCellContainer> E4prCells;
    if (!m_E4prContainerKey.key().empty()) {
      E4prCells = std::make_unique<TileCellContainer>(SG::VIEW_ELEMENTS);
    }

    SelectAllObject<TileRawChannelContainer> selAll(rawChannelContainer.cptr());
    SelectAllObject<TileRawChannelContainer>::const_iterator begin = selAll.begin();
    SelectAllObject<TileRawChannelContainer>::const_iterator end = selAll.end();

    if (m_mergeChannels
        && m_dspRawChannelContainerKey.key() != m_rawChannelContainerKey.key()
        && !m_dspRawChannelContainerKey.key().empty()) {

      ATH_MSG_DEBUG( "Merging " << m_rawChannelContainerKey.key()
                     << " and " << m_dspRawChannelContainerKey.key() );

      SG::ReadHandle<TileRawChannelContainer> dspRawChannelContainer(m_dspRawChannelContainerKey, ctx);

      if (!dspRawChannelContainer.isValid()) {
        // no DSP channels, build cells from primary container
        ATH_MSG_DEBUG( " No " << m_dspRawChannelContainerKey.key() << " found, nothing to merge " );

      } else {

        const TileRawChannelContainer* dspContainer = dspRawChannelContainer.cptr();
        std::unique_ptr<TileMutableRawChannelContainer> copiedDspContainer;

        if (m_noiseFilterTools.size() > 0) {
          ATH_MSG_DEBUG( " Running noise filter on " << m_dspRawChannelContainerKey.key()
                         << " (i.e. on second container only) " );

          // apply noise filter on dsp container before merging it with offline container
          copiedDspContainer = std::make_unique<TileMutableRawChannelContainer> (*dspContainer);
          ATH_CHECK( copiedDspContainer->status() );
          dspContainer = copiedDspContainer.get();

          for (const ToolHandle<ITileRawChannelTool>& noiseFilterTool : m_noiseFilterTools) {
            ATH_CHECK( noiseFilterTool->process(*copiedDspContainer, ctx) );
          }
        }
        
        TileFragHash::TYPE dspType = dspContainer->get_type();
        TileRawChannelUnit::UNIT dspUnit = dspContainer->get_unit();
        unsigned int dspFlags = dspContainer->get_bsflags();
        int DataType = (dspFlags & 0x30000000) >> 28;
        float dspTimeCut = params.m_maxTimeCorr;
        bool dspCorrectAmplitude = false, dspCorrectTime = false, dspOf2 = true;
        if (DataType < 3) { // real data
          dspOf2 = ((dspFlags & 0x4000000) != 0);
          if (dspOf2 != params.m_of2) ATH_MSG_DEBUG( "OF2 flag in DSPcontainer is " << ((dspOf2)?"True":"False"));
          dspTimeCut = 63.9375; // 64-1/16 ns is hard limit in DSP
          dspCorrectAmplitude = ((dspFlags & 0x3000000) == 0);
          dspCorrectTime = ((dspFlags & 0x3000000) != 0);
        } else { // dsp container contains results of offline reco
          dspTimeCut = ((dspFlags >> 27) & 1) ? 100.0 : 75.0; // 100 or 75 ns is the limit for 9 or 7 samples
        }

        SelectAllObject<TileRawChannelContainer> selAllDsp(dspContainer);
        SelectAllObject<TileRawChannelContainer>::const_iterator beginDsp = selAllDsp.begin();
        SelectAllObject<TileRawChannelContainer>::const_iterator endDsp = selAllDsp.end();

        std::vector<const TileRawChannel *> oflVec;
        std::vector<const TileRawChannel *> dspVec;

        SelectAllObject<TileRawChannelContainer>::const_iterator oflItr = begin;
        SelectAllObject<TileRawChannelContainer>::const_iterator dspItr = beginDsp;
        
        if (oflItr != end) {
          const TileRawChannel *p1 = (*oflItr);
          HWIdentifier id1 = p1->adc_HWID();

          for (; dspItr != endDsp; ++dspItr) {

            const TileRawChannel *p2 = (*dspItr);
            HWIdentifier id2 = p2->adc_HWID();

            if (id2 < id1) {
              dspVec.push_back(p2);
            } else if (id2 == id1) {
              oflVec.push_back(p1);
              ++oflItr;
              if (oflItr != end) {
                p1 = (*oflItr);
                id1 = p1->adc_HWID();
              } else {
                ++dspItr;
                break;
              }
            } else {
              while (id2 >= id1) {
                oflVec.push_back(p1);
                ++oflItr;
                if (oflItr != end) {
                  p1 = (*oflItr);
                  bool id2gtid1 = (id2 > id1);
                  id1 = p1->adc_HWID();
                  if (id2gtid1 && (id2 < id1)) {
                    dspVec.push_back(p2); // id2 is in the gap between old and new id1 - keep it
                  }
                } else {
                  if (id2 == id1) ++dspItr;
                  break;
                }
              }
              if (id2 >= id1) break;
            }
          }
          // copy all remaining channels 
          for (; oflItr != end; ++oflItr) {
            oflVec.push_back(*oflItr);
          }
        }
        for (; dspItr != endDsp; ++dspItr) {
          dspVec.push_back(*dspItr);
        }

        VecParams params1 = params;
        VecParams params2;
        params2.m_RChType = dspType;
        params2.m_RChUnit = dspUnit;
        params2.m_maxTimeCorr = dspTimeCut;
        params2.m_correctAmplitude = dspCorrectAmplitude;
        params2.m_correctTime = dspCorrectTime;
        params2.m_of2 = dspOf2;

        // build here with special iterator over 2 vectors
        DoubleVectorIterator<std::vector<const TileRawChannel *>, const TileRawChannel *> vecBeg(
            params,
            &oflVec, params1,
            &dspVec, params2, 0);
        DoubleVectorIterator<std::vector<const TileRawChannel *>, const TileRawChannel *> vecEnd(
            params,
            &oflVec, params1,
            &dspVec, params2, 2);

        ATH_MSG_DEBUG("Build raw channels from two vectors:"
                      << " offline vector size = " << oflVec.size()
                      << ", dsp vector size = " << dspVec.size() );

        build (ctx, drawerEvtStatus, params, vecBeg, vecEnd, theCellContainer,
               MBTSCells.get(), E4prCells.get());
        begin = end;
      }

    }

    if (begin != end) { // no merging applied, use original raw channel container

      std::unique_ptr<TileMutableRawChannelContainer> copiedContainer;
      std::unique_ptr<SelectAllObject<TileRawChannelContainer> > selCopied;

      if (m_noiseFilterTools.size() > 0) {
        ATH_MSG_DEBUG( " Running noise filter on " << m_rawChannelContainerKey.key() );
        // apply noise filter on input container before sending it to the build() method
        copiedContainer = std::make_unique<TileMutableRawChannelContainer> (*rawChannelContainer);
        ATH_CHECK( copiedContainer->status() );

        for (const ToolHandle<ITileRawChannelTool>& noiseFilterTool : m_noiseFilterTools)
        {
          ATH_CHECK( noiseFilterTool->process(*copiedContainer, ctx) );
        }

        selCopied = std::make_unique<SelectAllObject<TileRawChannelContainer> > (copiedContainer.get());
        begin = selCopied->begin();
        end = selCopied->end();
      }
      
      ATH_MSG_DEBUG( " Calling build() method for rawChannels from " << m_rawChannelContainerKey.key() );
      build (ctx, drawerEvtStatus, params, begin, end, theCellContainer,
             MBTSCells.get(), E4prCells.get());
    }
    
    if (!m_MBTSContainerKey.key().empty()) {
      SG::WriteHandle<TileCellContainer> MBTSContainer(m_MBTSContainerKey, ctx);
      ATH_CHECK( MBTSContainer.record(std::move(MBTSCells)) );
    }
    
    if (!m_E4prContainerKey.key().empty()) {
      SG::WriteHandle<TileCellContainer> E4prContainer(m_E4prContainerKey, ctx);
      ATH_CHECK( E4prContainer.record(std::move(E4prCells)) );
    }
    
    CaloCell_ID::SUBCALO caloNum = CaloCell_ID::TILE;

    //specify that a given calorimeter has been filled
    theCellContainer->setHasCalo(caloNum);
  }
  
  //enum EventFlagErrorState { NotSet, Warning, Error };
  xAOD::EventInfo::EventFlagErrorState error = xAOD::EventInfo::NotSet;
  // flag will contain status of a given event
  // every 4 bits - status of partitions LBA,LBC,EBA,EBC
  // bits 0-3   - there is a signal above threshold in partitions
  // bits 4-7   - there are channels with underflow (sample=0) in partition (since rel 17.2.6.4)
  // bits 8-11  - there are channels with overflow (sample=m_tileInfo->ADCmax()) in partition (since rel 17.2.6.4)
  // bits 12-15 - there are at least 16 drawers with bad quality in partition
  // bits 16-19 - maximal length of consecutive bad area (since rel 17.2.6.5)
  // bits 20-23 - there are at least 16 drawers which are completely masked in partition
  // bits 24-27 - there are at least 16 drawers which do not send data in partition
  // bits 28-31 - reserved for global good/warning/bad status
  // bits 20-27 are also used for module number which gives warning status (since release 17.2.6.5)
  //            in case of warning we are sure that bits which indicates error are not filled
  unsigned int flag = 0;
  
  int drConsecMaxMax = 0;
  int drConsecNum = 0;

  for (int p = 1; p < 5; ++p) {
    TileDrawerEvtStatus * evt = drawerEvtStatus[p];
    //TileDrawerRunStatus * run = m_drawerRunStatus[p];
    int drAbsent = 0;
    int drMasked = 0;
    int drConsec = 0;
    int drConsecMax = 0;
    int hasBadQ = 0;
    int hasOver = 0;
    int hasUnder = 0;
    int hasSig = 0;
    for (int d = 0; d < 64; ++d) {
      if (evt[d].nChannels == 0) {
        ++drConsec;
        ++drAbsent;
        //++(run[d].drawerAbsent);
      } else if (evt[d].nMaskedChannels >= evt[d].nChannels) {
        ++drConsec;
        ++drMasked;
        //++(run[d].drawerMasked);
      } else {
        if (drConsec > drConsecMax) {
          drConsecMax = drConsec;
          if (drConsecMax > drConsecMaxMax) {
            drConsecMaxMax = drConsecMax;
            drConsecNum = ((p - 1) << 6) | (d - drConsec);
          }
        }
        drConsec = 0;
        if (evt[d].nMaskedChannels > 0) {
          //++(run[d].channelsMasked);
        }
        if (evt[d].nBadQuality) ++hasBadQ;
        if (evt[d].nOverflow) ++hasOver;
        if (evt[d].nUnderflow) ++hasUnder;
        if (evt[d].nSomeSignal) ++hasSig;
      }
    }
    if (drConsec != 0 && drConsecMax < 64) { // 64th drawer is bad - check transition from 64th to 1st drawer
      for (int d = 0; d < drConsecMax; ++d) {
        if (evt[d].nChannels == 0 || evt[d].nMaskedChannels >= evt[d].nChannels) {
          ++drConsec;
        } else {
          break;
        }
      }
      if (drConsec > drConsecMax) {
        drConsecMax = drConsec;
      }
    }
    unsigned int fl = 0;
    if (drAbsent > 15) {
      fl |= 0x01000000;
      error = xAOD::EventInfo::Error;
    }
    if (drMasked > 15) {
      fl |= 0x00100000;
      error = xAOD::EventInfo::Error;
    }
    //if (drConsecMax > 1)fl |= 0x00010000; // want to use these bits for length of consecutive area
    if (hasBadQ > 15) fl |= 0x00001000;
    if (hasOver) fl |= 0x00000100;
    if (hasUnder) fl |= 0x00000010;
    if (hasSig) fl |= 0x00000001;

#ifdef ALLOW_DEBUG_COUT
    std::cout<<"partition "<<p<<" drAbsent "<<drAbsent<<" drMasked "<<drMasked<<" drConsec "<<drConsecMax
    <<" hasBadQ "<<hasBadQ<<" hasOver "<<hasOver<<" hasUnder "<<hasUnder<<" hasSig "<<hasSig<<std::endl;
#endif
    flag |= fl << (p - 1);
  }

  // number of consecutively masked modules (if it's > 15 we have error already set)
  flag |= (std::min(15, drConsecMaxMax) << 16);

  if (drConsecMaxMax > 1 && error < xAOD::EventInfo::Warning) {
    // setting warning flag
    error = xAOD::EventInfo::Warning;
    // putting starting module number of consecutive bad area
    // instead of bits which indicates 16 masked or 16 absent modules in partition
    flag |= (drConsecNum << 20);
#ifdef ALLOW_DEBUG_COUT
    std::cout<<"warning in partition " << (drConsecNum>>6)+1 << " for modules "
    <<(drConsecNum)%64 <<" - " <<(drConsecNum+drConsecMaxMax-1)%64 <<std::endl;
#endif
  }
  
#ifdef ALLOW_DEBUG_COUT
  std::cout<<"partition flag 0x0"<<std::hex<<flag<<std::dec<<" error "<<error<<std::endl;
#endif

  //++m_eventErrorCounter[error]; // error index is 0 or 1 or 2 here
  //++m_eventErrorCounter[3]; // count separately total number of events
  

  // retrieve EventInfo
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);

  if (eventInfo.isValid()) {

    if (flag != 0) {
      ATH_MSG_DEBUG( " set eventInfo for Tile for this event to 0x" << MSG::hex << flag << MSG::dec );
      if (!eventInfo->updateEventFlags(xAOD::EventInfo::Tile, flag)) {
        ATH_MSG_WARNING( " cannot set eventInfo for Tile " );
      }
    }
    
    if (error != xAOD::EventInfo::NotSet) {
      ATH_MSG_DEBUG( " set error bits for Tile for this event to " << error );
      if (!eventInfo->updateErrorState(xAOD::EventInfo::Tile, error)) {
        ATH_MSG_WARNING( " cannot set error state for Tile " );
      }
    }

  }
  else {
    ATH_MSG_WARNING( " cannot retrieve EventInfo, will not set Tile information " );
  }
  
  // Execution completed.
  ATH_MSG_DEBUG( "TileCellBuilder execution completed." );

  return StatusCode::SUCCESS;
}

//************************************************************************
void TileCellBuilder::correctCell(TileCell* pCell, int correction, int pmt, int gain
    , float ener, float time, unsigned char iqual, unsigned char qbit, int ch_type) const {
//************************************************************************

// Merge two pmts in one cell if needed
// and apply corrections

  // do not trust to energies below certain threshold
  if (ener < m_minEneChan[ch_type]) {
#ifdef ALLOW_DEBUG_COUT
    std::cout << "channel with negative energy " << ener << " => setting quality to 255" << std::endl;
#endif
    iqual = 255;
  }

  switch (correction) {
    case 1: // first pmt for this cell
      pCell->addEnergy(ener, pmt, gain);
      pCell->setTime(time); // overwrite time completely
      pCell->setQuality(iqual, qbit, pmt);
      pCell->setQuality(0, 0, 1 - pmt);
      break;
    case 2: // second pmt for this cell
      pCell->addEnergy(ener, pmt, gain);
      pCell->setTime(time, pmt); // calculate average time and timeDiff
      pCell->setQuality(iqual, qbit, pmt);
      break;
  }
}

unsigned char TileCellBuilder::qbits (TileDrawerEvtStatusArray& drawerEvtStatus,
                                      TileFragHash::TYPE RChType,
                                      int ros,
                                      int drawer,
                                      bool count_over,
                                      bool good_time,
                                      bool good_ener,
                                      bool overflow,
                                      bool underflow,
                                      bool overfit) const
{
  ++drawerEvtStatus[ros][drawer].nChannels;
  // new feature in rel 17.2.7 - count underflows and overflows
  if (count_over) {
    if (overflow) ++drawerEvtStatus[ros][drawer].nOverflow;
    if (underflow) ++drawerEvtStatus[ros][drawer].nUnderflow;
  }
#ifdef ALLOW_DEBUG_COUT
  if (overflow)  std::cout << "channel with overflow " << ((count_over)?"":"MBTS") << std::endl;
  if (underflow) std::cout << "channel with underflow " << ((count_over)?"":"MBTS") << std::endl;
  if (overfit)   std::cout << "channel with corrected overflow " << ((count_over)?"":"MBTS") << std::endl;
#endif

  unsigned char qbit = (overfit) ? (TileFragHash::FitFilter & TileCell::MASK_ALGO)
                                 : (RChType & TileCell::MASK_ALGO);
  if (good_time) qbit |= TileCell::MASK_TIME;
  if (overflow || underflow) qbit |= TileCell::MASK_OVER;
      
  if (good_ener) {
    qbit |= TileCell::MASK_AMPL;
    if (count_over) {
      ++drawerEvtStatus[ros][drawer].nSomeSignal;
    }
  }

  return qbit;
}

// masking for MBTS with single channel
bool
TileCellBuilder::maskBadChannel (TileDrawerEvtStatusArray& drawerEvtStatus,
                                 const TileDQstatus* DQstatus,
                                 TileCell* pCell, HWIdentifier hwid) const
{
  int ros = m_tileHWID->ros(hwid);
  int drawer = m_tileHWID->drawer(hwid);
  int chan = m_tileHWID->channel(hwid);
  int gain = m_tileHWID->adc(hwid);
  int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);
  TileBchStatus chStatus = m_tileBadChanTool->getAdcStatus(drawerIdx, chan, gain);

  // check quality first
  bool bad = ((int) pCell->qual1() > m_qualityCut);
  if (bad) {
    ++drawerEvtStatus[ros][drawer].nBadQuality;

  } else {
    // check bad status in DB
    bad = chStatus.isBad();

    // Now checking the DQ status
    if (!bad && m_notUpgradeCabling && DQstatus) {
      bad = !(DQstatus->isAdcDQgood(ros, drawer, chan, gain)
            && isChanDCSgood(ros, drawer, chan));
    }
  }

  if (bad) {
    // only one channel in this cell and it is bad
    ++drawerEvtStatus[ros][drawer].nMaskedChannels;

    //pCell->setEnergy(m_zeroEnergy,0.0,TileID::LOWGAIN,CaloGain::INVALIDGAIN); // reset energy completely, indicate problem putting low gain
    //pCell->setTime(0.0); // reset time completely
    //pCell->setQuality(255,TileCell::MASK_BADCH,0); // reset quality flag for first pmt

    if (gain == CaloGain::INVALIDGAIN) {
      pCell->setEnergy(0.0, 0.0, TileID::LOWGAIN, CaloGain::INVALIDGAIN); // reset energy completely, indicate problem putting low gain
    } else {
      pCell->setEnergy(0.0, 0.0); // reset energy completely without changing the gain
    }
    pCell->setTime(-100.0); // reset time to big negative number to distinguish this bad cell from good cells
    pCell->setQuality(255, (TileCell::MASK_BADCH | (pCell->qbit1() & TileCell::MASK_ALGO)), 0); // reset quality flag for first pmt
    pCell->setQuality(0, TileCell::MASK_BADCH, 1); // reset quality flag for second pmt

    return true;

  } else if (chStatus.isBadTiming()) {
    pCell->setTime(0.0); // channel with bad timing - no cell time
    uint8_t qbit1 = pCell->qbit1() & (~(TileCell::MASK_TIME)); // clear time bit for first pmt
    pCell->setQuality(pCell->qual1(), qbit1, 0); // update qbits for first pmt
  }

  return false;
}
  

// masking for normal cells
bool TileCellBuilder::maskBadChannels (TileDrawerEvtStatusArray& drawerEvtStatus,
                                       const TileDQstatus* DQstatus,
                                       TileCell* pCell) const
{
  bool single_PMT_C10 = false;

  const CaloDetDescrElement* caloDDE = pCell->caloDDE();

  IdentifierHash hash1 = caloDDE->onl1();
  IdentifierHash hash2 = caloDDE->onl2();

  int gain1 = pCell->gain1();

  HWIdentifier ch_id1 = m_tileHWID->channel_id(hash1);

  int ros1 = m_tileHWID->ros(ch_id1);
  int drawer1 = m_tileHWID->drawer(ch_id1);
  int chan1 = m_tileHWID->channel(ch_id1);
  int drawerIdx1 = TileCalibUtils::getDrawerIdx(ros1, drawer1);
  const TileBchStatus& chStatus1 = m_tileBadChanTool->getAdcStatus(drawerIdx1, chan1, (gain1 < 0) ? 1 : gain1);
  
  // check quality first
  bool bad1 = ((int) pCell->qual1() > m_qualityCut);
  if (bad1) {
    ++drawerEvtStatus[ros1][drawer1].nBadQuality;

  } else {
    // check bad status in DB
    bad1 = (gain1 < 0) || chStatus1.isBad();

    // Now checking the DQ status
    if (!bad1 && m_notUpgradeCabling && DQstatus) {
      bad1 = !(DQstatus->isAdcDQgood(ros1, drawer1, chan1, gain1)
              && isChanDCSgood(ros1, drawer1, chan1));
    }
  }

  if (hash2 == TileHWID::NOT_VALID_HASH) {
    // gap/crack scintillators with one PMT per cell

    if (bad1) {
      // only one channel in this cell and it is bad
      ++drawerEvtStatus[ros1][drawer1].nMaskedChannels;

      if (gain1 == CaloGain::INVALIDGAIN) {
        pCell->setEnergy(m_zeroEnergy, 0.0, TileID::LOWGAIN, CaloGain::INVALIDGAIN); // reset energy completely, indicate problem putting low gain
      } else {
        pCell->setEnergy(m_zeroEnergy, 0.0); // reset energy completely without changing gain
      }
      pCell->setTime(0.0); // reset time completely
      pCell->setQuality(255, (TileCell::MASK_BADCH | (pCell->qbit1() & TileCell::MASK_ALGO)), 0); // reset quality flag for first pmt
      pCell->setQuality(0, TileCell::MASK_BADCH, 1); // reset quality flag for second pmt

      return true;

    } else if (chStatus1.isBadTiming()) {
      pCell->setTime(0.0); // channel with bad timing - no cell time
      uint8_t qbit1 = pCell->qbit1() & (~(TileCell::MASK_TIME)); // clear time bit for first pmt
      pCell->setQuality(pCell->qual1(), qbit1, 0); // update qbits for first pmt
    }
    
  } else { //cell has both PMTs

    int gain2 = pCell->gain2();
    
    HWIdentifier ch_id2 = m_tileHWID->channel_id(hash2);

    int ros2 = m_tileHWID->ros(ch_id2);
    int drawer2 = m_tileHWID->drawer(ch_id2);
    int chan2 = m_tileHWID->channel(ch_id2);
    int drawerIdx2 = TileCalibUtils::getDrawerIdx(ros2, drawer2);
    const TileBchStatus& chStatus2 = m_tileBadChanTool->getAdcStatus(drawerIdx2, chan2, (gain2 < 0) ? 1 : gain2);

    // check quality first
    bool bad2 = ((int) pCell->qual2() > m_qualityCut);
    if (bad2) {
      ++drawerEvtStatus[ros2][drawer2].nBadQuality;

    } else {
      // check bad status in DB
      bad2 = (gain2 < 0) || chStatus2.isBad();

      // Now checking the DQ status
      if (!bad2 && m_notUpgradeCabling && DQstatus) {
        bad2 = !(DQstatus->isAdcDQgood(ros2, drawer2, chan2, gain2)
                && isChanDCSgood(ros2, drawer2, chan2));
      }
    }

    single_PMT_C10 = (((ros2 == TileHWID::EXTBAR_POS && chan1 == 4)
                      || (ros2 == TileHWID::EXTBAR_NEG && chan2 == 4))
                      && !m_cabling->C10_connected(drawer2));
    if (single_PMT_C10) {
      // for special C10 disconnected channel might be masked in DB
      // and energy of good channel is taken twice with correct weight
      // but if this channel is not masked in DB - set its bad status
      // equal to bad status of real channel, so that cell is masked correctly
      // if real channel connected to a cell is bad
#ifdef ALLOW_DEBUG_COUT
      static int cnt=0;
      if (++cnt < 17) {
        std::cout << "special C10 in " << ((ros2==TileHWID::EXTBAR_POS) ? "EBA" : "EBC")
        << drawer2+1 << " status " << chan1 << "/" << chan2 << " "
        << (chStatus1.isBad()?"bad":"good") << "/"
        << (chStatus2.isBad()?"bad":"good") << "/"
        << ((m_run2plus)?" RUN2+ cabling": "RUN1 cabling")
        << std::endl;
      }
#endif
      if (chan1 == 4) {
        if (m_run2plus || !chStatus1.isBad()) {
#ifdef ALLOW_DEBUG_COUT
          if (cnt < 17) {
            std::cout << "Ene of chan1 was " << pCell->ene1() << " changing to half of " << pCell->ene2()
            << " and setting bad1=true" << std::endl;
          }
#endif
          pCell->setEnergy(pCell->ene2()/2., pCell->ene2()/2., gain2, gain2);
          //bad1 = bad2;
          bad1 = true;
          --drawerEvtStatus[ros1][drawer1].nMaskedChannels; // since it's fake masking, decrease counter by 1 in advance
        }
      } else {
        if (m_run2plus || !chStatus2.isBad()) {
#ifdef ALLOW_DEBUG_COUT
          if (cnt < 17) {
            std::cout << "Ene of chan2 was " << pCell->ene2() << " changing to half of " << pCell->ene1()
            << " and setting bad2=true" << std::endl;
          }
#endif
          pCell->setEnergy(pCell->ene1()/2., pCell->ene1()/2., gain1, gain1);
          //bad2 = bad1;
          bad2 = true;
          --drawerEvtStatus[ros2][drawer2].nMaskedChannels; // since it's fake masking, decrease counter by 1 in advance
        }
      }
    }
    if (bad1 && bad2) {
      // both channels are bad
      ++drawerEvtStatus[ros1][drawer1].nMaskedChannels;
      ++drawerEvtStatus[ros2][drawer2].nMaskedChannels;

      if (gain1 == CaloGain::INVALIDGAIN || gain2 == CaloGain::INVALIDGAIN) {
        if (gain1 == CaloGain::INVALIDGAIN) gain1 = 0; // this is TileID::LOWGAIN; - commented out to make Coverity happy
        if (gain2 == CaloGain::INVALIDGAIN) gain2 = 0; // this is TileID::LOWGAIN; - commented out to make Coverity happy
        pCell->setEnergy(m_zeroEnergy, m_zeroEnergy, gain1, gain2); // reset energy completely, indicate problem putting low gain
      } else {
        pCell->setEnergy(m_zeroEnergy, m_zeroEnergy); // reset energy completely without changing gain
      }
      pCell->setTime(0.0);  // reset time completely
      pCell->setQuality(255, (TileCell::MASK_BADCH | (pCell->qbit1() & TileCell::MASK_ALGO)), 0); // reset quality flag for first pmt
      pCell->setQuality(255, (TileCell::MASK_BADCH | (pCell->qbit2() & TileCell::MASK_ALGO)), 1); // reset quality flag for second pmt

      return true;

    } else if (bad1 && !bad2) {
      // first channel is bad
      ++drawerEvtStatus[ros1][drawer1].nMaskedChannels;

      float ene2 = pCell->ene2();
      pCell->setEnergy(ene2, ene2, gain2, gain2); // use energy/gain from second pmt for both pmts

      if (chStatus2.isBadTiming()) {
        pCell->setTime(0.0); // time in second pmt is bad - no cell time
        uint8_t qual2 = pCell->qual2();
        uint8_t qbit2 = pCell->qbit2() & (~(TileCell::MASK_TIME)); // clear time bit for second pmt
        uint8_t qbit1 = qbit2 | TileCell::MASK_BADCH; // set bad channel bit for first pmt
        uint8_t qual1 = qual2 + (gain1 - gain2); // if gains are different, qua11 and qual2 will be different
        if (qual1 > m_qualityCut && gain1 > gain2) qual1 = qual2 - (gain1 - gain2); // new feature in release 17.2
        pCell->setQuality(qual1, qbit1, 0); // change quality and qbits for first pmt
        pCell->setQuality(qual2, qbit2, 1); // update qbits for second pmt
      } else {
        pCell->setTime(pCell->time2());  // use time from second pmt as cell time
        pCell->setQuality(pCell->qual2(), (pCell->qbit2() | TileCell::MASK_BADCH), 0); // change quality flag for first pmt
      }
      
      return true;

    } else if (!bad1 && bad2) {
      // second channel is bad
      ++drawerEvtStatus[ros2][drawer2].nMaskedChannels;

      float ene1 = pCell->ene1();
      pCell->setEnergy(ene1, ene1, gain1, gain1);  // use energy/gain from first pmt for both pmts

      if (chStatus1.isBadTiming()) {
        pCell->setTime(0.0); // time in first pmt is bad - no cell time
        uint8_t qual1 = pCell->qual1();
        uint8_t qbit1 = pCell->qbit1() & (~(TileCell::MASK_TIME)); // clear time bit for first pmt
        uint8_t qbit2 = qbit1 | TileCell::MASK_BADCH; // set bad channel bit for second pmt
        uint8_t qual2 = qual1 + (gain2 - gain1); // if gains are different, qua11 and qual2 will be different
        if (qual2 > m_qualityCut && gain2 > gain1) qual2 = qual1 - (gain2 - gain1); // new feature in release 17.2
        pCell->setQuality(qual1, qbit1, 0); // update qbits for first pmt
        pCell->setQuality(qual2, qbit2, 1); // change quality and qbits for second pmt
      } else {
        pCell->setTime(pCell->time1());  // use time from first pmt as cell time
        pCell->setQuality(pCell->qual1(), (pCell->qbit1() | TileCell::MASK_BADCH), 1); // change quality flag for second pmt
      }
      
      return true;

    } else {

      if (chStatus1.isBadTiming()) {

        if (chStatus2.isBadTiming()) {
          pCell->setTime(0.0); // time in both pmts is bad - no cell time
          uint8_t qbit2 = pCell->qbit2() & (~(TileCell::MASK_TIME)); // clear time bit for second pmt
          pCell->setQuality(pCell->qual2(), qbit2, 1); // update qbits for second pmt
        } else {
          pCell->setTime(pCell->time2());  // use time from second pmt as cell time
        }
        uint8_t qbit1 = pCell->qbit1() & (~(TileCell::MASK_TIME)); // clear time bit for first pmt
        pCell->setQuality(pCell->qual1(), qbit1, 0); // update qbits for first pmt

      } else if (chStatus2.isBadTiming()) {

        pCell->setTime(pCell->time1());  // use time from first pmt as cell time
        uint8_t qbit2 = pCell->qbit2() & (~(TileCell::MASK_TIME)); // clear time bit for second pmt
        pCell->setQuality(pCell->qual2(), qbit2, 1); // update qbits for second pmt
      }
    }
    
  }
  
  return single_PMT_C10;
}


template<class ITERATOR, class COLLECTION>
void TileCellBuilder::build (const EventContext& ctx,
                             TileDrawerEvtStatusArray& drawerEvtStatus,
                             VecParams& params,
                             const ITERATOR & begin,
                             const ITERATOR & end,
                             COLLECTION* coll,
                             TileCellContainer* MBTSCells,
                             TileCellContainer* E4prCells) const
{
  // Now retrieve the TileDQstatus
  const TileDQstatus* DQstatus = nullptr;
  if(m_notUpgradeCabling) {
    DQstatus = SG::makeHandle (m_DQstatusKey, ctx).get();
  }

  /* zero all counters and sums */
  int nTwo = 0;
  int nCell = 0;
  int nFake = 0;
  int nMBTS = 0;
  int nE4pr = 0;
  int nChan = 0;
  float eCh = 0.0;
  float eCellTot = 0.0;
  float eMBTSTot = 0.0;
  float eE4prTot = 0.0;
  bool EBdrawerPresent[128];
  memset(EBdrawerPresent, 0, sizeof(EBdrawerPresent));
  DataPool<TileCell> tileCellsP(5217);
  //**
  //* Iterate over raw channels, creating new TileCells (or incrementing
  //* existing ones). Add each new TileCell to the output collection
  //**

  std::vector<TileCell*> allCells (m_tileID->cell_hash_max(), nullptr);

  for (ITERATOR rawItr = begin; rawItr != end; ++rawItr) {

    const TileRawChannel* pChannel = (*rawItr);
    HWIdentifier adc_id = pChannel->adc_HWID();
    int ros = m_tileHWID->ros(adc_id);
    int drawer = m_tileHWID->drawer(adc_id);
    int channel = m_tileHWID->channel(adc_id);
    int gain = m_tileHWID->adc(adc_id);
    if (gain == m_skipGain) {
      ATH_MSG_VERBOSE (" skipping adc_id=" << m_tileHWID->to_string(adc_id));
      continue; // select only one of two gains in calib runs
    }
    int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);
    if (channel == 0 && ros > 2) EBdrawerPresent[(ros - 3) * 64 + drawer] = true; // EB drawer appeared in the data

    float time = pChannel->uncorrTime(); // take uncorrected time (if available)
    float amp = pChannel->amplitude();

    TileRawChannelUnit::UNIT oldUnit = params.m_RChUnit;
    if (params.m_correctAmplitude && time > m_timeMinThresh && time < m_timeMaxThresh) { // parabolic correction
      if (params.m_RChUnit > TileRawChannelUnit::OnlineADCcounts) { // convert from online units to ADC counts
        oldUnit = TileRawChannelUnit::ADCcounts;
        amp = m_tileToolEmscale->undoOnlCalib(drawerIdx, channel, gain, amp, params.m_RChUnit);
        if (amp > m_ampMinThresh) // amp cut in ADC counts
          amp *= TileRawChannelBuilder::correctAmp(time,params.m_of2);
      } else if (params.m_RChUnit == TileRawChannelUnit::ADCcounts
                 || params.m_RChUnit == TileRawChannelUnit::OnlineADCcounts) {
        if (amp > m_ampMinThresh)
          amp *= TileRawChannelBuilder::correctAmp(time,params.m_of2);
      } else {
        ATH_MSG_ERROR( "Units in raw channel container is " << params.m_RChUnit );
        ATH_MSG_ERROR( "But amplitude correction works only with ADC counts " );
        ATH_MSG_ERROR( "Please, disable CIS calibration in optimal filter " );
      }
    }

    float qual = pChannel->quality();

    // check that time was really reconstructed
    bool good_time = (fabs(time) < params.m_maxTimeCorr);
    bool non_zero_time = (params.m_RChType == TileFragHash::OptFilterDspCompressed)
                          ? ((qual > 2.99 && qual < 4.01))
                          : ((qual > 0.0 || params.m_RChType == TileFragHash::OptFilterDsp));

    // new feature in rel 17.2.7 - pedestal keeps information about overflow and underflow
    // if there is an underflow, 10000 is added to pedestal value
    // if there is an overflow,  20000 is added to pedestal value
    // if there is an underflow in all samples, 80000 is added to pedestal value
    // if there is an overflow in all samples,  90000 is added to pedestal value
    // if there is bad pattern nunber N 100000+N*10000 is added to pedestal value
    bool overflow = false;
    bool underflow = false;
    bool overfit = false;
    float ped = pChannel->pedestal();
    if (ped > 59500.) { // one of bad patterns
      qual = 9999; // mask all bad patterns
    } else if (ped > 39500.) { // 40000 for constant value or 50000 for all zeros in disconnexted channel
      // nothing to do
    } else if (ped > m_ADCmaskValuePlusEps) { // 10000 for underflow or 20000 for overflow or 10000+20000
      // NOTE: opt filter can yield values between (-500, 4600) and overlay magic number is 4800 in case of 12-bit ADCs
      underflow = ((ped < 10000. + m_ADCmaskValuePlusEps) || (ped > 29500.));
      overflow  =  (ped > 10000. + m_ADCmaskValuePlusEps);
      // special flag indicating that fit method was applied for overflow channels
      overfit = ( (ped > 20000. + m_ADCmaskValueMinusEps && ped < 29500) || (ped > 30000. + m_ADCmaskValueMinusEps && ped < 39500) );

      if (overflow
          && gain == TileID::LOWGAIN
          && amp > 0
          && time > m_timeMinThresh
          && time < m_timeMaxThresh) {

        qual = fabs(qual);
        if (qual > m_qualityCut && qual < 9999.) {
          qual = m_qualityCut; // to avoid masking of overflow in low gain
        }
      }
    }

    // apply time correction if needed
    if (params.m_correctTime && good_time && non_zero_time)
      time -= m_tileToolTiming->getSignalPhase(drawerIdx, channel, gain);
    else
      time = pChannel->time();

    ++nChan;
    eCh += amp;

    int index, pmt;
    int channel1 = channel;

    if (m_useDemoCabling == 2015 && ros == 4 && drawer == 1) {
        int pmt2channel[48] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
                               26,25,24,29,31,32,27,28,30,35,34,33,38,37,43,44,41,40,39,36,42,47,46,45};
        channel1 = pmt2channel[channel];

    } else if ( (m_useDemoCabling >= 2016 && m_useDemoCabling <= 2019)
		&& (ros == 2 && (drawer == 1 || drawer>2)) ) {
      int pmt2channel[48] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
			     26,25,24,29,28,27,32,31,30,35,34,33,38,37,36,41,40,39,44,43,42,47,46,45};
      channel1 = pmt2channel[channel];
    } else if ( (m_useDemoCabling >= 2018)
                && (ros == 4 && drawer>=2) ) {
      int pmt2channelEB[48] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
			       31,32,30,35, 33,34, 38,37,41,40,39,36, 26,25,24,29,28,27, 44,43,42,47,46,45};
      channel1 = pmt2channelEB[channel];
    }

    Identifier cell_id = m_cabling->h2s_cell_id_index (ros, drawer, channel1, index, pmt);

    if (index == -3) { // E4' cells

      if (E4prCells) { // do something with them only if the container exists
        ++nE4pr;

        // convert ADC counts to MeV. like for normal cells
        float ener = m_tileToolEmscale->channelCalib(drawerIdx, channel, gain, amp , oldUnit
                                                     , TileRawChannelUnit::MegaElectronVolts);

        eE4prTot += ener;
        unsigned char iqual = iquality(qual);
        // for E4' cell qbit use only non_zero_time flag and check that energy is above standatd energy threshold in MeV
        unsigned char qbit = qbits(drawerEvtStatus, params.m_RChType,
                                   ros, drawer, true, non_zero_time, (fabs(ener) > m_eneForTimeCut)
                                   , overflow, underflow, overfit);
        CaloGain::CaloGain cgain = (gain == TileID::HIGHGAIN)
                                   ? CaloGain::TILEONEHIGH
                                   : CaloGain::TILEONELOW;

        TileCell* pCell = tileCellsP.nextElementPtr();
        // no CaloDDE
        // Cell ID is set explicitly
        pCell->set(NULL, cell_id);
        pCell->setEnergy_nonvirt(ener, 0, cgain, 3);
        pCell->setTime_nonvirt(time);
        pCell->setQual1(iqual);
        pCell->setQual2(0);
        pCell->setQbit1(qbit);
        pCell->setQbit2(0);

        if (msgLvl(MSG::VERBOSE)) {
          msg(MSG::VERBOSE) << " E4'  cell_id=" << m_tileTBID->to_string(cell_id)
                            << " adc_id=" << m_tileHWID->to_string(adc_id)
                            << " ene= " << ener
                            << " amp= " << amp
                            << " time= " << time
                            << " qual= " << pChannel->quality()
                            << " iqual= " << (int) iqual
                            << " qbit = 0x" << MSG::hex << (int) qbit << MSG::dec;

          if (ped > m_ADCmaskValuePlusEps)
            msg(MSG::VERBOSE) << " err = " << TileRawChannelBuilder::BadPatternName(ped) << endmsg;
          else
            msg(MSG::VERBOSE) << endmsg;
        }

        if (m_maskBadChannels && maskBadChannel(drawerEvtStatus, DQstatus,
                                                pCell, adc_id))
          ATH_MSG_VERBOSE ( "cell with id=" << m_tileTBID->to_string(cell_id)
                             << " bad channel masked, new energy=" << pCell->energy() );

        E4prCells->push_back(pCell); // store cell in container

      }

    } else if (index == -2) { // MBTS cells

      if (MBTSCells) { // do something with them only if contaier existst
        ++nMBTS;

        // convert ADC counts to pCb and not to MeV
        float ener = m_tileToolEmscale->channelCalib(drawerIdx, channel, gain, amp , oldUnit
                                                     , TileRawChannelUnit::PicoCoulombs);

        eMBTSTot += ener;
        unsigned char iqual = iquality(qual);
        // for MBTS qbit use AND of good_time and non_zero_time and check that energy is above MBTS energy threshold in pC
        unsigned char qbit = qbits(drawerEvtStatus, params.m_RChType,
                                   ros, drawer, false, (good_time && non_zero_time),
                                   (fabs(ener) > m_eneForTimeCutMBTS), overflow, underflow, overfit);

        CaloGain::CaloGain cgain = (gain == TileID::HIGHGAIN)
                                   ? CaloGain::TILEONEHIGH
                                   : CaloGain::TILEONELOW;

        TileCell* pCell = tileCellsP.nextElementPtr();
        // MBTS CaloDDE
        // Cell ID is set explicitly
        pCell->set((m_mbtsMgr) ? m_mbtsMgr->get_element(cell_id) : NULL, cell_id);
        pCell->setEnergy_nonvirt(ener, 0, cgain, 3);
        pCell->setTime_nonvirt(time);
        pCell->setQual1(iqual);
        pCell->setQual2(0);
        pCell->setQbit1(qbit);
        pCell->setQbit2(0);

        if (msgLvl(MSG::VERBOSE)) {
          msg(MSG::VERBOSE) << " MBTS cell_id=" << m_tileTBID->to_string(cell_id)
                            << " adc_id=" << m_tileHWID->to_string(adc_id)
                            << " ene= " << ener
                            << " amp= " << amp
                            << " time= " << time
                            << " qual= " << pChannel->quality()
                            << " iqual= " << (int) iqual
                            << " qbit = 0x" << MSG::hex << (int) qbit << MSG::dec;

          if (ped > m_ADCmaskValuePlusEps)
            msg(MSG::VERBOSE) << " err = " << TileRawChannelBuilder::BadPatternName(ped) << endmsg;
          else
            msg(MSG::VERBOSE) << endmsg;
        }

        if (m_maskBadChannels && maskBadChannel(drawerEvtStatus, DQstatus,
                                                pCell, adc_id))
          ATH_MSG_VERBOSE ( "cell with id=" << m_tileTBID->to_string(cell_id)
                             << " bad channel masked, new energy=" << pCell->energy() );

        MBTSCells->push_back(pCell); // store cell in container

      }
    } else if (index != -1) { // connected channel

      float ener = m_tileToolEmscale->channelCalib(drawerIdx, channel, gain, amp
           , oldUnit, TileRawChannelUnit::MegaElectronVolts);

      eCellTot += ener;

      unsigned char iqual = iquality(qual);
      // for normal cell qbit use only non_zero_time flag and check that energy is above standatd energy threshold in MeV
      unsigned char qbit = qbits(drawerEvtStatus, params.m_RChType,
                                 ros, drawer, true, non_zero_time, (fabs(ener) > m_eneForTimeCut)
          , overflow, underflow, overfit);


      if (m_run2plus && channel == E1_CHANNEL && ros > 2) { // Raw channel -> E1 cell.

       int drawer2 = m_cabling->E1_merged_with_run2plus(ros,drawer);
       if (drawer2 != 0) { // Raw channel splitted into two E1 cells for Run 2.
         int side = (ros == 3) ? 1 : -1;
         Identifier cell_id2 = m_tileID->cell_id(TileID::GAPDET, side, drawer2, m_towerE1, TileID::SAMP_E);
         int index2 = m_tileID->cell_hash(cell_id2);
         TileCell* pCell2 = tileCellsP.nextElementPtr();
         ++nCell;
         allCells[index2] = pCell2;
         const CaloDetDescrElement* dde2 = m_tileMgr->get_cell_element(index2);
         pCell2->set(dde2, cell_id2);
         pCell2->setEnergy_nonvirt(0, 0, CaloGain::INVALIDGAIN, CaloGain::INVALIDGAIN);
         int pmt2(0);
         ener /= 2.0F;
         correctCell(pCell2, 1, pmt2, gain, ener, time, iqual, qbit, 1);

         ATH_MSG_DEBUG("E1 cell Id => " << m_tileID->to_string(cell_id)
                      << " splitted into " << m_tileID->to_string(cell_id2));


       }

     }

      TileCell* pCell = allCells[index];
      if (pCell) {
        ++nTwo;
        correctCell(pCell, 2, pmt, gain, ener, time, iqual, qbit, 0); // correct & merge 2 PMTs in one cell
      } else {
        ++nCell;
        allCells[index] = pCell = tileCellsP.nextElementPtr();
        const CaloDetDescrElement* dde = m_tileMgr->get_cell_element(index);
        pCell->set(dde, cell_id);
        pCell->setEnergy_nonvirt(0, 0, CaloGain::INVALIDGAIN, CaloGain::INVALIDGAIN);
        int ch_type = (dde->onl2() == TileHWID::NOT_VALID_HASH) ? 1 : 0;
        correctCell(pCell, 1, pmt, gain, ener, time, iqual, qbit, ch_type); // correct & save e,t,q in new cell
      }

      if (msgLvl(MSG::VERBOSE)) {
        float calib1 = (amp != 0) ? ener / amp : 0.0;
        msg(MSG::VERBOSE) << " cell_id=" << m_tileID->to_string(cell_id, -2)
                          << " adc_id=" << m_tileHWID->to_string(adc_id)
                          << " calib=" << calib1
                          << " nCell=" << nCell
                          << " energy=" << ener << " (" << pCell->energy() << ", " << pCell->eneDiff() << ")" << endmsg;

        msg(MSG::VERBOSE) << " amp= " << amp
                          << " time= " << time
                          << " qual= " << pChannel->quality()
                          << " iqual= " << (int) iqual
                          << " qbit = 0x" << MSG::hex << (int) qbit << MSG::dec;

        if (ped > m_ADCmaskValuePlusEps)
          msg(MSG::VERBOSE) << " err = " << TileRawChannelBuilder::BadPatternName(ped) << endmsg;
        else
          msg(MSG::VERBOSE) << endmsg;
      }

    } else {

      if (msgLvl(MSG::VERBOSE)) {

        unsigned char iqual = iquality(qual);
        unsigned char qbit = qbits(drawerEvtStatus, params.m_RChType,
                                   0, drawer, false, non_zero_time, false, overflow, underflow, overfit); //fake ros number here

        msg(MSG::VERBOSE) << " channel with adc_id=" << m_tileHWID->to_string(adc_id)
                          << " is not connected" << endmsg;

        msg(MSG::VERBOSE) << " amp= " << amp
                          << " time= " << time
                          << " qual= " << pChannel->quality()
                          << " iqual= " << (int) iqual
                          << " qbit = 0x" << MSG::hex << (int) qbit << MSG::dec;

        if (ped > m_ADCmaskValuePlusEps)
          msg(MSG::VERBOSE) << " err = " << TileRawChannelBuilder::BadPatternName(ped) << endmsg;
        else
          msg(MSG::VERBOSE) << endmsg;
      }
    }
    if (msgLvl(MSG::VERBOSE)) {
      if ((params.m_correctTime && good_time && non_zero_time) || pChannel->sizeTime() > 1) {
        msg(MSG::VERBOSE) << " OF_time = " << pChannel->uncorrTime()
                          << " corr_time = " << time << endmsg;
      }
    }
  }

  //**
  // Now store all TileCells
  //**
  for (unsigned int index = 0; index < allCells.size(); ++index) {

    TileCell * pCell = allCells[index];

    if (pCell) {      // cell exists

      if (m_maskBadChannels)
        if (maskBadChannels (drawerEvtStatus, DQstatus, pCell))
          ATH_MSG_VERBOSE ( "cell with id=" << m_tileID->to_string(pCell->ID(), -2)
                           << " bad channels masked, new energy=" << pCell->energy() );

      if (m_thresholdNotSet
          || (pCell->energy() > m_eThreshold
              && fabs(pCell->timeDiff()) < m_maxTimeDiff
              && pCell->time1() < m_maxTime && pCell->time1() > m_minTime
              && pCell->time2() < m_maxTime && pCell->time2() > m_minTime
              && pCell->qual1() > m_minChi2 && pCell->qual1() < m_maxChi2
              && pCell->qual2() > m_minChi2 && pCell->qual2() < m_maxChi2)) {

        coll->push_back(pCell); // store cell in container

      } else {

        //delete pCell; it's dangerous to delete cell, if it's in DataPool

      }

      allCells[index] = 0;             // clear pointer for next event
    } else if (m_fakeCrackCells) { // can be true only for full-size container

      pCell = tileCellsP.nextElementPtr();
      const CaloDetDescrElement* dde = m_tileMgr->get_cell_element(index);
      pCell->set(dde, dde->identify());

      if (m_tileID->section(pCell->ID()) == TileID::GAPDET) { // missing D4/E3/E4 cell

        int ind = m_tileID->module(pCell->ID()) + ((m_tileID->side(pCell->ID()) > 0) ? 0 : 64);
        if (EBdrawerPresent[ind]) {
          ++nFake;
          if (m_tileID->sample(pCell->ID()) == TileID::SAMP_E) {
            pCell->setEnergy(0.0, 0.0, TileID::LOWGAIN, CaloGain::INVALIDGAIN); // reset energy completely, indicate problem putting low gain
            pCell->setQuality(0, TileCell::MASK_BADCH, 0); // reset quality flag for first pmt
            pCell->setQuality(0, TileCell::MASK_BADCH, 1); // reset quality flag for second pmt
          } else {
            pCell->setEnergy(0.0, 0.0, TileID::LOWGAIN, TileID::LOWGAIN); // reset energy completely, indicate problem putting low gain
            pCell->setQuality(0, 0, 0); // reset quality flag for first pmt
            pCell->setQuality(0, 0, 1); // reset quality flag for second pmt
          }
          pCell->setTime(0.0); // reset time completely

          ATH_MSG_VERBOSE ( "adding fake cell with id=" << m_tileID->to_string(pCell->ID(), -2)
                           << " ene=" << pCell->energy()
                           << " status=" << (pCell->badcell() ? "bad" : "good") );

          coll->push_back(pCell); // store cell in container
        }
      }
    }
  }

  if (msgLvl(MSG::DEBUG)) {
    msg(MSG::DEBUG) << " nChan=" << nChan
                    << " RawChSum=" << eCh
                    << " nCell=" << nCell
                    << " n2=" << nTwo
                    << " nFake=" << nFake
                    << " eneTot=" << eCellTot;

    if (MBTSCells)
      msg(MSG::DEBUG) << " nMBTS=" << nMBTS
                      << " eMBTS=" << eMBTSTot;
    if (E4prCells)
      msg(MSG::DEBUG) << " nE4pr=" << nE4pr
                      << " eE4pr=" << eE4prTot;

    msg(MSG::DEBUG) << endmsg;
  }
}


bool TileCellBuilder::isChanDCSgood (int ros, int drawer, int channel) const
{
  if (!m_checkDCS) return true;
  TileDCSState::TileDCSStatus status = m_tileDCS->getDCSStatus(ros, drawer, channel);

  if (status > TileDCSState::WARNING) {
    ATH_MSG_DEBUG("Module=" << TileCalibUtils::getDrawerString(ros, drawer)
                  << " channel=" << channel
                  << " masking becasue of bad DCS status=" << status);
    return false;
  }

  return true;
}
