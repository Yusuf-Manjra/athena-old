/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 *
 * NAME:     TileTileBCOffsetFilter
 * PACKAGE:  offline/TileCalorimeter/TileRecUtils
 *
 * AUTHOR :  T. Davidek, A. Solodkov
 * CREATED:  Feb 2019
 *
 * PURPOSE:  mask the time offsets by +/-1 or +/-2 bunch crossings in the given
 *           Tile DMU
 *
 *  Input: TileRawChannelContainer
 *  Output: TileRawChannelContainer is modified
 *  Parameters:  none
 *
 ********************************************************************/

// Tile includes
#include "TileRecUtils/TileTimeBCOffsetFilter.h"
#include "TileIdentifier/TileHWID.h"
#include "TileEvent/TileRawChannel.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileIdentifier/TileRawChannelUnit.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileRecUtils/TileRawChannelBuilder.h"

// Atlas includes
#include "AthenaKernel/errorcheck.h"
#include "Identifier/HWIdentifier.h"
#include "GaudiKernel/ThreadLocalContext.h"


//========================================================
// constructor
TileTimeBCOffsetFilter::TileTimeBCOffsetFilter(const std::string& type,
    const std::string& name, const IInterface* parent)
    : base_class(type, name, parent)
    , m_tileHWID(nullptr)
    , m_cabling(nullptr)
{
  declareProperty("EneThreshold3", m_ene_threshold_3chan = 1000);
  declareProperty("EneThreshold1", m_ene_threshold_1chan = 3000);
  declareProperty("TimeThreshold", m_time_threshold_diff = 15);
  declareProperty("AverTimeEneThreshold", m_ene_threshold_aver_time = 500);
  declareProperty("RefTimeThreshold", m_time_threshold_ref_ch = 10);
  declareProperty("SampleDiffMaxMin_HG", m_sample_diffmaxmin_threshold_hg = 15);
  declareProperty("SampleDiffMaxMin_LG", m_sample_diffmaxmin_threshold_lg = -1);

  declareProperty("CheckDCS", m_checkDCS = false);
}

