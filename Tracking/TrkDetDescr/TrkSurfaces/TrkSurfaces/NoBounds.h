/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CylinderBounds.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKSURFACES_NOBOUNDS_H
#define TRKSURFACES_NOBOUNDS_H

#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkSurfaces/SurfaceBounds.h"

// Eigen
#include "GeoPrimitives/GeoPrimitives.h"

class MsgStream;

namespace Trk {

/**
 @class NoBounds
 Bounds object for a boundless surface (...)

 @author Andreas.Salzburger@cern.ch
 */

class NoBounds final : public SurfaceBounds
{
public:
  /**Default Constructor*/
  NoBounds() = default;
  NoBounds(const NoBounds&) = default;
  NoBounds& operator=(const NoBounds&) = default;
  NoBounds(NoBounds&&) noexcept = default;
  NoBounds& operator=(NoBounds&&) noexcept  = default;
  /** Destructor */
  ~NoBounds() = default;

  /**Equality operator */
  virtual bool operator==(const SurfaceBounds& sbo) const override final;

  /** Return SurfaceBounds for persistency */
  virtual SurfaceBounds::BoundsType type() const override { return SurfaceBounds::Other; }

  /** Method inside() returns true for any case */
  virtual bool inside(const Amg::Vector2D& locpo, double tol1 = 0., double tol2 = 0.) const override final;
  virtual bool inside(const Amg::Vector2D& locpo, const BoundaryCheck& bchk) const override final;
  /** This method checks inside bounds in loc1
    - loc1/loc2 correspond to the natural coordinates of the surface */
  virtual bool insideLoc1(const Amg::Vector2D& locpo, double tol1 = 0.) const override final;

  /** This method checks inside bounds in loc2
    - loc1/loc2 correspond to the natural coordinates of the surface */
  virtual bool insideLoc2(const Amg::Vector2D& locpo, double tol2 = 0.) const override final;

  /** Minimal distance to boundary (=0 if inside) */
  virtual double minDistance(const Amg::Vector2D& pos) const override final;

  /** Clone method to complete inherited interface */
  virtual NoBounds* clone() const override final;

  /** r() method to complete inherited interface */
  virtual double r() const override final;

  /** Output Method for MsgStream*/
  virtual MsgStream& dump(MsgStream& sl) const override final;

  /** Output Method for std::ostream */
  virtual std::ostream& dump(std::ostream& sl) const override final;

};
} // end of namespace

#include "TrkSurfaces/NoBounds.icc"
#endif // TRKSURFACES_NOBOUNDS_H
