/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BinnedArray.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUTILS_BINNEDARRAY_H
#define TRKDETDESCRUTILS_BINNEDARRAY_H

#include "TrkDetDescrUtils/BinUtility.h"
#include "TrkDetDescrUtils/SharedObject.h"
// GaudiKernel
#include "GaudiKernel/GaudiException.h"
// Eigen
#include "CxxUtils/span.h"
#include "GeoPrimitives/GeoPrimitives.h"

// STL
#include <vector>

class MsgStream;

namespace Trk {
/** @class BinnedArray

   Binned Array for avoiding map searches/

   @author Andreas.Salzburger@cern.ch
   @author Christos Anastopoulos (AthenaMT)
   */
template<class T>
using BinnedArraySpan = CxxUtils::span<T>;

template<class T>
class BinnedArray
{

public:
  /**Default Constructor - needed for inherited classes */
  BinnedArray() {}

  /**Virtual Destructor*/
  virtual ~BinnedArray() {}

  /** Implicit constructor */
  virtual BinnedArray* clone() const = 0;

  /** Returns the pointer to the templated class object from the BinnedArray,
      it returns 0 if not defined, takes local position */
  virtual T* object(const Amg::Vector2D& lp) const = 0;

  /** Returns the pointer to the templated class object from the BinnedArray
      it returns 0 if not defined, takes global position */
  virtual T* object(const Amg::Vector3D& gp) const = 0;

  /** Returns the pointer to the templated class object from the BinnedArray -
   * entry point*/
  virtual T* entryObject(const Amg::Vector3D&) const = 0;

  /** Returns the pointer to the templated class object from the BinnedArray,
   * takes 3D position & direction */
  virtual T* nextObject(const Amg::Vector3D& gp,
                        const Amg::Vector3D& mom,
                        bool associatedResult = true) const = 0;

  /** Return all objects of the Array non-const
   * we can still modify the T*/
  virtual BinnedArraySpan<T* const> arrayObjects() = 0;

  /** Return all objects of the Array const */
  virtual BinnedArraySpan<T const * const> arrayObjects() const = 0;

  /** Number of Entries in the Array */
  virtual unsigned int arrayObjectsNumber() const = 0;

  /** Return the BinUtility*/
  virtual const BinUtility* binUtility() const = 0;

private:
};
} // end of namespace Trk

#endif // TRKDETDESCRUTILS_BINNEDARRAY_H
