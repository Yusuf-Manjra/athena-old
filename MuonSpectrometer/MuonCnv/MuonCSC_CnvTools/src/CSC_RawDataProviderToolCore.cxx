/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "CSC_RawDataProviderToolCore.h"

#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "ByteStreamCnvSvcBase/ByteStreamCnvSvcBase.h"
#include "ByteStreamData/ROBData.h"
#include "ByteStreamData/RawEvent.h"
#include "MuonIdHelpers/CscIdHelper.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

//================ Constructor =================================================

Muon::CSC_RawDataProviderToolCore::CSC_RawDataProviderToolCore(const std::string& t, const std::string& n, const IInterface* p) :
    AthAlgTool(t, n, p), m_robDataProvider("ROBDataProviderSvc", n), m_cabling("CSCcablingSvc", n) {}

//================ Initialisation =================================================

StatusCode Muon::CSC_RawDataProviderToolCore::initialize() {
    ATH_CHECK(m_cabling.retrieve());
    ATH_CHECK(m_robDataProvider.retrieve());
    ATH_MSG_INFO("Retrieved service " << m_robDataProvider);

    ATH_CHECK(m_idHelperSvc.retrieve());
    m_hid2re.set(&(*m_cabling), &m_idHelperSvc->cscIdHelper());

    // Retrieve decoder
    ATH_CHECK(m_decoder.retrieve());
    ATH_MSG_INFO("Retrieved tool " << m_decoder);

    ATH_CHECK(m_containerKey.initialize());
    ATH_CHECK(m_eventInfoKey.initialize());

    ATH_MSG_INFO("initialize() successful in " << name());
    return StatusCode::SUCCESS;
}

//============================================================================================

StatusCode Muon::CSC_RawDataProviderToolCore::convertIntoContainer(
    const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs, const EventContext& ctx,
    CscRawDataContainer& container) const {
    std::set<uint32_t> robIdSet;
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);

    ATH_MSG_DEBUG("Before processing numColls=" << container.numberOfCollections());

    ATH_MSG_DEBUG("vector of ROB ID to decode: size = " << vecRobs.size());

    for (const ROBFragment* frag : vecRobs) {
        uint32_t robid = frag->rod_source_id();

        // check if this ROBFragment was already decoded (EF case in ROIs
        if (!robIdSet.insert(robid).second) {
            ATH_MSG_DEBUG(" ROB Fragment with ID  " << std::hex << robid << std::dec << " already decoded, skip");
        } else {
            m_decoder->fillCollection(*eventInfo, *frag, container);
        }
    }

    ATH_MSG_DEBUG("After processing numColls=" << container.numberOfCollections());

    return StatusCode::SUCCESS;
}
