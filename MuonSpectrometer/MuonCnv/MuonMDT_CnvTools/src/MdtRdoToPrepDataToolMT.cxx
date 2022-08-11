/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtRdoToPrepDataToolMT.h"

Muon::MdtRdoToPrepDataToolMT::MdtRdoToPrepDataToolMT(const std::string& t, const std::string& n, const IInterface* p) :
    base_class(t, n, p) {
    declareProperty("MdtPrdContainerCacheKey", m_prdContainerCacheKey, "Optional external cache for the MDT PRD container");
}

StatusCode Muon::MdtRdoToPrepDataToolMT::initialize() {
    ATH_MSG_VERBOSE("Starting init");
    ATH_CHECK(MdtRdoToPrepDataToolCore::initialize());
    ATH_CHECK(m_prdContainerCacheKey.initialize(!m_prdContainerCacheKey.key().empty()));
    ATH_MSG_DEBUG("initialize() successful in " << name());
    return StatusCode::SUCCESS;
}

void Muon::MdtRdoToPrepDataToolMT::printPrepData() const {
    const EventContext& ctx = Gaudi::Hive::currentContext();

    SG::ReadHandleKey<Muon::MdtPrepDataContainer> k(m_mdtPrepDataContainerKey.key());
    k.initialize().ignore();
    printPrepDataImpl(SG::makeHandle(k, ctx).get());
}

Muon::MdtPrepDataContainer* Muon::MdtRdoToPrepDataToolMT::setupMdtPrepDataContainer(unsigned int /*sizeVectorRequested*/,
                                                                                    bool& fullEventDone) const {
    fullEventDone = false;

    SG::WriteHandle<Muon::MdtPrepDataContainer> handle(m_mdtPrepDataContainerKey);

    // Caching of PRD container
    const bool externalCachePRD = !m_prdContainerCacheKey.key().empty();
    if (!externalCachePRD) {
        // without the cache we just record the container
        StatusCode status = handle.record(std::make_unique<Muon::MdtPrepDataContainer>(m_idHelperSvc->mdtIdHelper().module_hash_max()));
        if (status.isFailure() || !handle.isValid()) {
            ATH_MSG_FATAL("Could not record container of MDT PrepData Container at " << m_mdtPrepDataContainerKey.key());
            return nullptr;
        }
        ATH_MSG_DEBUG("Created container " << m_mdtPrepDataContainerKey.key());
    } else {
        // use the cache to get the container
        SG::UpdateHandle<MdtPrepDataCollection_Cache> update(m_prdContainerCacheKey);
        if (!update.isValid()) {
            ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
            return nullptr;
        }
        StatusCode status = handle.record(std::make_unique<Muon::MdtPrepDataContainer>(update.ptr()));
        if (status.isFailure() || !handle.isValid()) {
            ATH_MSG_FATAL("Could not record container of MDT PrepData Container using cache " << m_prdContainerCacheKey.key() << " - "
                                                                                              << m_mdtPrepDataContainerKey.key());
            return nullptr;
        }
        ATH_MSG_DEBUG("Created container using cache for " << m_prdContainerCacheKey.key());
    }
    // Pass the container from the handle
    return handle.ptr();
}
