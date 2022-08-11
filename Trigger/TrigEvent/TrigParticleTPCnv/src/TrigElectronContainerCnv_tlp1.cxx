/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigParticleTPCnv/TrigElectronContainerCnv_tlp1.h"


//constructor
TrigElectronContainerCnv_tlp1::TrigElectronContainerCnv_tlp1(){

  // add the "main" base class converter (ie. TrigElectronContainerCnv)
  addMainTPConverter();

  // add all converters needed in the top level converter
  // do not change the order of adding converters
  addTPConverter( &m_trigElectronCnv );
  addTPConverter( &m_p4PtEtaPhiMCnv );


}

void TrigElectronContainerCnv_tlp1::setPStorage( TrigElectronContainer_tlp1* storage ){


   //for the base class converter
   setMainCnvPStorage( &storage->m_trigElectronContainerVec );

   //for all other converters defined in the base class
   m_trigElectronCnv.             setPStorage( &storage->m_trigElectronVec );
   m_p4PtEtaPhiMCnv.              setPStorage( &storage->m_p4PtEtaPhiMVec );

}

