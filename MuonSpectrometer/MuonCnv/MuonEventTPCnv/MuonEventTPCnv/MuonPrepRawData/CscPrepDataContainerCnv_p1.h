/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CSCPREPDATACONTAINERCNV_P1_H
#define CSCPREPDATACONTAINERCNV_P1_H

//-----------------------------------------------------------------------------
//
// file:   CscPrepDataContainerCnv_p1.h
//
//-----------------------------------------------------------------------------

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include "MuonPrepRawData/CscPrepDataContainer.h"
#include "MuonEventTPCnv/MuonPrepRawData/MuonPRD_Container_p1.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkEventCnvTools/IEventCnvSuperTool.h"


class MsgStream;
class StoreGateSvc;
class CscIdHelper;

namespace Muon{
    class CscPrepDataContainerCnv_p1 : public T_AthenaPoolTPCnvBase< Muon::CscPrepDataContainer, Muon::MuonPRD_Container_p1 >

    {
    public:
        typedef Muon::MuonPRD_Container_p1 PERS; 
        typedef Muon::CscPrepDataContainer TRANS;
        CscPrepDataContainerCnv_p1() = default;
        virtual void persToTrans(const PERS* persCont, TRANS* transCont, MsgStream &log); 
        virtual void transToPers(const TRANS* transCont, PERS* persCont, MsgStream &log);
        virtual Muon::CscPrepDataContainer* createTransient(const Muon::MuonPRD_Container_p1* persObj, MsgStream& log);
      private:
        StatusCode initialize(MsgStream &log);

        const MuonGM::CscReadoutElement* getReadOutElement(const Identifier& id ) const;
    const CscIdHelper *m_cscId{nullptr};
    StoreGateSvc *m_storeGate{nullptr};
    ToolHandle  < Trk::IEventCnvSuperTool >  m_eventCnvTool{"Trk::EventCnvSuperTool/EventCnvSuperTool"}; 
    bool m_isInitialized{false};
    };

}
#endif 

