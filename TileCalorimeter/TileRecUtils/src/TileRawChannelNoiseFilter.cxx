/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Tile includes
#include "TileRecUtils/TileRawChannelNoiseFilter.h"
#include "TileIdentifier/TileHWID.h"
#include "TileEvent/TileRawChannel.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileMutableRawChannelContainer.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileConditions/TileInfo.h"

// Atlas includes
#include "AthenaKernel/errorcheck.h"
#include "Identifier/Identifier.h"
#include "GaudiKernel/ThreadLocalContext.h"


//========================================================
// constructor
TileRawChannelNoiseFilter::TileRawChannelNoiseFilter(const std::string& type,
    const std::string& name, const IInterface* parent)
    : base_class(type, name, parent)
    , m_tileHWID(0)
    , m_truncationThresholdOnAbsEinSigma(3.0) // 3 sigma of ADC HF noise by default
    , m_minimumNumberOfTruncatedChannels(0.6) // at least 60% of channels should be below threshold
    , m_useTwoGaussNoise(false) // do not use 2G - has no sense for ADC HF noise for the moment
    , m_useGapCells(false) // use gap cells for noise filter as all normal cells
    , m_tileInfo(0)
{

  declareProperty("TruncationThresholdOnAbsEinSigma", m_truncationThresholdOnAbsEinSigma);
  declareProperty("MinimumNumberOfTruncatedChannels", m_minimumNumberOfTruncatedChannels);
  declareProperty("UseTwoGaussNoise", m_useTwoGaussNoise);
  declareProperty("UseGapCells", m_useGapCells);
  declareProperty("MaxNoiseSigma", m_maxNoiseSigma = 5.0, "Channels with noise more than that value are igonred in calculation of correction");
  declareProperty("TileInfoName", m_infoName = "TileInfo");
}

//========================================================
// Initialize
StatusCode TileRawChannelNoiseFilter::initialize() {
  ATH_MSG_INFO("Initializing...");

  if (msgLvl(MSG::DEBUG)) {
      msg(MSG::DEBUG) << "TruncationThresholdOnAbsEinSigma = " 
                      << m_truncationThresholdOnAbsEinSigma << endmsg;
      msg(MSG::DEBUG) << "MinimumNumberOfTruncatedChannels = " 
                      << m_minimumNumberOfTruncatedChannels << endmsg;
      msg(MSG::DEBUG) << "UseTwoGaussNoise = " 
                      << ((m_useTwoGaussNoise)?"true":"false") << endmsg;
      msg(MSG::DEBUG) << "UseGapCells = " 
                      << ((m_useGapCells)?"true":"false") << endmsg;
  }

  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  //=== get TileCondToolEmscale
  ATH_CHECK( m_tileToolEmscale.retrieve() );

  //=== get TileCondToolNoiseSample
  ATH_CHECK( m_tileToolNoiseSample.retrieve() );

  //=== get TileBadChanTool
  ATH_CHECK( m_tileBadChanTool.retrieve() );

  //=== get TileInfo
  CHECK( detStore()->retrieve(m_tileInfo, m_infoName) );
  m_ADCmaskValueMinusEps = m_tileInfo->ADCmaskValue() - 0.01;  // indicates channels which were masked in background dataset

  ATH_CHECK( m_DQstatusKey.initialize() );

  return StatusCode::SUCCESS;
}

