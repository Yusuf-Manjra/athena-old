/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

 /***************************************************************************
                           CPMSim.h  -  description
                              -------------------
     begin                : Wed Mar 05 2014
     email                : Alan.Watson@CERN.CH
  ***************************************************************************/

 /***************************************************************************
  *                                                                         *
  *                                                                         *
  ***************************************************************************/
 #ifndef TRIGT1CALOSIM_CPMSIM_H
 #define TRIGT1CALOSIM_CPMSIM_H

 // STL
 #include <string>
 #include <vector>

 // Athena/Gaudi
 #include "AthenaBaseComps/AthReentrantAlgorithm.h"
 #include "GaudiKernel/ServiceHandle.h"
 #include "GaudiKernel/ToolHandle.h"
 
 #include "xAODTrigL1Calo/CPMTowerContainer.h"
 #include "AthContainers/DataVector.h"
 #include "GaudiKernel/DataSvc.h"
 #include "StoreGate/ReadHandleKey.h"
 #include "StoreGate/WriteHandleKey.h"
 #include "StoreGate/WriteHandleKeyArray.h"
 
 #include "TrigT1Interfaces/TrigT1CaloDefs.h"

 // LVL1 Calo Trigger
 #include "TrigT1CaloToolInterfaces/IL1CPMTools.h"
 #include "TrigConfData/L1Menu.h"

 // For RoI output 
 #include "TrigT1Interfaces/TrigT1Interfaces_ClassDEF.h"
 #include "TrigT1Interfaces/SlinkWord.h"

 #include "TrigT1CaloEvent/CPMCMXData.h"
 #include "TrigT1CaloEvent/CPMTobRoI.h"


 namespace LVL1 {

   //using namespace TrigConf;

 class CPMTower;
 class CPMTobRoI;
 class CPMCMXData;

   //Doxygen Class definition below:
   /**
  The algorithm responsible for simulating the Em/tau calo trigger.
   */
 class CPMSim : public AthReentrantAlgorithm
 {

  public:

   //-------------------------
   // Constructors/Destructors
   //
   // Athena requires that the constructor takes certain arguments
   // (and passes them directly to the constructor of the base class)
   //-------------------------

   CPMSim( const std::string& name, ISvcLocator* pSvcLocator ) ;

   //------------------------------------------------------
   // Methods used by Athena to run the algorithm
   //------------------------------------------------------

   virtual StatusCode initialize() override;
   virtual StatusCode execute(const EventContext& ctx) const override;

 private: // Private methods
#if 0
   /** Simulate Slink data for RoIB input */
   StatusCode storeSlinkObjects (const DataVector<CPMTobRoI>& allTOBs,
                                 const EventContext& ctx) const;
  
  /** adds slink header */
  void addHeader (DataVector<LVL1CTP::SlinkWord>& slink,
                  unsigned int subDetID,
                  unsigned int moduleId,
                  const EventContext& ctx) const;
  /** add Slink tail */
  void addTail (DataVector<LVL1CTP::SlinkWord>& slink,
                unsigned int numberOfDataWords) const;
  /** creates a new SlinkWord object with the passed word, and returns a pointer.*/
  std::unique_ptr<LVL1CTP::SlinkWord> getWord(unsigned int tword) const;
#endif

  
 private: // Private attributes

   SG::ReadHandleKey<TrigConf::L1Menu>  m_L1MenuKey{ this, "L1TriggerMenu", "DetectorStore+L1TriggerMenu", "L1 Menu" };

   /** Where to store the CPMTowers */
   SG::ReadHandleKey<xAOD::CPMTowerContainer> m_CPMTowerLocation
   { this, "CPMTowerLocation", TrigT1CaloDefs::CPMTowerLocation, "" };
   /** Locations of outputs in StoreGate */
   SG::WriteHandleKey<DataVector<CPMTobRoI> > m_CPMTobRoILocation
   { this, "CPMTobRoILocation", TrigT1CaloDefs::CPMTobRoILocation, "" };
   SG::WriteHandleKey<DataVector<CPMCMXData> > m_CPMCMXDataLocation
   { this, "CPMCMXDataLocation", TrigT1CaloDefs::CPMCMXDataLocation, "" };
   
   /** The essentials - data access, configuration, tools */
   ToolHandle<LVL1::IL1CPMTools> m_CPMTool;
};

 } // end of namespace bracket


 #endif
