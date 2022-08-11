/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOPIXELSICRYSTAL_H
#define GEOPIXELSICRYSTAL_H

#include "Identifier/Identifier.h"
#include "GeoVPixelFactory.h"

#include <memory>

class GeoLogVol;

namespace InDetDD {
  class SiDetectorDesign;
  class PixelDiodeMatrix;
}

class GeoPixelSiCrystal : public GeoVPixelFactory {
 public:
  GeoPixelSiCrystal(InDetDD::PixelDetectorManager* ddmgr,
                    PixelGeometryManager* mgr,
                    bool isBLayer, bool isModule3D=false);
  virtual GeoVPhysVol* Build() override;
  inline Identifier getID();

  bool GetModule3DFlag() { return m_isModule3D; };

 private:
  std::shared_ptr<const InDetDD::PixelDiodeMatrix> makeMatrix(double phiPitch, double etaPitch, double etaPitchLong, double etaPitchLongEnd,
					 int circuitsPhi, int circuitsEta, int diodeRowPerCirc, int diodeColPerCirc);
  Identifier m_id;
  const InDetDD::SiDetectorDesign* m_design;
  bool m_isBLayer = false;
  bool m_isModule3D = false;
};
//
// Add this method to store the ID in the factory. This is used by the
// module factory to pass it to the alignment class.
//
Identifier GeoPixelSiCrystal::getID() {return m_id;}
#endif
