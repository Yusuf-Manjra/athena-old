/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSTRACKINGGEOMETRYSVC_H
#define ACTSGEOMETRY_ACTSTRACKINGGEOMETRYSVC_H

// ATHENA
#include "AthenaBaseComps/AthService.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/EventContext.h"

// PACKAGE
#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/IActsTrackingVolumeBuilder.h"
#include "ActsGeometry/ActsLayerBuilder.h"

// ACTS
#include "Acts/Geometry/CylinderVolumeBuilder.hpp"

// STL
#include <map>

#include <tbb/concurrent_unordered_map.h>

namespace InDetDD {
  class InDetDetectorManager;
  class SiDetectorManager;
  class TRT_DetectorManager;
}

class TRT_ID;
class ActsAlignmentStore;
class BeamPipeDetectorManager;

class ActsDetectorElement;

namespace Acts {

class TrackingGeometry;
class CylinderVolumeHelper;
class ILayerBuilder;

class GeometryIdentifier;
class BinnedSurfaceMaterial;

}


class ActsTrackingGeometrySvc : public extends<AthService, IActsTrackingGeometrySvc> {
public:

  StatusCode initialize() override;
  //virtual StatusCode finalize() override;

  ActsTrackingGeometrySvc( const std::string& name, ISvcLocator* pSvcLocator );

  std::shared_ptr<const Acts::TrackingGeometry>
  trackingGeometry() override;

  void
  populateAlignmentStore(ActsAlignmentStore *store) const override;

  const ActsAlignmentStore*
  getNominalAlignmentStore() const override;

private:
  ActsLayerBuilder::Config
  makeLayerBuilderConfig(const InDetDD::InDetDetectorManager* manager);

  std::shared_ptr<const Acts::ILayerBuilder>
  makeStrawLayerBuilder(const InDetDD::InDetDetectorManager* manager);


  std::shared_ptr<Acts::TrackingVolume>
  makeSCTTRTAssembly(const Acts::GeometryContext& gctx, const Acts::ILayerBuilder& sct_lb,
      const Acts::ILayerBuilder& trt_lb, const Acts::CylinderVolumeHelper& cvh,
      const std::shared_ptr<const Acts::TrackingVolume>& pixel);

  Acts::CylinderVolumeBuilder::Config makeBeamPipeConfig(
      std::shared_ptr<const Acts::CylinderVolumeHelper> cvh) const;

  ServiceHandle<StoreGateSvc> m_detStore;
  const InDetDD::SiDetectorManager* p_pixelManager;
  const InDetDD::SiDetectorManager* p_SCTManager;
  const InDetDD::TRT_DetectorManager* p_TRTManager;
  const InDetDD::SiDetectorManager* p_ITkPixelManager;
  const InDetDD::SiDetectorManager* p_ITkStripManager;
  const BeamPipeDetectorManager* p_beamPipeMgr;

  std::shared_ptr<std::vector<std::shared_ptr<const ActsDetectorElement>>> m_elementStore;
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;

  const TRT_ID *m_TRT_idHelper;

  std::unique_ptr<const ActsAlignmentStore> m_nominalAlignmentStore{nullptr};

  Gaudi::Property<bool> m_useMaterialMap{this, "UseMaterialMap", false, ""};
  Gaudi::Property<bool> m_objDebugOutput{this, "ObjDebugOutput", false, ""};
  Gaudi::Property<std::string> m_materialMapInputFile{this, "MaterialMapInputFile", "", ""};
  Gaudi::Property<bool> m_buildBeamPipe{this, "BuildBeamPipe", false, ""};

  Gaudi::Property<std::vector<size_t>> m_barrelMaterialBins{this, "BarrelMaterialBins", {10, 10}};
  Gaudi::Property<std::vector<size_t>> m_endcapMaterialBins{this, "EndcapMaterialBins", {5, 20}};
  Gaudi::Property<std::vector<std::string>> m_buildSubdetectors{this, "BuildSubDetectors", {"Pixel", "SCT", "TRT", "Calo"}};

  ToolHandle<IActsTrackingVolumeBuilder> m_caloVolumeBuilder{this, 
      "CaloVolumeBuilder", "", "CaloVolumeBuilder"};

};



#endif