StatusCode TileTimeBCOffsetFilter::initialize() {
  ATH_MSG_INFO( "TileTimeBCOffsetFilter::initialize()" );

  if (msgLvl(MSG::DEBUG)) {
      msg(MSG::DEBUG) << "EneThreshold3 = " 
                      << m_ene_threshold_3chan << endmsg;
      msg(MSG::DEBUG) << "EneThreshold1 = " 
                      << m_ene_threshold_1chan << endmsg;
      msg(MSG::DEBUG) << "TimeThreshold = " 
                      << m_time_threshold_diff << endmsg;
      msg(MSG::DEBUG) << "AverTimeEneThreshold = "
                      << m_ene_threshold_aver_time << endmsg;
      msg(MSG::DEBUG) << "RefTimeThreshold = "
                      << m_time_threshold_ref_ch << endmsg;
      msg(MSG::DEBUG) << "SampleDiffMaxMin_HG = "
                      << m_sample_diffmaxmin_threshold_hg << endmsg;
      msg(MSG::DEBUG) << "SampleDiffMaxMin_LG = "
                      << m_sample_diffmaxmin_threshold_lg << endmsg;
      msg(MSG::DEBUG) << "CheckDCS = " 
                      << ((m_checkDCS)?"true":"false") << endmsg;
  }

  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  //=== get TileCondToolEmscale
  ATH_CHECK( m_tileToolEmscale.retrieve() );

  //=== get TileBadChanTool
  ATH_CHECK( m_tileBadChanTool.retrieve() );

  CHECK( m_tileDCS.retrieve(EnableTool{m_checkDCS}) );

  ATH_CHECK( m_cablingSvc.retrieve() );
  m_cabling = m_cablingSvc->cablingService();

  ATH_CHECK( m_DQstatusKey.initialize() );

  ATH_CHECK(m_digitsContainerKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode TileTimeBCOffsetFilter::finalize() {
  return StatusCode::SUCCESS;
}

StatusCode
TileTimeBCOffsetFilter::process (TileMutableRawChannelContainer& rchCont, const EventContext& ctx) const
{
  ATH_MSG_DEBUG("in TileTimeBCOffsetFilter::process()");

  // Now retrieve the TileDQstatus
  const TileDQstatus* DQstatus = SG::makeHandle (m_DQstatusKey, ctx).get();

  const char * part[5] = {"AUX","LBA","LBC","EBA","EBC"};
  const int nchan_dmu = 3; // number of channels in a single DMU

  TileRawChannelUnit::UNIT rchUnit = rchCont.get_unit();

  for (IdentifierHash hash : rchCont.GetAllCurrentHashes()) {
    TileRawChannelCollection* coll = rchCont.indexFindPtr (hash);

    if (coll->size() != TileCalibUtils::MAX_CHAN) {
      ATH_MSG_DEBUG("drawer container does not contain all channels");
      continue;
    }

    /* Get drawer ID and build drawer index. */
    HWIdentifier drawer_id = m_tileHWID->drawer_id(coll->identify());
    int ros = m_tileHWID->ros(drawer_id);
    int drawer = m_tileHWID->drawer(drawer_id);

    unsigned int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);
    std::vector<int> channel_time_ok(TileCalibUtils::MAX_CHAN,0);
    std::vector<int> bad_dmu;
    if (drawer_ok(drawerIdx,channel_time_ok,bad_dmu)) continue;
    if (msgLvl(MSG::VERBOSE)) {
      for (unsigned int ch=0; ch<TileCalibUtils::MAX_CHAN; ++ch) {
        if (channel_time_ok[ch]!=0) {
          ATH_MSG_VERBOSE( "Checking BCOffset in module " << part[ros]
                           << std::setw(2) << std::setfill('0') << drawer+1
                           << " channel " << std::setw(2) << std::setfill(' ') << ch );
        }
      }
    }

    // in loop below rely on the channel order 0..47 inside collection
    // print an ERROR and do nothing if order is wrong

    for (int dmu : bad_dmu) {
      ATH_MSG_VERBOSE( "Checking DMU " << dmu);
      int dmu_offset = dmu * nchan_dmu;

      /* the order is: [0] - channel on a different DMU, being a partner of
         one of the channels on this DMU. We call it reference channel.
         Indices [1]..[3] correspond to the channels on this DMU.
         There might be more reference channels in one DMU (e.g. LB DMU#0 has
         two partners on different DMU: ch=3 and ch=4), in that case we pick up
         that with the highest energy.*/
      bool ch_status[nchan_dmu+1];
      int ch_number[nchan_dmu+1];
      float ch_amp[nchan_dmu+1];
      float ch_time[nchan_dmu+1];
      bool ch_mask[nchan_dmu+1];

      // search for reference channels, picking that with highest energy
      int ch_p = -1, ch_0 = -1;
      float ch_p_amp = -1000000;
      for(int i=0; i < nchan_dmu; ++i) {
        int ch = i+dmu_offset;
        int ch_p_tmp = find_partner(ros,ch);
        if ((ch_p_tmp >= 0) && (ch_p_tmp/nchan_dmu != dmu)) { // partner on different DMU
          const TileRawChannel * rch = coll->at(ch_p_tmp);
          HWIdentifier adc_id = rch->adc_HWID();
          if (m_tileHWID->channel(adc_id) != ch_p_tmp) {
            ATH_MSG_ERROR("Channels in drawer collection are out of order");
            continue;
          }
          int gain = m_tileHWID->adc(adc_id);
          if (channel_time_ok[ch_p_tmp]==0 &&
              (! ch_masked_or_empty(ros,drawer,ch_p_tmp,gain,DQstatus))) {
            float amp = rch->amplitude();
            if (rchUnit != TileRawChannelUnit::OnlineMegaElectronVolts) {
              amp = m_tileToolEmscale->channelCalib(drawerIdx, ch_p_tmp, gain, amp, rchUnit, TileRawChannelUnit::MegaElectronVolts);
            }
            if (amp > ch_p_amp) {
              ch_p = ch_p_tmp;
              ch_p_amp = amp;
              ch_0 = ch;
            }
          }
        }
      }
      if (ch_p < 0) { // no suitable reference channel found, cannot do anything
        continue;
      }

      ch_number[0] = ch_p;
      ch_status[0] = channel_time_ok[ch_p];
      ch_number[1] = ch_0;
      ch_status[1] = channel_time_ok[ch_0];
      int index = 2;
      for(int i=0; i < nchan_dmu; ++i) {
        int ch = i+dmu_offset;
        if (ch != ch_0) {
          ch_number[index] = ch;
          ch_status[index] = channel_time_ok[ch];
          ++index;
        }
      }

      // init ch_amp, ch_time, ch_mask; ch_status already filled-in above
      int nprob = 0; // number of potentially problematic channels in the DMU
      for(int i=0; i <= nchan_dmu; ++i) {
        int ch = ch_number[i];
        const TileRawChannel * rch = coll->at(ch);
        HWIdentifier adc_id = rch->adc_HWID();
        if (m_tileHWID->channel(adc_id) != ch) {
          ATH_MSG_ERROR("Channels in drawer collection are out of order");
          continue;
        }
        int gain = m_tileHWID->adc(adc_id);
        if (ch_masked_or_empty(ros,drawer,ch,gain,DQstatus)) {
          ch_number[i] = -10;
          ch_amp[i] = 0;
          ch_time[i] = 0;
          ch_mask[i] = false;
        } else {
          ch_amp[i]    = rch->amplitude();
          if (rchUnit != TileRawChannelUnit::OnlineMegaElectronVolts) {
            ch_amp[i] = m_tileToolEmscale->channelCalib(drawerIdx, ch, gain, ch_amp[i], rchUnit, TileRawChannelUnit::MegaElectronVolts);
          }
          ch_time[i]   = rch->time();
          ch_mask[i]   = false;
          if ((i > 0) && (ch_status[i]!=0)) ++nprob;
        }
      }

      ATH_MSG_VERBOSE( "DMU " << dmu << " number of problems found " << nprob);
      if (nprob == 0) continue; // no potential problems, skipping

      float ene_threshold;
      if (nprob == 1) {
        ene_threshold = m_ene_threshold_1chan;
      } else {
        ene_threshold = m_ene_threshold_3chan;
      }
      /* Look at both reference and potentially affected channels in the given
         DMU. At least one channel of the four should be above threshold,
         and the time difference of affected channsl wrt reference channel
         should be above 15 ns. */
      bool ene_above = false;
      for(int i=0; i <= nchan_dmu; ++i) {
        ene_above = ene_above || ((ch_number[i] >= 0) && (std::abs(ch_amp[i]) > ene_threshold));
      }
      if (ene_above) { // at least 1 channel above the threshold
        ATH_MSG_VERBOSE("Absolute energy above threshold in at least one relevant channel");
        /* first check whether the times of 1-3 channels on the DMU are within
           15 ns, also calculate the average time, use only channels with energy above threshold) */
        bool time_dmu_same = true;
        float time_dmu_aver = 0;
        int n_dmu_aver = 0;
        for(int i=1; (i <= nchan_dmu) && time_dmu_same; ++i) {
          if (ch_number[i] < 0 || std::abs(ch_amp[i]) < m_ene_threshold_aver_time) continue;
          time_dmu_aver += ch_time[i];
          ++n_dmu_aver;
          for(int j=i+1; (j <= nchan_dmu) && time_dmu_same; ++j) {
            if (ch_number[j] >= 0 && std::abs(ch_amp[j]) > m_ene_threshold_aver_time) {
              time_dmu_same = time_dmu_same &&
                (std::abs(ch_time[i]-ch_time[j]) < m_time_threshold_diff);
            }
          }
        }
        if (time_dmu_same && (n_dmu_aver != 0)) {
          time_dmu_aver /= n_dmu_aver;
          ATH_MSG_VERBOSE( "Average time "<< time_dmu_aver);
          
          int expected_sign = 0;
          for(int i=1; i <= nchan_dmu; ++i) {
            if (ch_number[i] >= 0) {
              expected_sign = channel_time_ok[ch_number[i]];
              ATH_MSG_VERBOSE("Expected BCOffset sign in " << part[ros]
                              << std::setw(2) << std::setfill('0') << drawer+1
                              << ": " << expected_sign);
              break;
            }
          }
          bool time_offset = false; // if reference time is far from average DMU time
          switch (expected_sign) // correct bool expression for given expected sign of BCOffset in given DMU
          {
          case +1:
            time_offset = time_dmu_aver - ch_time[0] > m_time_threshold_diff;
            break;
          case -1:
            time_offset = ch_time[0] - time_dmu_aver > m_time_threshold_diff;
            break;
          }
          // masking channels with BCOffset
          if (time_offset && (std::abs(ch_time[0]) < m_time_threshold_ref_ch)) {  // BCOffset && reference time ~0 ns
            // calculate difference between maximal and minimal sample value in reference channel
            ATH_MSG_VERBOSE("Retrieving digits in " << part[ros]
                            << std::setw(2) << std::setfill('0') << drawer+1
                            << " ch " << std::setw(2) << std::setfill(' ') << ch_number[0]
                            << " run " << ctx.eventID().run_number()
                            << " evt " <<  ctx.eventID().event_number());
            float ref_maxmindiff = ref_digits_maxmindiff(ros, drawer, ch_number[0]);
            // set threshold for high and low gain
            float sample_threshold;
            const TileRawChannel * rch = coll->at(ch_number[0]);
            HWIdentifier adc_id = rch->adc_HWID();
            int gain = m_tileHWID->adc(adc_id);
            if (gain == 1) {
              sample_threshold = m_sample_diffmaxmin_threshold_hg;
            } else {
              sample_threshold = m_sample_diffmaxmin_threshold_lg;
            }
            sample_threshold = sample_threshold - 0.5; // samples are integers in float data type -> '-0.5' for float comparison
            if (ref_maxmindiff > sample_threshold) { // max - min sample should be above threshold
              for(int i=1; i <= nchan_dmu; ++i) {
                if (ch_number[i] >= 0) ch_mask[i] = true;
              }
            }
          }
        } else {
          ATH_MSG_VERBOSE( "Can not determine average time");
        }
      }
      // now apply mask to RawChannelContainer for channels spotted above
      for(int i=1; i<= nchan_dmu;++i) {
        if ((ch_number[i] >= 0) && ch_mask[i]) {
          ATH_MSG_DEBUG( "Masking " << part[ros]
                         << std::setw(2) << std::setfill('0') << drawer+1
                         << " ch " << std::setw(2) << std::setfill(' ') << ch_number[i]
                         << " ene " << ch_amp[i]
                         << " time " << ch_time[i]
                         << " ref_ene " << ch_amp[0]
                         << " ref_time " << ch_time[0]
                         << " run " << ctx.eventID().run_number()
                         << " evt " <<  ctx.eventID().event_number() );
          coll->at(ch_number[i])->setPedestal(fmod(coll->at(ch_number[i])->pedestal(),10000.)+240000.0);
        }
      }
    } // end-of-loop over all DMUs in the given drawer
  } // end-of loop over drawers

  return StatusCode::SUCCESS;
}

bool TileTimeBCOffsetFilter::drawer_ok(int drawerIdx,
                                       std::vector<int> & channel_time_ok,
                                       std::vector<int> & bad_dmu) const {
  /* checks whether the whole drawer is ok, i.e. none of its channel
     is potentially suffering from the +/-25 ns (or +/-50 ns) time shifts
     and fill vectors with status for every channel at the same time
  */
  bool status = true;
  int last_dmu = -1;
  const int nchan_dmu = 3; // number of channels in a single DMU
  for (unsigned int ch=0; ch<TileCalibUtils::MAX_CHAN; ++ch) {
    TileBchStatus chStatus = m_tileBadChanTool->getAdcStatus(drawerIdx, ch, 0);
    if (chStatus.isTimingDmuBcOffset()) { // channel timing is not stable
      int dmu = ch/nchan_dmu;
      if (dmu!=last_dmu) {
        bad_dmu.push_back(dmu);
        last_dmu=dmu;
      }
      if (chStatus.isTimingDmuBcOffsetNeg()) {
        channel_time_ok[ch] = -1;
      } else {
        channel_time_ok[ch] = +1;
      }
      status = false;
    }
  }
  return status;
}

bool TileTimeBCOffsetFilter::ch_masked_or_empty(int ros, int drawer, int channel, int gain,
                                                const TileDQstatus* DQstatus) const {
  // check if channel is connected
  int index, pmt;
  m_cabling->h2s_cell_id_index(ros, drawer, channel, index, pmt);
  if (index < 0) return true; // disconnected or MBTS

  // DB status
  int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);
  TileBchStatus chStatus = m_tileBadChanTool->getAdcStatus(drawerIdx, channel, gain);
  if (chStatus.isBad()) return true;

  // check DQstatus now
  bool bad = !(DQstatus->isAdcDQgood(ros, drawer, channel, gain)
               && isChanDCSgood(ros, drawer, channel));
  return bad;
}

