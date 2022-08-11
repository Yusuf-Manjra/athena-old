/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_IACTSMATERIALJSONWRITERTOOL_H
#define ACTSGEOMETRY_IACTSMATERIALJSONWRITERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IAlgTool.h"

#include "ActsGeometry/ActsGeometryContext.h"

#include "Acts/Plugins/Json/MaterialMapJsonConverter.hpp"

namespace Acts {
  class TrackingGeometry;
}

class IActsMaterialJsonWriterTool : virtual public IAlgTool {
public:

  DeclareInterfaceID(IActsMaterialJsonWriterTool, 1, 0);

  virtual
  void
  write(const ActsGeometryContext& gctx, const Acts::MaterialMapJsonConverter::DetectorMaterialMaps& detMaterial) const = 0;

  virtual
  void
  write(const ActsGeometryContext& gctx, const Acts::TrackingGeometry& tGeometry) const = 0;

};


#endif
