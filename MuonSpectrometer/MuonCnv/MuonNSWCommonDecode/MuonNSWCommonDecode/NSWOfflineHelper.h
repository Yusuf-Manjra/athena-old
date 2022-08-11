/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_OFFLINE_HELPER_H_
#define _MUON_NSW_OFFLINE_HELPER_H_

#include <memory>

#include "MuonNSWCommonDecode/NSWDecodeHelper.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"
#include "MuonNSWCommonDecode/sTGCMapper.h"

namespace Muon
{
  namespace nsw
  {
    namespace helper
    {
      class NSWOfflineHelper
      {
       public:
        NSWOfflineHelper (Muon::nsw::NSWResourceId *res_id, uint16_t roc_vmm, uint16_t vmm_channel_number);

        virtual ~NSWOfflineHelper () {};

        // Direct access to offline parameters from elinkId

        bool is_large_station  () {return m_elinkId->is_large_station ();};

        int8_t  station_eta    () {return m_elinkId->station_eta ();};
        uint8_t station_phi    () {return m_elinkId->station_phi ();};
        uint8_t multi_layer    () {return m_elinkId->multi_layer ();};
        uint8_t gas_gap        () {return m_elinkId->gas_gap ();};

        // Offline parameters

        uint8_t  channel_type   ();
        uint16_t channel_number ();
        uint16_t vmm            () { return m_vmm; }
 
       private:
        Muon::nsw::NSWResourceId *m_elinkId {nullptr};

        uint16_t m_vmm  {0};
        uint16_t m_chan {0};
      };

      class NSWOfflineRobId
      {
       public:
	NSWOfflineRobId (const std::string &station_name, int8_t station_eta, uint8_t station_phi);
	virtual ~NSWOfflineRobId () {};

	uint32_t get_id () const {return m_sourceId;};

       private:
	uint32_t m_sourceId;
      };

      static const std::map <const std::pair <std::string, bool>, const uint32_t> s_station_to_detector_map =
      {
	{{"MM", true},  eformat::MUON_MMEGA_ENDCAP_A_SIDE},
	{{"MM", false}, eformat::MUON_MMEGA_ENDCAP_C_SIDE},
	{{"ST", true},  eformat::MUON_STGC_ENDCAP_A_SIDE},
	{{"ST", false}, eformat::MUON_STGC_ENDCAP_C_SIDE}
      };
    }
  }
}


//=====================================================================
inline Muon::nsw::helper::NSWOfflineHelper::NSWOfflineHelper(Muon::nsw::NSWResourceId *res_id, uint16_t roc_vmm, uint16_t vmm_channel_number)
: m_elinkId (res_id)
, m_vmm (roc_vmm)
, m_chan (vmm_channel_number) 
{
  // Conversion of an online vmm id (captured by the ROC, read from the fragment) 
  // to the offline id used in all mappings, and vice versa, since the conversion is symmetric.
  constexpr uint8_t vmmRemap[8] = { 2, 3, 0, 1, 5, 4, 6, 7 };  
  if (res_id->detId() == eformat::MUON_STGC_ENDCAP_A_SIDE || res_id->detId() == eformat::MUON_STGC_ENDCAP_C_SIDE)
    m_vmm = vmmRemap [roc_vmm];
};

#endif // _MUON_NSW_OFFLINE_HELPER_H_