// ============================================================================
// process container
StatusCode
TileRawChannelNoiseFilter::process (TileMutableRawChannelContainer& rchCont, const EventContext& ctx) const
{
  ATH_MSG_DEBUG("in TileRawChannelNoiseFilter::process()");

  TileRawChannelUnit::UNIT rChUnit = rchCont.get_unit();
  std::string units[8] = { "ADC counts", "pC", "CspC", "MeV",
      "online ADC counts", "online pC", "online CspC", "online MeV" };

  if (rChUnit > TileRawChannelUnit::ADCcounts
      && rChUnit < TileRawChannelUnit::OnlineADCcounts) {

    ATH_MSG_ERROR( "Units in container is " << units[rChUnit] );
    ATH_MSG_ERROR( "Due to non-linear CIS constants noise filter is possible only with ADC counts ");
    ATH_MSG_ERROR( "Please, disable CIS calibration in optimal filter " );

    return StatusCode::FAILURE;
  }

  bool undoOnlCalib = (rChUnit > TileRawChannelUnit::OnlineADCcounts);
  ATH_MSG_VERBOSE( "Units in container is " << units[rChUnit] );

  // Now retrieve the TileDQStatus
  const TileDQstatus* DQstatus = SG::makeHandle (m_DQstatusKey, ctx).get();

  for (IdentifierHash hash : rchCont.GetAllCurrentHashes()) {
    TileRawChannelCollection* coll = rchCont.indexFindPtr (hash);

    /* Get drawer ID and build drawer index. */
    HWIdentifier drawer_id = m_tileHWID->drawer_id(coll->identify());
    int ros = m_tileHWID->ros(drawer_id);
    int drawer = m_tileHWID->drawer(drawer_id);
    unsigned int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);
    bool eb = (ros > 2);
    bool ebspD4 = ((ros == 3 && drawer == 14) || (ros == 4 && drawer == 17));
    bool ebNsp  = !ebspD4 && eb;
    bool ebspC10 = (ebNsp && ((drawer>37 && drawer<42) || (drawer>53 && drawer<58) ) );

    static const int maxChannelDrawer = 48; // number of channels in one drawer
    static const int maxChannel = 12; // number of channels per motherboard
    static const int maxMOB = 4; // number of motherboards in one drawer

    float calib[maxChannelDrawer];
    float commonmode[maxMOB];
    int nemptychan[maxMOB];
    int ngoodchan[maxMOB];
    int chanmap[maxChannelDrawer];
    memset(calib, 0, sizeof(calib));
    memset(commonmode, 0, sizeof(commonmode));
    memset(nemptychan, 0, sizeof(nemptychan));
    memset(ngoodchan, 0, sizeof(ngoodchan));
    memset(chanmap, 0, sizeof(chanmap));

    // iterate over all channels in a collection
    for (const TileRawChannel* rch : *coll) {
      HWIdentifier adc_id = rch->adc_HWID();
      //int index,pmt;
      //Identifier cell_id = rch->cell_ID_index(index,pmt);
      //if ( index == -1 ) continue; // this is to ignore disconnected channels - just for tests
      //if ( index < 0 )   continue; // this is to ingnore disconnected channels and MBTS - just for tests

      int chan = m_tileHWID->channel(adc_id);
      int gain = m_tileHWID->adc(adc_id);
      int mob = chan / maxChannel;
      bool empty = (eb && ( (chan > 41) || (chan > 23 && chan < 30) || (ebspD4 && chan < 3) ) );

      // use only good channel
      float ped=rch->pedestal();
      if (empty || ped > 59500. || (ped > m_ADCmaskValueMinusEps && ped < 39500.) // all bad patterns, ped=m_tileInfo->ADCmaskValue(), underflow, overflow (see TileRawChannelMaker.cxx for the logic)
          || m_tileBadChanTool->getAdcStatus(drawerIdx, chan, gain).isBad()
          || (!DQstatus->isAdcDQgood(ros, drawer, chan, gain))) continue;


      bool usechan = m_useGapCells ||  // always true if we want to use gap cells
                   ( ! ( ( ebNsp && (chan==0 || chan==1  || chan==12 || chan==13)) ||
                         ( ebspC10 && (chan==4 || chan==5)) ||
                         ( ebspD4 && (chan==18 || chan==19 || chan==12 || chan==13)) ) );

      ++chanmap[chan];
      // do not count good channels twice
      if (chanmap[chan] < 2 && usechan) ++ngoodchan[mob];
      // use only high gain
      if (gain != TileHWID::HIGHGAIN) continue;

      float amp = rch->amplitude();
      if (undoOnlCalib) {
        calib[chan] = m_tileToolEmscale->undoOnlCalib(drawerIdx, chan, gain, 1.0, rChUnit);
        amp *= calib[chan];
      } else {
        calib[chan] = 1.0;
      }

      
      if (usechan) {

        float noise_sigma = 1.5; // default value of HFN in high gain channel
        if (m_useTwoGaussNoise) {
          //float sigma1 = m_tileToolNoiseSample->getHfn1(drawerIdx, chan, gain, ctx);
          //float sigma2 = m_tileToolNoiseSample->getHfn2(drawerIdx, chan, gain, ctx);
          //float norm   = m_tileToolNoiseSample->getHfnNorm(drawerIdx, chan, gain, ctx);
          // still need to define noise_sigma in this case 
          // noise_sigma = ...
        } else {
          // take single gauss noise sigma from DB (high frequency noise)
          noise_sigma = m_tileToolNoiseSample->getHfn(drawerIdx, chan, gain, TileRawChannelUnit::ADCcounts, ctx);
        }
        
        float significance = 999.999;
        if ((noise_sigma != 0.0) 
            && (noise_sigma < m_maxNoiseSigma)
            /* && (!m_tileBadChanTool->getAdcStatus(drawerIdx, chan, gain).isNoisy()) */) {

          significance = fabs(amp / noise_sigma); // caluclate signal/noise ratio
        } else {
          --ngoodchan[mob]; // ignore completely channels with zero sigma
        }

        ATH_MSG_VERBOSE( "HWID " << m_tileHWID->to_string(adc_id)
                         << " calib " << 1. / calib[chan]
                         << " amp " << amp
                         << " noise " << noise_sigma
                         << " significance " << significance );

        if (significance > m_truncationThresholdOnAbsEinSigma) continue;

        commonmode[mob] += amp;
        ++nemptychan[mob];

      } else {

        ATH_MSG_VERBOSE( "HWID " << m_tileHWID->to_string(adc_id)
                         << " calib " << 1. / calib[chan]
                         << " amp " << amp
                         << " channel is not used" );
      }
      
    }

    int ncorr = 0;
    int nchmin = m_minimumNumberOfTruncatedChannels;

    for (int k = 0; k < maxMOB; k++) {

      if (m_minimumNumberOfTruncatedChannels < 1.0) {
        nchmin = ceil(m_minimumNumberOfTruncatedChannels * ngoodchan[k]);
        if (nchmin < 2) nchmin = 2;
      }

      if (nemptychan[k] >= nchmin) {
        commonmode[k] /= nemptychan[k];
        ++ncorr;

        ATH_MSG_VERBOSE( "ros " << ros
                        << " drawer " << std::setw(2) << drawer
                        << " mb " << k << " mean " << commonmode[k]
                        << " taken from " << nemptychan[k] << " channels"
                        << " nchgood " << ngoodchan[k]
                        << " nchmin " << nchmin );

      } else {
        if (msgLvl(MSG::VERBOSE)) {
          if (commonmode[k] != 0.0) {
            msg(MSG::VERBOSE) << "ros " << ros
                              << " drawer " << std::setw(2) << drawer
                              << " mb " << k
                              << " mean is zero instead of " << commonmode[k] << " / " << nemptychan[k]
                              << " nchgood " << ngoodchan[k]
                              << " nchmin " << nchmin
                              << endmsg;
          } else {
            msg(MSG::VERBOSE) << "ros "
                              << ros << " drawer " << std::setw(2) << drawer
                              << " mb " << k
                              << " mean is zero - nothing to correct"
                              << " nchgood " << ngoodchan[k]
                              << " nchmin " << nchmin
                              << endmsg;
          }
        }
        commonmode[k] = 0.0;
      }
    }

    if (ncorr == 0) continue; // nothing to correct

    // iterate over all channels in a collection again
    for (TileRawChannel* rch : *coll) {
      int chan = m_tileHWID->channel(rch->adc_HWID());
      int gain = m_tileHWID->adc(rch->adc_HWID());

      // use only good channel and high gain - for them calib was set to non-zero value above
      if (calib[chan] > 0.0 && gain == TileHWID::HIGHGAIN) {
        // correct amplitude directly in channel
        // (will change this to set() method once it is available in TileRawChannel)
        int mob = chan/maxChannel;
        if (undoOnlCalib)
          rch->setAmplitude (rch->amplitude() - commonmode[mob] / calib[chan]);
        else
          rch->setAmplitude (rch->amplitude() - commonmode[mob]);
        rch->setPedestal (rch->pedestal() + commonmode[mob]);
      }
    }
  }

  return StatusCode::SUCCESS;
}

// ============================================================================
// finalize
StatusCode TileRawChannelNoiseFilter::finalize() {
  return StatusCode::SUCCESS;
}