bool TileTimeBCOffsetFilter::isChanDCSgood (int ros, int drawer, int channel) const
{
  bool good=true;

  if (m_checkDCS) {
    
    TileDCSState::TileDCSStatus status = m_tileDCS->getDCSStatus(ros, drawer, channel);

    if (status > TileDCSState::WARNING) {
      good=false;
      ATH_MSG_DEBUG("Module=" << TileCalibUtils::getDrawerString(ros, drawer)
                    << " channel=" << channel
                    << " masking becasue of bad DCS status=" << status);
    } 
  }

  return good;
}


int TileTimeBCOffsetFilter::find_partner(int ros, int ch) const {
  /* returns the "partner" channel, i.e. the channel reading the same cell
     as channel "ch" */
  int lbcells[48] = {
    -1,  4,  3,  2,  1,  8,  7,  6,  5,
    10,  9, 12, 11, 14, 13, 18, 17, 16, 15,
    20, 19, 22, 21, 26, 25, 24, 23,
    28, 27, 32, -1, -1, 29,
    34, 33, 38, 37, 36, 35,
    40, 39, 44, 47, -1, 41, 46, 45, 42
  };
  int ebcells[48] = {
    -1, -1,  3,  2,  5,  4,
     7,  6,  9,  8, 11, 10,
    -1, -1, 15, 14, 17, 16,
    -1, -1, 21, 20, 23, 22,
    -1, -1, -1, -1, -1, -1,
    35, 32, 31, -1, -1, 30,
    39, 38, 37, 36, 41, 40,
    -1, -1, -1, -1, -1, -1
  };
  if (ros < 3) // LB
    return lbcells[ch];
  else         // EB
    return ebcells[ch];
}

