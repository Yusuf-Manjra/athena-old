/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPrepRawData/CscPrepData.h"
#include "MuonPrepRawData/CscPrepDataContainer.h"
#include "MuonEventTPCnv/MuonPrepRawData/CscPrepData_p1.h"
#include "MuonEventTPCnv/MuonPrepRawData/MuonPRD_Container_p1.h"

#include "MuonIdHelpers/CscIdHelper.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonEventTPCnv/MuonPrepRawData/CscPrepDataCnv_p1.h"
#include "MuonEventTPCnv/MuonPrepRawData/CscPrepDataContainerCnv_p1.h"

// Gaudi
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/MsgStream.h"

// Athena
#include "StoreGate/StoreGateSvc.h"



#include "AthAllocators/DataPool.h"



StatusCode Muon::CscPrepDataContainerCnv_p1::initialize(MsgStream &log) {
   // Do not initialize again:
    m_isInitialized=true;

   // Get Storegate, ID helpers, and so on
    ISvcLocator* svcLocator = Gaudi::svcLocator();
   // get StoreGate service
    StatusCode sc = svcLocator->service("StoreGateSvc", m_storeGate);
    if (sc.isFailure()) {
        log << MSG::FATAL << "StoreGate service not found !" << endmsg;
        return StatusCode::FAILURE;
    }

   // get DetectorStore service
    StoreGateSvc *detStore;
    sc = svcLocator->service("DetectorStore", detStore);
    if (sc.isFailure()) {
        log << MSG::FATAL << "DetectorStore service not found !" << endmsg;
        return StatusCode::FAILURE;
    } else {
        if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Found DetectorStore." << endmsg;
    }

   // Get the pixel helper from the detector store
    sc = detStore->retrieve(m_cscId);
    if (sc.isFailure()) {
        log << MSG::FATAL << "Could not get CSC ID helper !" << endmsg;
        return StatusCode::FAILURE;
    } else {
        if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Found the CSC ID helper." << endmsg;
    }

    if (m_eventCnvTool.retrieve().isFailure()) {
         log << MSG::FATAL << "Could not get DetectorDescription manager" << endmsg;
            return StatusCode::FAILURE;
    }

    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Converter initialized." << endmsg;
    return StatusCode::SUCCESS;
}
const MuonGM::CscReadoutElement* Muon::CscPrepDataContainerCnv_p1::getReadOutElement(const Identifier& id ) const {
    const Trk::ITrkEventCnvTool* cnv_tool = m_eventCnvTool->getCnvTool(id);
    if (!cnv_tool) return nullptr; 
    return dynamic_cast<const MuonGM::CscReadoutElement*>(cnv_tool->getDetectorElement(id));
}
void Muon::CscPrepDataContainerCnv_p1::transToPers(const Muon::CscPrepDataContainer* transCont,  Muon::MuonPRD_Container_p1* persCont, MsgStream &log) 
{

    // The transient model has a container holding collections and the
    // collections hold channels.
    //
    // The persistent model flattens this so that the persistent
    // container has two vectors:
    //   1) all collections, and
    //   2) all RDO
    //
    // The persistent collections, then only maintain indexes into the
    // container's vector of all channels. 
    //
    // So here we loop over all collection and add their channels
    // to the container's vector, saving the indexes in the
    // collection. 

    typedef Muon::CscPrepDataContainer TRANS;
    typedef ITPConverterFor<Trk::PrepRawData> CONV;

    CscPrepDataCnv_p1  chanCnv;
    TRANS::const_iterator it_Coll     = transCont->begin();
    TRANS::const_iterator it_CollEnd  = transCont->end();
    unsigned int collIndex = 0;
    unsigned int chanBegin = 0;
    unsigned int chanEnd = 0;
    int numColl = transCont->numberOfCollections();
    
   
    persCont->m_collections.resize(numColl);    
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG  << " Preparing " << persCont->m_collections.size() << "Collections" << endmsg;

    for (collIndex = 0; it_Coll != it_CollEnd; ++collIndex, ++it_Coll)  {
        // Add in new collection
        if (log.level() <= MSG::DEBUG) log << MSG::DEBUG  << " New collection" << endmsg;
        const Muon::CscPrepDataCollection& collection = (**it_Coll);
        chanBegin  = chanEnd;
        chanEnd   += collection.size();
        Muon::MuonPRD_Collection_p1& pcollection = persCont->m_collections[collIndex];
        pcollection.m_id    = collection.identify().get_identifier32().get_compact();
        pcollection.m_hashId = (unsigned int) collection.identifyHash();
        pcollection.m_begin = chanBegin;
        pcollection.m_end   = chanEnd;
        // Add in channels
        persCont->m_PRD.resize(chanEnd);
        for (unsigned int i = 0; i < collection.size(); ++i) {
            const Muon::CscPrepData* chan = collection[i];
            persCont->m_PRD[i + chanBegin] = toPersistent((CONV**)nullptr, chan, log );
        }
    }
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG  << " ***  Writing CscPrepDataContainer ***" << endmsg;
}

