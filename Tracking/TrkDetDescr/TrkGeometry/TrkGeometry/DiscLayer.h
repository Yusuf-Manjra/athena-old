/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DiscLayer.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKGEOMETRY_DISCLAYER_H
#define TRKGEOMETRY_DISCLAYER_H

class MsgStream;

// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Trk
#include "TrkGeometry/Layer.h"
#include "TrkSurfaces/DiscSurface.h"
//
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkGeometry/ApproachDescriptor.h"
// STL sorting
#include <algorithm>
#include <memory>
namespace Trk {

class DiscBounds;
class VolumeBounds;
class LayerMaterialProperties;
class OverlapDescriptor;

/**
 @class DiscLayer

 Class to describe a disc-like detector layer for tracking,
 it inhertis from both, Layer base class
 and DiscSurface class

 @author Andreas.Salzburger@cern.ch
 */

class DiscLayer final
  : public DiscSurface
  , public Layer
{
  friend class TrackingVolume;

public:
  /**Default Constructor*/
  DiscLayer() = default;

  /**Constructor with DiscSurface components and MaterialProperties */
  DiscLayer(const Amg::Transform3D& transform,
            DiscBounds* dbounds,
            const LayerMaterialProperties& laymatprop,
            double thickness = 0.,
            OverlapDescriptor* od = nullptr,
            int laytyp = int(Trk::active));

  /**Constructor with DiscSurface and MaterialProperties */
  DiscLayer(DiscSurface* disc,
            const LayerMaterialProperties& laymatprop,
            double thickness = 0.,
            OverlapDescriptor* od = nullptr,
            int laytyp = int(Trk::active));

  /**Constructor with DiscSurface components and pointer to SurfaceArray
   * (passing ownership) */
  DiscLayer(const Amg::Transform3D& transform,
            DiscBounds* dbounds,
            SurfaceArray* surfaceArray,
            double isontolerance = 0.,
            OverlapDescriptor* od = nullptr,
            IApproachDescriptor* ad = nullptr,
            int laytyp = int(Trk::active));

  /**Constructor with DiscSurface components,
     MaterialProperties and pointer SurfaceArray (passing ownership) */
  DiscLayer(const Amg::Transform3D& transform,
            DiscBounds* dbounds,
            SurfaceArray* surfaceArray,
            const LayerMaterialProperties& laymatprop,
            double thickness = 0.,
            OverlapDescriptor* od = nullptr,
            IApproachDescriptor* ad = nullptr,
            int laytyp = int(Trk::active));

  /**Copy constructor of DiscLayer*/
  DiscLayer(const DiscLayer& cla);

  /**Copy constructor with shift*/
  DiscLayer(const DiscLayer& cla, const Amg::Transform3D& tr);

  /**Assignment operator for DiscLayers */
  DiscLayer& operator=(const DiscLayer&);

  /**Destructor*/
  virtual ~DiscLayer() = default;

  /** Transforms the layer into a Surface representation for extrapolation */
  virtual const DiscSurface& surfaceRepresentation() const override final;

  /** Surface seen on approach - if not defined differently, it is the
   * surfaceRepresentation() */
  virtual const Surface& surfaceOnApproach(
      const Amg::Vector3D& pos, const Amg::Vector3D& mom, PropDirection pdir,
      const BoundaryCheck& bcheck, bool resolveSubSurfaces = 0,
      const ICompatibilityEstimator* ice = nullptr) const override final;

  /** getting the MaterialProperties back - for pre-update*/
  virtual double preUpdateMaterialFactor(
      const Trk::TrackParameters& par,
      Trk::PropDirection dir) const override final;

  /** getting the MaterialProperties back - for post-update*/
  virtual double postUpdateMaterialFactor(
      const Trk::TrackParameters& par,
      Trk::PropDirection dir) const override final;

  /** move the Layer non-const*/
  virtual void moveLayer(Amg::Transform3D& shift) override final;

 private:
  /** Resize the layer to the tracking volume - only works for
   * CylinderVolumeBouns */
  virtual void resizeLayer(const VolumeBounds& vBounds,
                           double envelope) override final;

  /** Resize the layer to the tracking volume - not implemented.*/
  virtual void resizeAndRepositionLayer(const VolumeBounds& vBounds,
                                        const Amg::Vector3D& cCenter,
                                        double envelop) override final;

  /** build approach surfaces */
  void buildApproachDescriptor();

  /** Surface seen on approach - if not defined differently, it is the
   * surfaceRepresentation() */
  const Surface& approachSurface(const Amg::Vector3D& pos,
                                 const Amg::Vector3D& dir,
                                 const BoundaryCheck& bcheck) const;

 protected:
  //!< surface for approaching
  std::unique_ptr<IApproachDescriptor> m_approachDescriptor;
};

/** @class DiscLayerSorterZ
      simple helper function to allow sorting of DiscLayers in z
*/
class DiscLayerSorterZ {
 public:
  bool operator()(const DiscLayer* one, const DiscLayer* two) const {
    return (one->center().z() < two->center().z());
  }
};

}  // namespace Trk

#endif  // TRKGEOMETY_DISCLAYER_H

