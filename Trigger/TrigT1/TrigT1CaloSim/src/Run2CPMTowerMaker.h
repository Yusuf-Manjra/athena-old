/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// ================================================
// Run2CPMTowerMaker class descriptionn
// ================================================



#ifndef RUN2CPMTOWERMAKER_H
#define RUN2CPMTOWERMAKER_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTrigL1Calo/CPMTowerContainer.h"
#include "xAODTrigL1Calo/CPMTowerAuxContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"

#include "TrigT1Interfaces/TrigT1CaloDefs.h"

#include "TrigT1CaloToolInterfaces/IL1CPMTowerTools.h"



//********************************************
//Default for parameters of this algorithm
//These can be overridden by job options.
//Descriptions are given in the class declaration
//********************************************

namespace LVL1 {

//                 Run2CPMTowerMaker class declaration
/*
 An "Athena algorithm" is something which gets scheduled and controlled
 by the framework In its simplest definition it is something which
  - gets called for each event
  - can get anything it wants out of the Transient Event Store"
  - can put anything it makes into the store
 It must inherit from the  AthAlgorithm base class
 */
/**
The Run2CPMTowerMaker class takes Trigger Towers from the TES and
forms CPMTowers, which it then places back into the TES.
The CPMTowers so formed are used for the bytestream simulation.
*/
class Run2CPMTowerMaker : public AthAlgorithm
{
  typedef xAOD::CPMTowerContainer     CPMTCollection;
  typedef xAOD::CPMTowerAuxContainer  CPMTAuxCollection;
 public:

  //-------------------------
  // Constructors/Destructors
  //
  // Athena requires that the constructor takes certain arguments
  // (and passes them directly to the constructor of the base class)
  //-------------------------

  Run2CPMTowerMaker( const std::string& name, ISvcLocator* pSvcLocator ) ;

  //------------------------------------------------------
  // Methods used by Athena to run the algorithm
  //------------------------------------------------------

  StatusCode initialize() ;
  StatusCode execute() ;

 private:

   ToolHandle<LVL1::IL1CPMTowerTools> m_CPMTowerTool;

  /** locations within the TES to store collections of JEs*/
  SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_triggerTowerKey{this, "TriggerTowerLocation", TrigT1CaloDefs::xAODTriggerTowerLocation};
  SG::WriteHandleKey<CPMTCollection>             m_cpmTowerKey{this, "CPMTowerLocation", TrigT1CaloDefs::CPMTowerLocation};


};

} // end of namespace bracket
#endif