float TileTimeBCOffsetFilter::ref_digits_maxmindiff(int ros, int drawer, int ref_channel) const {
  /* Retrieve digits in reference channel of bad DMUs for calculation of
     sample_max - sample_min used later in loop over raw channel collections
     if samples are not in the collection the return value is set to -10. */
  float max_min_diff = 0.;
  // get named TileDigitsContaner from TES
  SG::ReadHandle<TileDigitsContainer> digitsContainer(m_digitsContainerKey);
  // Iterate over all collections (drawers) with digits
  for (const TileDigitsCollection *digitsCollection : *digitsContainer) {
    // Get drawer ID and build drawer index.
    HWIdentifier drawer_id = m_tileHWID->drawer_id(digitsCollection->identify());
    int digits_ros = m_tileHWID->ros(drawer_id);
    int digits_drawer = m_tileHWID->drawer(drawer_id);
    // retrieve reference channel digits
    if (digits_ros == ros && digits_drawer == drawer) {
      bool ref_ch_is_in_coll = false;
      for (uint i_ch = 0; i_ch < digitsCollection->size(); i_ch++) {
        const TileDigits * tdig = digitsCollection->at(i_ch);
        HWIdentifier adc_id = tdig->adc_HWID();
        int ch_number = m_tileHWID->channel(adc_id);
        if (ch_number == ref_channel) {
          std::vector<float> ref_samples = tdig->samples();

          float max_sample = *max_element(ref_samples.begin(), ref_samples.end());             
          float min_sample = *min_element(ref_samples.begin(), ref_samples.end());               
          max_min_diff = max_sample - min_sample;               
          ATH_MSG_VERBOSE("Reference channel: max - min sample = " << max_min_diff);
          ref_ch_is_in_coll = true;
        } 
      }
      if (!ref_ch_is_in_coll) {
        ATH_MSG_VERBOSE("Reference channel is not in the digits collection");
        max_min_diff = -10.;
      }
    }
  } // end of loop over drawers
  return max_min_diff;
}
