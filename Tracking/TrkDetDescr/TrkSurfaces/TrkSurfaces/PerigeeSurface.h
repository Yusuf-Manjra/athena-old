/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// PerigeeSurface.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKSURFACES_PERIGEESURFACE_H
#define TRKSURFACES_PERIGEESURFACE_H

// Trk
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkParametersBase/ParametersT.h"
#include "TrkSurfaces/NoBounds.h"
#include "TrkSurfaces/Surface.h"
// Amg
#include "CxxUtils/CachedValue.h"
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"

class MsgStream;

namespace Trk {

class LocalParameters;
template<int DIM, class T, class S>
class ParametersT;

/**
 @class PerigeeSurface

 Class describing the Line to which the Perigee refers to.
 The Surface axis is fixed to be the z-axis of the Tracking frame.
 It inherits from Surface.

 @author Andreas.Salzburger@cern.ch
 @author Christos Anastopoulos (Thread safety and interface cleanup)
 @author Shaun Roe (interface cleanup)
 */

class PerigeeSurface final : public Surface
{

public:
  /** The surface type static constexpr */
  static constexpr SurfaceType staticType = SurfaceType::Perigee;

  /**Default Constructor - needed for persistency*/
  PerigeeSurface();

  /**Copy constructor*/
  PerigeeSurface(const PerigeeSurface& pesf);

  /**Assignment operator*/
  PerigeeSurface& operator=(const PerigeeSurface& slsf);

  /**Copy constructor*/
  PerigeeSurface(PerigeeSurface&& pesf) noexcept = default;

  /**Assignment operator*/
  PerigeeSurface& operator=(PerigeeSurface&& slsf) noexcept = default;

  /**Destructor*/
  virtual ~PerigeeSurface() = default;

  /**Constructor from GlobalPosition*/
  PerigeeSurface(const Amg::Vector3D& gp);

  /**Constructor with a Transform by ref - needed for tilt */
  PerigeeSurface(const Amg::Transform3D& tTransform);

  /**Copy constructor with shift*/
  PerigeeSurface(const PerigeeSurface& pesf, const Amg::Transform3D& transf);

  /**Virtual constructor*/
  virtual PerigeeSurface* clone() const override final;
  
  /** NVI unique_ptr method **/
  std::unique_ptr<PerigeeSurface> uniqueClone() const;

  /**Equality operator*/
  virtual bool operator==(const Surface& sf) const override;

  // Needed to prevent ambiguities with c++20.
  bool operator==(const PerigeeSurface& cf) const;

