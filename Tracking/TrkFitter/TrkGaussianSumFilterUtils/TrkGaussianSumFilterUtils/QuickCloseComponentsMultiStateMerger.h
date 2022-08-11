/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   QuickCloseComponentsMultiStateMerger.h
 * @date   Wednesday 3rd September 2008
 * @author Anthony Morley, Christos Anastopoulos
 *
 * Functions for merging components of a multi-state based on
 * combination of those which are "close" together as
 * defined by some metric.
 */

#ifndef TrkQuickCloseComponentsMultiStateMerger_H
#define TrkQuickCloseComponentsMultiStateMerger_H

#include "TrkGaussianSumFilterUtils/MultiComponentState.h"

namespace Trk {
namespace QuickCloseComponentsMultiStateMerger {
/// Method for merging components - ownership of objects is passed
MultiComponentState
merge(Trk::MultiComponentState&&, const unsigned int maximumNumberOfComponents);

}
}

#endif
