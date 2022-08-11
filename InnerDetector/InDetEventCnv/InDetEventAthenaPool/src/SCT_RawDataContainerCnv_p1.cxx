/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "SCT_RawDataContainerCnv_p1.h"


#include "MsgUtil.h"
#include "SCT1_RawDataCnv_p1.h"
#include "SCT3_RawDataCnv_p1.h"

#include "InDetEventAthenaPool/SCT3_RawData_p1.h"
#include "InDetEventAthenaPool/InDetRawDataCollection_p1.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetRawData/SCT_RDO_Collection.h"
#include "InDetRawData/SCT3_RawData.h"

#include <memory>

void SCT_RawDataContainerCnv_p1::transToPers(const SCT_RDO_Container* transCont, SCT_RawDataContainer_p1* persCont, MsgStream& log)
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

  using TRANS = SCT_RDO_Container;

  SCT1_RawDataCnv_p1  chan1Cnv;
  SCT3_RawDataCnv_p1  chan3Cnv;
  TRANS::const_iterator it_Coll     = transCont->begin();
  TRANS::const_iterator it_CollEnd  = transCont->end();
  unsigned int collIndex;
  unsigned int chanBegin = 0;
  unsigned int chanEnd = 0;
  int numColl = transCont->numberOfCollections();
  persCont->m_collections.resize(numColl);
  
  for (collIndex = 0; it_Coll != it_CollEnd; ++collIndex, ++it_Coll)  {
    // Add in new collection
    const SCT_RDO_Collection& collection = (**it_Coll);
    chanBegin  = chanEnd;
    chanEnd   += collection.size();
    InDetRawDataCollection_p1& pcollection = persCont->m_collections[collIndex];
    pcollection.m_id    = collection.identify().get_compact();
    pcollection.m_hashId = static_cast<unsigned int>(collection.identifyHash());
    pcollection.m_begin = chanBegin;
    pcollection.m_end   = chanEnd;
    // Add in channels
    if (m_type == 1) {
      persCont->m_rawdata.resize(chanEnd);
      for (unsigned int i = 0; i < collection.size(); ++i) {
        InDetRawData_p1* pchan = &(persCont->m_rawdata[i + chanBegin]);
        const SCT1_RawData* chan = dynamic_cast<const SCT1_RawData*>(collection[i]);
        if (chan) {
          chan1Cnv.transToPers(chan, pchan, log);
        }
      }
    } else if (m_type == 3) {
      persCont->m_sct3data.resize(chanEnd);
      for (unsigned int i = 0; i < collection.size(); ++i) {
        SCT3_RawData_p1* pchan = &(persCont->m_sct3data[i + chanBegin]);
        const SCT3_RawData* chan = dynamic_cast<const SCT3_RawData*>(collection[i]);
        if (chan) {
          chan3Cnv.transToPers(chan, pchan, log);
        }
      }
    }
  }
}

void  SCT_RawDataContainerCnv_p1::persToTrans(const SCT_RawDataContainer_p1* persCont, SCT_RDO_Container* transCont, MsgStream& log)
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

  SCT1_RawDataCnv_p1  chan1Cnv;
  SCT3_RawDataCnv_p1  chan3Cnv;
  // check for the type of the contained objects:
  //
  if (persCont->m_rawdata.size() !=0 && persCont->m_sct3data.size() != 0) {
    log << MSG::FATAL << "The collection has mixed SCT1 and SCT3 elements, this is not allowed " << endmsg;
  }
  if (persCont->m_rawdata.size() != 0 ) m_type = 1;
  if (persCont->m_sct3data.size() != 0 ) m_type = 3;

  for (unsigned int icoll = 0; icoll < persCont->m_collections.size(); ++icoll) {
    // Create trans collection - in NOT owner of SCT_RDO_RawData (SG::VIEW_ELEMENTS)
    // IDet collection don't have the Ownership policy c'tor
    const InDetRawDataCollection_p1& pcoll = persCont->m_collections[icoll];
    Identifier collID(pcoll.m_id);
    IdentifierHash collIDHash(IdentifierHash(pcoll.m_hashId));
    std::unique_ptr<SCT_RDO_Collection> coll = std::make_unique<SCT_RDO_Collection>(collIDHash);
    coll->setIdentifier(collID);
    unsigned int nchans = pcoll.m_end - pcoll.m_begin;
    coll->resize(nchans);
    // Fill with channels
    for (unsigned int ichan = 0; ichan < nchans; ++ ichan) {
      if (m_type == 1) {
        const InDetRawData_p1* pchan = &(persCont->m_rawdata[ichan + pcoll.m_begin]);
        std::unique_ptr<SCT1_RawData> chan = std::make_unique<SCT1_RawData>();
        chan1Cnv.persToTrans(pchan, chan.get(), log);
        (*coll)[ichan] = chan.release();
      } else if (m_type == 3) {
        const SCT3_RawData_p1* pchan = &(persCont->m_sct3data[ichan + pcoll.m_begin]);
        std::unique_ptr<SCT3_RawData> chan = std::make_unique<SCT3_RawData>();
        chan3Cnv.persToTrans(pchan, chan.get(), log);
        (*coll)[ichan] = chan.release();
      }
    }

    // register the rdo collection in IDC with hash - faster addCollection
    StatusCode sc = transCont->addCollection(coll.release(), collIDHash);
    if (sc.isFailure()) {
      throw std::runtime_error("Failed to add collection to ID Container");
    }
    MSG_VERBOSE(log,"AthenaPoolTPCnvIDCont::persToTrans, collection, hash_id/coll id = "
                << collIDHash.value() << " / " << collID.get_compact() << ", added to Identifiable container.");
  }
}

//================================================================
SCT_RDO_Container* SCT_RawDataContainerCnv_p1::createTransient(const SCT_RawDataContainer_p1* persObj, MsgStream& log) {
  std::unique_ptr<SCT_RDO_Container> trans(std::make_unique<SCT_RDO_Container>(m_sctId->wafer_hash_max()));
  persToTrans(persObj, trans.get(), log);
  return trans.release();
}
