// -*- C++ -*-

/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SCT_PrepDataToxAOD.h
//   Header file for class SCT_PrepDataToxAOD
///////////////////////////////////////////////////////////////////

#ifndef SCT_PREPDATATOXAOD_H
#define SCT_PREPDATATOXAOD_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

#include <atomic>
#include <string>

class Identifier;
class SCT_ID;
class SCT_RDORawData;
class SiHit;

namespace InDet
{
  class SCT_Cluster;
}

class SCT_PrepDataToxAOD : public AthReentrantAlgorithm {

 public:
  // Constructor with parameters:
  SCT_PrepDataToxAOD(const std::string& name, ISvcLocator* pSvcLocator);

  // Basic algorithm methods:
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;
  /** Make this algorithm clonable. */
  virtual bool isClonable() const override { return true; };

 private:

  void addSDOInformation(xAOD::TrackMeasurementValidation* xprd,
                         const InDet::SCT_Cluster* prd,
                         const InDetSimDataCollection* sdoCollection) const;


  void addSiHitInformation(xAOD::TrackMeasurementValidation* xprd,
                           const InDet::SCT_Cluster* prd,
                           const std::vector<const SiHit*>* siHits) const;

  void findAllHitsCompatibleWithCluster(const InDet::SCT_Cluster* prd,
                                        const std::vector<const SiHit*>* siHits,
                                        std::vector<SiHit>& matchingHits) const;

  void addRDOInformation(xAOD::TrackMeasurementValidation*,
                         const InDet::SCT_Cluster*,
                         const std::map<Identifier,
                         const SCT_RDORawData*>& idToRAWDataMap) const;
 
  const SCT_ID* m_SCTHelper{nullptr};
  SG::ReadHandleKey<InDet::SCT_ClusterContainer> m_clustercontainer{this, "SiClusterContainer", "SCT_Clusters"};
  SG::ReadHandleKey<InDetSimDataCollection> m_SDOcontainer{this, "MC_SDOs", "SCT_SDO_Map"};
  SG::ReadHandleKey<SiHitCollection> m_sihitContainer{this, "MC_Hits", "SCT_Hits"};
  SG::ReadHandleKey<PRD_MultiTruthCollection> m_multiTruth{this, "PRD_MultiTruth", "PRD_MultiTruthSCT"};
  SG::ReadHandleKey<SCT_RDO_Container> m_rdoContainer{this, "SctRdoContainer", "SCT_RDOs"};
  SG::WriteHandleKey<xAOD::TrackMeasurementValidationContainer> m_xAodContainer{this, "SctxAodContainer", "SCT_Clusters"};
  SG::WriteHandleKey<std::vector<unsigned int>> m_xAodOffset{this, "SctxAodOffset", "SCT_ClustersOffsets"};

  // For P->T converter of SCT_Clusters
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

  BooleanProperty m_useTruthInfo{this, "UseTruthInfo", false};
  BooleanProperty m_writeRDOinformation{this, "WriteRDOinformation", true};
  BooleanProperty m_writeSDOs{this, "WriteSDOs", true};
  BooleanProperty m_writeSiHits{this, "WriteSiHits", true};
  
  // --- private members
  mutable std::atomic_bool m_firstEventWarnings{false};
};

#endif // SCT_PREPDATATOXAOD_H
