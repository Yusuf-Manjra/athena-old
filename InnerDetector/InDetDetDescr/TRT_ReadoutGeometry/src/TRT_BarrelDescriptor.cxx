/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_ReadoutGeometry/TRT_BarrelDescriptor.h"

#include "TrkSurfaces/CylinderBounds.h"

#include "CLHEP/Units/SystemOfUnits.h"

namespace InDetDD {

TRT_BarrelDescriptor::TRT_BarrelDescriptor()
  : m_length(0),
    m_zDead(0),
    m_zPos(0),
    m_innerTubeRadius(2*CLHEP::mm), // FIXME: Hardwired for now!!
    m_f(nullptr),
    m_o(0),
    m_bounds()
{
}

TRT_BarrelDescriptor::~TRT_BarrelDescriptor()
{
}

void TRT_BarrelDescriptor::addStraw(double xPos, double yPos) {

  m_x.push_back(xPos);
  m_y.push_back(yPos);

}


void TRT_BarrelDescriptor::setStrawTransformField(const GeoXF::Function *xf, size_t offsetInto) {
  m_f=xf;
  m_o=offsetInto;
}

const Trk::SurfaceBounds & 
TRT_BarrelDescriptor::strawBounds() const
{
  if (not m_bounds) m_bounds.set(std::make_unique<Trk::CylinderBounds>(m_innerTubeRadius, 0.5*m_length));
  return *m_bounds;
}


}
