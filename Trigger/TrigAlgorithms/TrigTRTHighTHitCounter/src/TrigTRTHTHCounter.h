/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTRTHTHCOUNTER_H
#define TRIGTRTHTHCOUNTER_H

/** @class TrigTRTHTHCounter
    @author Jiri Masik <Jiri.Masik@hep.manchester.ac.uk>
    @brief TrigTRTHTHCounter gives back the number of TRT high-threshold hits 
           based on the code of HIPsFilterAlg by Sergey Burdin 
    @migrated from TrigTRTHTHCounter.h & cxx
*/

// standard stuff
#include <string>
#include <vector>

//Gaudi
#include "GaudiKernel/ToolHandle.h"

// Base class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "CxxUtils/phihelper.h"
#include "InDetIdentifier/TRT_ID.h"
#include "Identifier/IdentifierHash.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h" 

#include "xAODTrigRinger/TrigRNNOutput.h"
#include "xAODTrigRinger/TrigRNNOutputContainer.h"
#include "xAODTrigRinger/TrigRNNOutputAuxContainer.h"

#include "AthenaMonitoringKernel/GenericMonitoringTool.h"


class TrigTRTHTHCounter: public AthReentrantAlgorithm {
 public:
  
  TrigTRTHTHCounter(const std::string& name, ISvcLocator* pSvcLocator); 

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:

  const TRT_ID *m_trtHelper{nullptr};     //!<  TRT ID helper

  float m_maxCaloEta{1.7};

  //Gaudi::Property<std::string> m_trtDCContainerName {this,  "TRT_DC_ContainerName", "TRT_TrigDriftCircles" , " "};
  Gaudi::Property<float> m_etaHalfWidth {this,  "EtaHalfWidth", 0.1 , "subsection of RoI to retrieve fewer TRT hits"};
  Gaudi::Property<float> m_phiHalfWidth {this,  "PhiHalfWidth", 0.1 , "subsection of RoI to retrieve fewer TRT hits"};
  Gaudi::Property<bool> m_doFullScan {this,  "doFullScan", false , "Whether to use all RoI"};
  Gaudi::Property<float> m_roadWidth {this,  "RoadWidth", 4. , "Width of road in mm"};
  Gaudi::Property<int> m_nBinCoarse {this,  "nBinCoarse", 14 , "Number of coarse bins used while phi centering"};
  Gaudi::Property<int> m_nBinFine {this,  "nBinFine", 14 , "Number of fine bins used while phi centering"};
  Gaudi::Property<float> m_wedgeMinEta {this,  "WedgeMinEta", 0  , "Min eta for wedge algorithm"};
  Gaudi::Property<float> m_roadMaxEta {this,  "RoadMaxEta", 1.06 , "Max eta for road algorithm (barrel only)"};
  Gaudi::Property<int> m_wedgeNBin {this,  "WedgeNBin", 5 , "Number of fine bins to consider in the wedge algorithm"};

  SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey { this, 
      "RoIs",                             // property name
      "rois",                                                        // default value of StoreGate key
      "input RoI Collection name"};

  SG::ReadHandleKey<InDet::TRT_DriftCircleContainer> m_trtDCContainerKey{this, 
      "TRT_DC_ContainerName", 
      "TRT_DriftCircleContainer", 
      "TRT DriftCircle Container"};


  SG::WriteHandleKey<xAOD::TrigRNNOutputContainer> m_trigRNNOutputKey{ this,
      "RNNOutputName",                  // property name
      "RNNOutput",                      // default value of StoreGate key
      "output RNN container name "};

  ToolHandle< GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };

};

#endif
