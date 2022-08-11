/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SubtractedPlaneSurface.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRYSURFACES_SUBTRACTEDPLANESURFACE_H
#define TRKGEOMETRYSURFACES_SUBTRACTEDPLANESURFACE_H

// Trk
#include "TrkDetDescrUtils/AreaExcluder.h"
#include "TrkSurfaces/PlaneSurface.h"
// Geometry & Maths
#include "GeoPrimitives/GeoPrimitives.h"

class MsgStream;
class Identifier;

namespace Trk {

/**
 @class SubtractedPlaneSurface
 Class for a planar subtracted/shared surface in the ATLAS detector.
 It owns its surface bounds and subtracted volume.

 @author Sarka.Todorova@cern.ch
 */

class SubtractedPlaneSurface : public PlaneSurface
{
public:
  /** Default Constructor - needed for persistency*/
  SubtractedPlaneSurface();

  /** Copy Constructor*/
  SubtractedPlaneSurface(const SubtractedPlaneSurface& psf);

  /** Copy Constructor with shift*/
  SubtractedPlaneSurface(const SubtractedPlaneSurface& psf,
                         const Amg::Transform3D& transf);

  /** Constructor */
  SubtractedPlaneSurface(const PlaneSurface& ps,
                         AreaExcluder* vol,
                         bool shared);

  /**Destructor*/
  virtual ~SubtractedPlaneSurface();

  /**Assignment operator*/
  SubtractedPlaneSurface& operator=(const SubtractedPlaneSurface& psf);

  /**Equality operator*/
  virtual bool operator==(const Surface& sf) const override final;

  /** This method indicates the subtraction mode */
  bool shared() const;

  /**This method calls the inside() method of the Bounds*/
  virtual bool insideBounds(const Amg::Vector2D& locpos,
                            double tol1 = 0.,
                            double tol2 = 0.) const override final;

  /**This method allows access to the subtracted part*/
  SharedObject<AreaExcluder> subtractedVolume() const;

  /** Return properly formatted class name for screen output */
  virtual std::string name() const override final
  {
    return "Trk::SubtractedPlaneSurface";
  }

protected:
  SharedObject<AreaExcluder> m_subtrVol;
  bool m_shared;
};

} // end of namespace
#include "TrkGeometrySurfaces/SubtractedPlaneSurface.icc"
#endif // TRKGEOMETRYSURFACES_SUBTRACTEDPLANESURFACE_H