  /** Use the Surface as a ParametersBase constructor, from local parameters -
   * charged */
  virtual Surface::ChargedTrackParametersUniquePtr createUniqueTrackParameters(
    double l1,
    double l2,
    double phi,
    double theta,
    double qop,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const override final;

  /** Use the Surface as a ParametersBase constructor, from global parameters -
   * charged*/
  virtual Surface::ChargedTrackParametersUniquePtr createUniqueTrackParameters(
    const Amg::Vector3D& position,
    const Amg::Vector3D& momentum,
    double charge,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const override final;

  /** Use the Surface as a ParametersBase constructor, from local parameters -
   * neutral */
  virtual NeutralTrackParametersUniquePtr createUniqueNeutralParameters(
    double l1,
    double l2,
    double phi,
    double theta,
    double qop,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const override final;

  /** Use the Surface as a ParametersBase constructor, from global parameters -
   * neutral */
  virtual NeutralTrackParametersUniquePtr createUniqueNeutralParameters(
    const Amg::Vector3D& position,
    const Amg::Vector3D& momentum,
    double charge,
    std::optional<AmgSymMatrix(5)> cov = std::nullopt) const override final;

  /** Use the Surface as a ParametersBase constructor, from local parameters */
  template<int DIM, class T>
  std::unique_ptr<ParametersT<DIM, T, PerigeeSurface>> createUniqueParameters(
    double l1,
    double l2,
    double phi,
    double theta,
    double qop,
    std::optional<AmgSymMatrix(DIM)> cov = std::nullopt) const;

  /** Use the Surface as a ParametersBase constructor, from global parameters */
  template<int DIM, class T>
  std::unique_ptr<ParametersT<DIM, T, PerigeeSurface>> createUniqueParameters(
    const Amg::Vector3D& position,
    const Amg::Vector3D& momentum,
    double charge,
    std::optional<AmgSymMatrix(DIM)> cov = std::nullopt) const;

  /** Use the Surface as a ParametersBase constructor, from local parameters */
  template<int DIM, class T>
  ParametersT<DIM, T, PerigeeSurface> createParameters(
    double l1,
    double l2,
    double phi,
    double theta,
    double qop,
    std::optional<AmgSymMatrix(DIM)> cov = std::nullopt) const;

  /** Use the Surface as a ParametersBase constructor, from global parameters */
  template<int DIM, class T>
  ParametersT<DIM, T, PerigeeSurface> createParameters(
    const Amg::Vector3D& position,
    const Amg::Vector3D& momentum,
    double charge,
    std::optional<AmgSymMatrix(DIM)> cov = std::nullopt) const;

  /** Return the surface type */
  virtual SurfaceType type() const override final;

  /** Returns the x global axis */
  virtual const Amg::Vector3D& normal() const override final;

  /**Returns a normal vector at a specific localPosition*/
  virtual Amg::Vector3D normal(const Amg::Vector2D& lp) const override final;

  /** Return the measurement frame - this is needed for alignment, in particular
     for StraightLine and Perigee Surface
      - the default implementation is the the RotationMatrix3D of the transform
   */
  virtual Amg::RotationMatrix3D measurementFrame(
    const Amg::Vector3D& glopos,
    const Amg::Vector3D& glomom) const override final;

  /** Local to global method:
      Take care that by just providing locR and locZ the global position cannot
     be calculated. Therefor only the locZ-coordinate is taken and transformed
     into the global frame. for calculating the global position, a momentum
     direction has to be provided as well, use the appropriate function!
       */
  Amg::Vector3D localToGlobal(const LocalParameters& locpos) const;

  /** This method is the true local->global transformation.<br>
      by providing a locR and locZ coordinate such as a GlobalMomentum
      the global position can be calculated.
      The choice between the two possible canditates is done by the sign of the
     radius
      */
  Amg::Vector3D localToGlobal(const LocalParameters& locpos,
                              const Amg::Vector3D& glomom) const;

  /** LocalToGlobal method without dynamic memory allocation */
  virtual void localToGlobal(const Amg::Vector2D& locp,
                             const Amg::Vector3D& mom,
                             Amg::Vector3D& glob) const override final;

  /** GlobalToLocal method without dynamic memory allocation - boolean checks if
     on surface \image html SignOfDriftCircleD0.gif
  */
  virtual bool globalToLocal(const Amg::Vector3D& glob,
                             const Amg::Vector3D& mom,
                             Amg::Vector2D& loc) const override final;

  /** fast straight line intersection schema - standard: provides closest
     intersection and (signed) path length forceDir is to provide the closest
     forward solution

      b>mathematical motivation:</b>
      Given two lines in parameteric form:<br>
      - @f$ \vec l_{a}(\lambda) = \vec m_a + \lambda \cdot \vec e_{a} @f$ <br>
      - @f$ \vec l_{b}(\mu) = \vec m_b + \mu \cdot \vec e_{b} @f$ <br>
      the vector between any two points on the two lines is given by:
      - @f$ \vec s(\lambda, \mu) = \vec l_{b} - l_{a} = \vec m_{ab} + \mu \cdot
     \vec e_{b} - \lambda \cdot \vec e_{a}
     @f$, <br> when @f$ \vec m_{ab} = \vec m_{b} - \vec m_{a} @f$.<br>
      @f$ \vec s(\lambda_0, \mu_0) @f$  denotes the vector between the two
     closest points <br>
      @f$ \vec l_{a,0} = l_{a}(\lambda_0) @f$ and @f$ \vec l_{b,0} =
     l_{b}(\mu_0) @f$ <br> and is perpenticular to both, @f$ \vec e_{a} @f$ and
     @f$ \vec e_{b} @f$.

      This results in a system of two linear equations:<br>
      - (i) @f$ 0 = \vec s(\lambda_0, \mu_0) \cdot \vec e_a = \vec m_ab \cdot
     \vec e_a + \mu_0 \vec e_a \cdot \vec e_b - \lambda_0 @f$ <br>
      - (ii) @f$ 0 = \vec s(\lambda_0, \mu_0) \cdot \vec e_b = \vec m_ab \cdot
     \vec e_b + \mu_0  - \lambda_0 \vec e_b \cdot \vec e_a @f$ <br>

      Solving (i), (ii) for @f$ \lambda_0 @f$ and @f$ \mu_0 @f$ yields:
      - @f$ \lambda_0 = \frac{(\vec m_ab \cdot \vec e_a)-(\vec m_ab \cdot \vec
     e_b)(\vec e_a \cdot \vec e_b)}{1-(\vec e_a \cdot \vec e_b)^2} @f$ <br>
      - @f$ \mu_0 = - \frac{(\vec m_ab \cdot \vec e_b)-(\vec m_ab \cdot \vec
     e_a)(\vec e_a \cdot \vec e_b)}{1-(\vec e_a \cdot \vec e_b)^2} @f$ <br>
   */
  virtual Intersection straightLineIntersection(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& dir,
    bool forceDir = false,
    Trk::BoundaryCheck bchk = false) const override final;

  /** fast straight line distance evaluation to Surface */
  virtual DistanceSolution straightLineDistanceEstimate(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& dir) const override final;

  /** fast straight line distance evaluation to Surface - with bound option*/
  virtual DistanceSolution straightLineDistanceEstimate(
    const Amg::Vector3D& pos,
    const Amg::Vector3D& dir,
    bool Bound) const override final;

  /** the pathCorrection for derived classes with thickness */
  virtual double pathCorrection(const Amg::Vector3D&,
                                const Amg::Vector3D&) const override final;

  /**This method checks if a globalPosition in on the Surface or not*/
  virtual bool isOnSurface(const Amg::Vector3D& glopo,
                           const BoundaryCheck& bchk = true,
                           double tol1 = 0.,
                           double tol2 = 0.) const override final;

  /**This surface calls the iside method of the bounds*/
  virtual bool insideBounds(const Amg::Vector2D& locpos,
                            double tol1 = 0.,
                            double tol2 = 0.) const override final;

  virtual bool insideBoundsCheck(
    const Amg::Vector2D& locpos,
    const BoundaryCheck& bchk) const override final;

  /** Special method for StraightLineSurface - provides the Line direction from
   * cache: speedup */
  const Amg::Vector3D& lineDirection() const;

  /** Return bounds() method */
  virtual const NoBounds& bounds() const override final;

  /** Return properly formatted class name for screen output */
  virtual std::string name() const override final;

  /** Output Method for MsgStream*/
  virtual MsgStream& dump(MsgStream& sl) const override;
  /** Output Method for std::ostream*/
  virtual std::ostream& dump(std::ostream& sl) const override;

protected:
  //!< data members
  //!< cache of the line direction (speeds up)
  CxxUtils::CachedValue<Amg::Vector3D> m_lineDirection;
  static const NoBounds s_perigeeBounds;
};

} // end of namespace
#include "TrkSurfaces/PerigeeSurface.icc"

#endif // TRKSURFACES_PERIGEESURFACE_H
