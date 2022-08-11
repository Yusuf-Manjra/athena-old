/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// CaloCellPosition2Ntuple.h
//

#ifndef CALOCONDPHYSALGS_CALOCELLPOSITION2NTUPLE_H
#define CALOCONDPHYSALGS_CALOCELLPOSITION2NTUPLE_H

#include <string>

// Gaudi includes

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "GaudiKernel/ITHistSvc.h"
#include "TTree.h"
#include "CxxUtils/checker_macros.h"

class CaloIdManager;
class CaloCell_ID;
namespace CaloRec {
  class CaloCellPositionShift;
}

class CaloCellPosition2Ntuple : public AthAlgorithm {

  public:
    //Gaudi style constructor and execution methods
    /** Standard Athena-Algorithm Constructor */
    CaloCellPosition2Ntuple(const std::string& name, ISvcLocator* pSvcLocator) 
      ATLAS_CTORDTOR_NOT_THREAD_SAFE;//DataHanlde is not thread safe
    /** Default Destructor */
    ~CaloCellPosition2Ntuple() 
      ATLAS_CTORDTOR_NOT_THREAD_SAFE; //DataHandle is not thread safe
    
    /** standard Athena-Algorithm method */
    virtual StatusCode          initialize ATLAS_NOT_THREAD_SAFE() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          execute() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          finalize() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          stop ATLAS_NOT_THREAD_SAFE() override;

    
  private:

  //---------------------------------------------------
  // Member variables
  //---------------------------------------------------
  ITHistSvc* m_thistSvc;

  const CaloCell_ID*       m_calo_id;

  const DataHandle<CaloRec::CaloCellPositionShift> m_cellPos;//DataHandle is marked as not thread safe
  std::string m_key;
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };

  int m_Hash;
  int m_OffId;
  float m_eta;
  float m_phi;
  int m_layer;
  float m_dx;
  float m_dy;
  float m_dz;
  float m_volume;
  TTree* m_tree;

};
#endif