void  Muon::CscPrepDataContainerCnv_p1::persToTrans(const Muon::MuonPRD_Container_p1* persCont, Muon::CscPrepDataContainer* transCont, MsgStream &log) 
{

    // The transient model has a container holding collections and the
    // collections hold channels.
    //
    // The persistent model flattens this so that the persistent
    // container has two vectors:
    //   1) all collections, and
    //   2) all channels
    //
    // The persistent collections, then only maintain indexes into the
    // container's vector of all channels. 
    //
    // So here we loop over all collection and extract their channels
    // from the vector.


    Muon::CscPrepDataCollection* coll = nullptr;

    CscPrepDataCnv_p1  chanCnv;
    typedef ITPConverterFor<Trk::PrepRawData> CONV;
    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG  << " Reading " << persCont->m_collections.size() << "Collections" << endmsg;
    for (unsigned int icoll = 0; icoll < persCont->m_collections.size(); ++icoll) {

        // Create trans collection - is NOT owner of CscPrepData (SG::VIEW_ELEMENTS)
    // IDet collection don't have the Ownership policy c'tor
        const Muon::MuonPRD_Collection_p1& pcoll = persCont->m_collections[icoll];        
        Identifier collID(Identifier(pcoll.m_id));
        IdentifierHash collIDHash(IdentifierHash(pcoll.m_hashId));
        coll = new Muon::CscPrepDataCollection(collIDHash);
        coll->setIdentifier(Identifier(pcoll.m_id));
        unsigned int nchans           = pcoll.m_end - pcoll.m_begin;
        coll->resize(nchans);
        // Fill with channels
        for (unsigned int ichan = 0; ichan < nchans; ++ ichan) {
            const TPObjRef pchan = persCont->m_PRD[ichan + pcoll.m_begin];
            Muon::CscPrepData* chan = dynamic_cast<Muon::CscPrepData*>(createTransFromPStore((CONV**)nullptr, pchan, log ) );
            if (!chan) {
               log << MSG::ERROR << "AthenaPoolTPCnvIDCont::persToTrans: Cannot get CscPrepData!" << endmsg;
               continue;
            }
            const MuonGM::CscReadoutElement * de = getReadOutElement(chan->identify());
            chan->m_detEl = de;
            (*coll)[ichan] = chan;
        }

        // register the rdo collection in IDC with hash - faster addCollection
        StatusCode sc = transCont->addCollection(coll, collIDHash);
        if (sc.isFailure()) {
            throw std::runtime_error("Failed to add collection to ID Container");
        }
        if (log.level() <= MSG::DEBUG) {
            log << MSG::DEBUG << "AthenaPoolTPCnvIDCont::persToTrans, collection, hash_id/coll id = " << (int) collIDHash << " / " << 
                collID.get_compact() << ", added to Identifiable container." << endmsg;
        }
    }

    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG  << " ***  Reading CscPrepDataContainer" << endmsg;
}



//================================================================
Muon::CscPrepDataContainer* Muon::CscPrepDataContainerCnv_p1::createTransient(const Muon::MuonPRD_Container_p1* persObj, MsgStream& log) 
{
    if(!m_isInitialized) {
        if (this->initialize(log) != StatusCode::SUCCESS) {
            log << MSG::FATAL << "Could not initialize CscPrepDataContainerCnv_p1 " << endmsg;
            return nullptr;
        } 
    }
    std::unique_ptr<Muon::CscPrepDataContainer> trans(new Muon::CscPrepDataContainer(m_cscId->module_hash_max()));
    persToTrans(persObj, trans.get(), log);
    return(trans.release());
}


