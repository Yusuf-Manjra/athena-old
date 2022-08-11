/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// /***************************************************************************
//                           JEMEnergySim.cxx  -  description
//                              -------------------
//     begin                : Monday 12 May 2014
//     email                : Alan.Watson@CERN.CH
//  ***************************************************************************/
//
//================================================
// JEMEnergySim class Implementation
// ================================================
//
//
//

// Utilities
#include <cmath>

// This algorithm includes
#include "JEMEnergySim.h"
#include "TrigT1CaloUtils/JetEnergyModuleKey.h" 

#include "TrigT1Interfaces/TrigT1Interfaces_ClassDEF.h"
#include "xAODTrigL1Calo/JetElementContainer.h"
#include "TrigT1CaloEvent/EnergyCMXData_ClassDEF.h"
#include "TrigT1CaloUtils/ModuleEnergy.h"


namespace LVL1 {


//--------------------------------
// Constructors and destructors
//--------------------------------

JEMEnergySim::JEMEnergySim
  ( const std::string& name, ISvcLocator* pSvcLocator )
    : AthAlgorithm( name, pSvcLocator ), 
      m_EtTool("LVL1::L1EtTools/L1EtTools"),
      m_jemContainer(0)
{}


//---------------------------------
// initialise()
//---------------------------------

StatusCode JEMEnergySim::initialize()
{
  ATH_CHECK( m_JetElementInputKey.initialize() );
  ATH_CHECK( m_jemEtSumsOutputKey.initialize() );
  ATH_CHECK( m_energyCMXDataOutputKey.initialize() );

  ATH_CHECK( m_EtTool.retrieve() );
  return StatusCode::SUCCESS ;
}


//----------------------------------------------
// execute() method called once per event
//----------------------------------------------
//

StatusCode JEMEnergySim::execute( )
{
   ATH_MSG_DEBUG ( "Executing" );

  // form module sums
  m_jemContainer = new DataVector<ModuleEnergy>;  

  auto rh = SG::makeHandle(m_JetElementInputKey);

  if (rh.isValid()) {
    const DataVector<xAOD::JetElement>* jetelements = &(*rh);
    
    // Warn if we find an empty container
    if (jetelements->size() == 0)
      ATH_MSG_WARNING("Empty JetElementContainer - looks like a problem" );
    
    m_EtTool->moduleSums(jetelements, m_jemContainer);
  }
  else ATH_MSG_WARNING("No JetElementCollection at " << m_JetElementInputKey );
    
  // Done the processing. Now form & save the various output data

  // For CMX simulation
  storeBackplaneData();
  
  // for Bytestream simulation
  storeJEMEtSums();
  
  cleanup();
 
  return StatusCode::SUCCESS ;
}


} // end of LVL1 namespace bracket

/** delete pointers etc. */
void LVL1::JEMEnergySim::cleanup(){

  delete m_jemContainer;
}

/** Form JEMEtSums and put into SG */
void LVL1::JEMEnergySim::storeJEMEtSums() {
  
  ATH_MSG_DEBUG("storeJEMEtSums running");
  
  JEMEtSumsCollection* JEMRvector = new  JEMEtSumsCollection;

  DataVector<ModuleEnergy>::iterator it;
  for ( it=m_jemContainer->begin(); it!=m_jemContainer->end(); ++it ) {
    std::vector<unsigned int> eX;
    eX.push_back((*it)->ex());
    std::vector<unsigned int> eY;
    eY.push_back((*it)->ey());
    std::vector<unsigned int> eT;
    eT.push_back((*it)->et());
    JEMEtSums* jemEtSums = new JEMEtSums((*it)->crate(), (*it)->module(),eT,eX,eY,0);
    JEMRvector->push_back(jemEtSums);
  }

  ATH_MSG_DEBUG( JEMRvector->size()<<" JEMEtSums objects are being saved");
  
  const EventContext& ctx = Gaudi::Hive::currentContext();
  StatusCode sc = SG::makeHandle(m_jemEtSumsOutputKey, ctx).record( std::unique_ptr<DataVector<JEMEtSums>>(JEMRvector) );
  if (sc != StatusCode::SUCCESS) {
    ATH_MSG_ERROR ( "Error registering JEMEtSums collection in TDS " );
  }


  // StatusCode sc = evtStore()->overwrite(JEMRvector, m_jemEtSumsLocation, true);
  // if (sc != StatusCode::SUCCESS) ATH_MSG_ERROR ( "Error registering JEMEtSums collection in TDS " );
  // else {
  //   StatusCode sc2 = evtStore()->setConst(JEMRvector);
  //   if (sc2 != StatusCode::SUCCESS) ATH_MSG_ERROR ( "error setting JEMResult vector constant" );
  // }
  
  // return;
}

/** Form EnergyCMXData and put into SG */
void LVL1::JEMEnergySim::storeBackplaneData() {
  
  ATH_MSG_DEBUG("storeBackplaneData running");
  
  EnergyCMXDataCollection* bpVector = new  EnergyCMXDataCollection;

  DataVector<ModuleEnergy>::iterator it;
  for ( it=m_jemContainer->begin(); it!=m_jemContainer->end(); ++it ) {
    EnergyCMXData* bpData = new EnergyCMXData((*it)->crate(), (*it)->module(),
					      (*it)->ex(),(*it)->ey(),(*it)->et());
    bpVector->push_back(bpData);
  }

  ATH_MSG_DEBUG( bpVector->size()<<" EnergyCMXData objects are being saved");

  const EventContext& ctx = Gaudi::Hive::currentContext();
  StatusCode sc = SG::makeHandle(m_energyCMXDataOutputKey, ctx).record( std::unique_ptr<DataVector<EnergyCMXData>>(bpVector) );
  if (sc != StatusCode::SUCCESS) {
    ATH_MSG_ERROR ( "Error registering EnergyCMXData collection in TDS " );
  }
  
  // StatusCode sc = evtStore()->overwrite(bpVector, m_energyCMXDataLocation, true);
  // if (sc != StatusCode::SUCCESS) ATH_MSG_ERROR ( "Error registering EnergyCMXData collection in TDS " );
  
  // return;
}



