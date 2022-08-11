/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   MultiComponentStateAssembler.h
 * @date   Monday 20th December 2004
 * @author Atkinson,Anthony Morley, Christos Anastopoulos
 *
 *  Helper struct and methods to collect components of a
 *  multi-component state and put them all into a
 *  MultiComponentState. The addition of components can be
 *  one at a time through the addComponent method (taking a
 *  single ComponentParameters object) or many at a time,(taking a
 *  MultiComponentState object). It also included
 *  helpers to be used in weighting renormalisation
 *  calculations and the removal of components with
 *  insignificantly small weightings.
 */

#ifndef MultiComponentStateAssembler_H
#define MultiComponentStateAssembler_H

#include "TrkGaussianSumFilterUtils/MultiComponentState.h"
#include <vector>

namespace Trk {

/**
 * Helper struct representing a cache of the Multicomponent
 * state under assembly
 */
namespace MultiComponentStateAssembler {
struct Cache
{
  Cache()
    : multiComponentState{}
    , validWeightSum{ 0 }
    , invalidWeightSum{ 0 }
    , minimumValidFraction{ 0.01 }
    , minimumFractionalWeight{ 1e-09 }
    , assemblyDone{ false }
  {
    multiComponentState.reserve(72);
  }
  Trk::MultiComponentState multiComponentState;
  double validWeightSum;
  double invalidWeightSum;
  const double minimumValidFraction;
  const double minimumFractionalWeight;
  bool assemblyDone;
};

/** Method to add a single set of Trk::ComponentParameters to the cached
 * Trk::MultiComponentState object under construction */
bool
addComponent(
  MultiComponentStateAssembler::Cache& cache,
  ComponentParameters&& multiComponentState);

/** Method to add a new Trk::MultiComponentState to the cached
 * Trk::MultiComponentState o bject under construction */
bool
addMultiState(
  MultiComponentStateAssembler::Cache& cache,
  Trk::MultiComponentState&& multiComponentState);

/** Method to return the cached state object -
 * it performs a reweighting before returning the object based on the
 * valid and invaid weights */
MultiComponentState
assembledState(MultiComponentStateAssembler::Cache&& cache);

} // End MultiComponentStateAssembler namespace

} // End Trk namepace

#endif
