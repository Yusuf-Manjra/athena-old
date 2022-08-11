/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// /***************************************************************************
//                           JEMJetSim.cxx  -  description
//                              -------------------
//     begin                : Wed Mar 12 2014
//     email                : Alan.Watson@CERN.CH
//  ***************************************************************************/
//
//
//================================================
// JEMJetSim class Implementation
// ================================================
//
//
//

// Utilities

// This algorithm includes
#include "JEMJetSim.h"
#include "TrigT1CaloUtils/JetEnergyModuleKey.h"

#include "TrigT1Interfaces/TrigT1CaloDefs.h"
#include "TrigT1CaloUtils/CoordToHardware.h"
#include "TrigConfL1Data/L1DataDef.h"

#include "TrigT1Interfaces/TrigT1Interfaces_ClassDEF.h"
#include "xAODTrigL1Calo/JetElementContainer.h"
#include "TrigT1CaloEvent/JEMTobRoI_ClassDEF.h"
#include "TrigT1CaloEvent/JetCMXData_ClassDEF.h"
#include "TrigT1CaloEvent/JetInput.h"



namespace LVL1{

using namespace TrigConf;

//--------------------------------
// Constructors and destructors
//--------------------------------

JEMJetSim::JEMJetSim
  ( const std::string& name, ISvcLocator* pSvcLocator )
    : AthAlgorithm( name, pSvcLocator ), 
      m_allTOBs(0),
      m_JetCMXData(0),
      m_JetTool("LVL1::L1JEMJetTools/L1JEMJetTools")
{
}


//---------------------------------
// initialise()
//---------------------------------

StatusCode JEMJetSim::initialize()
{
  ATH_CHECK( m_JetElementInputKey.initialize() );
  ATH_CHECK( m_JEMTobRoIOutputKey.initialize() );
  ATH_CHECK( m_JetCMXDataOutputKey.initialize() );
  ATH_CHECK( m_JetTool.retrieve() );
  return StatusCode::SUCCESS ;
}


//----------------------------------------------
// execute() method called once per event
//----------------------------------------------
//


StatusCode JEMJetSim::execute( )
{
    
  /* 
    Retrieve collection of JetElements 
    Fill a std::map of JetInputs from them
    Loop over crates/modules
      For each crate, loop over towers and find TOBs 
                      form backplane data objects
                      form daq objects    
  */

  //make a message logging stream

  ATH_MSG_DEBUG ( "starting JEMJetSim" ); 

  // Create containers for this event
  m_JetCMXData   = new DataVector<JetCMXData>;  //Container of backplane data objects
  m_allTOBs      = new DataVector<JEMTobRoI>;  // Container to hold all TOB RoIs in event

  // Retrieve the JetElementContainer
  auto rh = SG::makeHandle(m_JetElementInputKey);

  if (rh.isValid()) {
     const DataVector<xAOD::JetElement>* storedJEs = &(*rh);
    if ( true ) {
       // Check size of JetElementCollection - zero would indicate a problem
      if (storedJEs->size() == 0)
         ATH_MSG_WARNING("Empty JetElementContainer - looks like a problem" );
	
      // Form JetInputs and put them in a map
      std::map<int, JetInput*>* inputMap = new std::map<int, JetInput*> ;
      m_JetTool->mapJetInputs(storedJEs, inputMap);
	 
      // Loop over crates and modules
      for (int iCrate = 0; iCrate < 2; ++iCrate) {
	for (int iModule = 0; iModule < 16; ++iModule) {
	    
	   // For each module, find TOBs and backplane data
	   std::vector<unsigned int> jetCMXData;
	   m_JetTool->findJEMResults(inputMap,iCrate,iModule,m_allTOBs,jetCMXData);
	   // Push backplane data into output DataVectors
	   JetCMXData* bpData = new JetCMXData(iCrate,iModule,jetCMXData);
	   m_JetCMXData -> push_back(bpData);
	   
	} // loop over modules
      } // loop over crates
  
      /// Clean up JetInputs
      for (std::map<int, JetInput*>::iterator it = inputMap->begin(); it != inputMap->end(); ++it) {
        delete (*it).second;
      }
      delete inputMap;
  
    } // found TriggerTowers
    
    else ATH_MSG_WARNING("Error retrieving JetElements" );
  }
  else ATH_MSG_WARNING("No JetElementContainer at " << m_JetElementInputKey );
  
       
  // Store module readout and backplane results in the TES
  storeBackplaneTOBs();
  storeModuleRoIs();
  
  // Clean up at end of event
  m_JetCMXData  = 0;
  m_allTOBs    = 0;

  return StatusCode::SUCCESS;
}

/** place backplane data objects (CPM -> CMX) in StoreGate */
void LVL1::JEMJetSim::storeBackplaneTOBs() {
   
  size_t datasize = m_JetCMXData->size();

  // Store backplane data objects
  ///StatusCode sc = evtStore()->overwrite(m_JetCMXData, m_JetCMXDataLocation, true);
  const EventContext& ctx = Gaudi::Hive::currentContext();
  StatusCode sc = SG::makeHandle(m_JetCMXDataOutputKey, ctx).record( std::unique_ptr<DataVector<JetCMXData>>(m_JetCMXData) );
  m_JetCMXData = nullptr;

  if (sc.isSuccess()) {
    ATH_MSG_VERBOSE ( "Stored " << datasize
                      << " JetCMXData at " << m_JetCMXDataOutputKey );
  }
  else {
     ATH_MSG_ERROR("failed to write JetCMXData to  "
         << m_JetCMXDataOutputKey );
  } 
    
  return;

} //end storeBackplaneTOBs


/** place final ROI objects in the TES. */
void LVL1::JEMJetSim::storeModuleRoIs() {

   size_t n_jetsTobs = m_allTOBs->size();

   const EventContext& ctx = Gaudi::Hive::currentContext();
   StatusCode sc = SG::makeHandle(m_JEMTobRoIOutputKey, ctx).record( std::unique_ptr<DataVector<JEMTobRoI>>(m_allTOBs) );
   m_allTOBs = nullptr;
   
   if (sc.isSuccess()) {
      ATH_MSG_VERBOSE ( "Stored " << n_jetsTobs
                        << " Jet TOBs at " << m_JEMTobRoIOutputKey );
   }
   else {
      ATH_MSG_ERROR("failed to write JEMTobRoIs to  "
                    << m_JEMTobRoIOutputKey );
   } 

} //end storeModuleRoIs


} // end of LVL1 namespace bracket


