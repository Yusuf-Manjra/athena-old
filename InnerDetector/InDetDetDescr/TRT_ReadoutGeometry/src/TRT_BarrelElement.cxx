/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "Identifier/Identifier.h"
#include "InDetReadoutGeometry/SurfaceCache.h"
#include "TRT_ReadoutGeometry/TRT_BarrelElement.h"
#include "TRT_ReadoutGeometry/TRT_Conditions.h"

#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/PlaneSurface.h"

#include "InDetIdentifier/TRT_ID.h"

#include "TRT_ConditionsData/ExpandedIdentifier.h"
#include "TRT_ConditionsData/StrawDxContainer.h"

#include "GeoPrimitives/CLHEPtoEigenConverter.h"

#include "GeoModelUtilities/GeoAlignmentStore.h"


namespace InDetDD {

TRT_BarrelElement::TRT_BarrelElement(const GeoVFullPhysVol* volume,
                                     const TRT_BarrelDescriptor* descriptor,
                                     bool isPositive,
                                     unsigned int modIndex,
                                     unsigned int phiIndex,
                                     unsigned int strawLayIndex,
                                     const TRT_ID* idHelper,
                                     const TRT_Conditions* conditions,
                                     const GeoAlignmentStore* geoAlignStore)
  : TRT_BaseElement(volume,
                    idHelper->layer_id((isPositive ? 1 : -1),
                                       phiIndex,
                                       modIndex,
                                       strawLayIndex),
                    idHelper,
                    conditions,
                    geoAlignStore)
  , m_code(isPositive, modIndex, phiIndex, strawLayIndex)
  , m_descriptor(descriptor)
  , m_nextInPhi(nullptr)
  , m_previousInPhi(nullptr)
  , m_nextInR(nullptr)
  , m_previousInR(nullptr)

{
  m_nstraws = m_descriptor->nStraws();
  m_strawSurfaces.resize(m_nstraws);
  m_strawSurfacesCache.resize(m_nstraws);
}

TRT_BarrelElement::TRT_BarrelElement(const TRT_BarrelElement& right,
                                     const GeoAlignmentStore* geoAlignStore)
  : TRT_BaseElement(right, geoAlignStore)
  , m_code(right.m_code)
  , m_descriptor(right.m_descriptor)
  , m_nextInPhi(right.m_nextInPhi)
  , m_previousInPhi(right.m_previousInPhi)
  , m_nextInR(right.m_nextInR)
  , m_previousInR(right.m_previousInR)
{
  m_nstraws = right.m_nstraws;
  m_strawSurfaces.resize(m_nstraws);
  m_strawSurfacesCache.resize(m_nstraws);
}

const TRT_BarrelConditions * TRT_BarrelElement::getConditionsData() const
{
  return nullptr;
}

const TRT_BarrelDescriptor * TRT_BarrelElement::getDescriptor() const
{
  return m_descriptor;
}

void  TRT_BarrelElement::setNextInPhi(const TRT_BarrelElement *element)
{
  m_nextInPhi=element;
}

void  TRT_BarrelElement::setPreviousInPhi(const TRT_BarrelElement *element)
{
  m_previousInPhi=element;
}

void  TRT_BarrelElement::setNextInR(const TRT_BarrelElement *element)
{
  m_nextInR=element;
}

void  TRT_BarrelElement::setPreviousInR(const TRT_BarrelElement *element)
{
  m_previousInR=element;
}



HepGeom::Transform3D TRT_BarrelElement::calculateStrawTransform(int straw) const
{
  // NB The tranformation to a straw is reconstructed here precisely as
  // it was ... hopefully... in the factory.  One could eliminate this
  // requirement and make the code a little more robust in this regard but
  // at the cost of doubling the descriptors.  (One descriptor now suffices
  // for both positive and negative endcaps).
  //std::cout << "In calculateStrawTransform" << std::endl;

  const GeoXF::Function *f= m_descriptor->getStrawTransform();
  if (f) {
    size_t offsetInto = m_descriptor->getStrawTransformOffset();
    double zPos = -m_descriptor->strawZPos();
    double zAng =  m_code.isPosZ() ? M_PI : 0;
    return  Amg::EigenTransformToCLHEP(getMaterialGeom()->getAbsoluteTransform()*((*f)(straw+offsetInto)))
      * HepGeom::RotateY3D(zAng)*HepGeom::TranslateZ3D(zPos)
      * calculateLocalStrawTransform(straw);
    ////return  conditions()->solenoidFrame()
    ////  * getMaterialGeom()->getAbsoluteTransform()*((*f)(straw+offsetInto))
    ////  * HepGeom::RotateY3D(zAng)*HepGeom::TranslateZ3D(zPos)
    ////  * calculateLocalStrawTransform(straw);
 } else {
    std::cout << "calculateStrawTransform:  f is 0 !!!!" << std::endl;
    return {};
  }
}

HepGeom::Transform3D TRT_BarrelElement::calculateLocalStrawTransform(int straw) const
{
  const TRTCond::StrawDxContainer* container = conditions()->dxContainer();
  HepGeom::Transform3D rc ;
  if(!container) {
    // std::cout << " TRT_BarrelElement: no local transform found " << std::endl;
  } else {
    // important note: dx1 moves the 'negative' wire endpoint end dx2
    // the 'positive' wire endpoint in the local straw frame. In the
    // global frame, 'dx1' corresponds to the readout side and 'dx2'
    // to the wire joint side for both sides of the barrel.

    int bec = getCode().isPosZ() ? +1 : -1 ;
    int layer = getCode().getModuleIndex();
    int phimodule = getCode().getPhiIndex();
    int strawlayer = getCode().getStrawLayerIndex();
    TRTCond::ExpandedIdentifier id = TRTCond::ExpandedIdentifier
      (bec,layer,phimodule,strawlayer,straw,TRTCond::ExpandedIdentifier::STRAW);
    double dx1 = container->getDx1(id);
    double dx2 = container->getDx2(id);
    double ang = (dx2-dx1)/strawLength();
    double dx = (dx2+dx1)/2.;
    rc = HepGeom::TranslateX3D(dx)*HepGeom::RotateY3D(ang) ;
  }
  return rc ;
}


HepGeom::Transform3D TRT_BarrelElement::defStrawTransform(int straw) const
{
  // Same as calculateStrawTransform, except we use getDefAbsoluteTransform()
  // rather than  getAbsoluteTransform()

  // NB The tranformation to a straw is reconstructed here precisely as
  // it was ... hopefully... in the factory.  One could eliminate this
  // requirement and make the code a little more robust in this regard but
  // at the cost of doubling the descriptors.  (One descriptor now suffices
  // for both positive and negative endcaps).

  const GeoXF::Function *f= m_descriptor->getStrawTransform();
  if (f) {
    size_t offsetInto = m_descriptor->getStrawTransformOffset();
    double zPos = -m_descriptor->strawZPos();
    double zAng =  m_code.isPosZ() ? M_PI : 0;
    return Amg::EigenTransformToCLHEP(getMaterialGeom()->getDefAbsoluteTransform()*((*f)(straw+offsetInto)))
      * HepGeom::RotateY3D(zAng)*HepGeom::TranslateZ3D(zPos);
  } else {
    std::cout << "calculateStrawTransform:  f is 0 !!!!" << std::endl;
    return {};
  }

}


const Trk::SurfaceBounds& TRT_BarrelElement::strawBounds() const
{
  return m_descriptor->strawBounds();
}

const Trk::Surface& TRT_BarrelElement::elementSurface() const
{
  if (not m_surface) {
    m_surface.set(std::make_unique<Trk::PlaneSurface>(*this));
  }
  return *m_surface;
}

void TRT_BarrelElement::createSurfaceCache() const
{
 // create the surface cache
 if (!m_surfaceCache.isValid()) {
   m_surfaceCache.set(createSurfaceCacheHelper());
 }
 // creaete the surface (only if needed, links are still ok even if cache
 // update)
 if (!m_surface) {
   elementSurface();
 }
}
SurfaceCache
TRT_BarrelElement::createSurfaceCacheHelper() const{

  // Calculate the surface from the two end straws.
  int firstStraw = 0;
  int lastStraw = nStraws() - 1;

  const Amg::Vector3D& centerFirstStraw =
    center(m_idHelper->straw_id(identify(), firstStraw));
  const Amg::Vector3D& centerLastStraw =
    center(m_idHelper->straw_id(identify(), lastStraw));

  // Calculate center as the average position of the end straws.
  auto center = Amg::Vector3D(0.5 * (centerFirstStraw + centerLastStraw));

  Amg::Vector3D phiAxis = centerLastStraw - centerFirstStraw;
  double width = phiAxis.mag();
  phiAxis = phiAxis.normalized();
  double elementWidth =
    width + 2 * m_descriptor->innerTubeRadius(); // Add the straw tube radius

  // Get local z-axis. This is roughly in +ve global z direction  (exactly if no
  // misalignment) We could probably use any straw for this but we average the
  // first and last straw and renormalize to a unit vector.
  Amg::Vector3D etaAxis = 0.5 * (strawAxis(firstStraw) + strawAxis(lastStraw));
  etaAxis = etaAxis.normalized();

  // Calculate normal. This will point away from the beam line based
  // on the assumption that the straw numbering goes in the direction
  // of increasing phi and the straw axis is in +ve z direction.
  auto normal = Amg::Vector3D(phiAxis.cross(etaAxis)); // phi cross z

  // Transform from local to global.
  // local x axis -> phiAxis
  // local y axis -> etaAxis
  // local z axis -> cross product of local x and local y
  // translation -> center

  Amg::RotationMatrix3D rotation;
  rotation.col(0) = phiAxis;
  rotation.col(1) = etaAxis;
  rotation.col(2) = normal;

  // This constructor takes three points in the two coordinate systems.
  auto transform = Amg::Transform3D(Amg::Translation3D(center) * rotation);

  // create the element bounds
  auto elementBounds = std::make_unique<Trk::RectangleBounds>(
    0.5 * elementWidth, 0.5 * strawLength());

  return { transform, center, normal, std::move(elementBounds) };
}

int TRT_BarrelElement::strawDirection() const
{
  // Return +1 if the straw local axis is in the same direction as the z axis, -1 otherwise.
  // The straw axis by convention goes in the direction away from the readout.
  // So for -ve endcap this is the positive z direction (we return +1) and in the
  // +ve endcap its in the -ve z direction (we return -1).
  //
  return !(m_code.isPosZ()) ? +1 : -1;
}


} // end namespace
