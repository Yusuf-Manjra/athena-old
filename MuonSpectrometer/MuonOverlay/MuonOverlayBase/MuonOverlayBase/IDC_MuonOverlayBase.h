/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

/**
 * @file
 *
 * Overlaying of Identifiable Containers.  Functions common to the
 * single-hit-per-readout-channel-in-an-event case and the
 * possible-multiple-hits-on-a-channel case are declared in this file.
 *
 * @author Tadej Novak <tadej@cern.ch>
 * @author Andrei Gaponenko <agaponenko@lbl.gov>, 2009
 *
 */

#ifndef IDC_MUONOVERLAYCOMMON_H
#define IDC_MUONOVERLAYCOMMON_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <Identifier/IdentifierHash.h>


class IDC_MuonOverlayBase : public AthReentrantAlgorithm
{
public:
  IDC_MuonOverlayBase(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

protected:
  template <class IDC_Container>
  StatusCode overlayContainer(const IDC_Container *bkgContainer,
                              const IDC_Container *signalContainer,
                              IDC_Container *outputContainer) const
  {
    return overlayContainerBase(bkgContainer, signalContainer, outputContainer, false);
  }

  template <class IDC_Container>
  StatusCode overlayMultiHitContainer(const IDC_Container *bkgContainer,
                                      const IDC_Container *signalContainer,
                                      IDC_Container *outputContainer) const
  {
    return overlayContainerBase(bkgContainer, signalContainer, outputContainer, true);
  }

private:
  template <class IDC_Container>
  StatusCode overlayContainerBase(const IDC_Container *bkgContainer,
                                  const IDC_Container *signalContainer,
                                  IDC_Container *outputContainer,
                                  bool isMultiHitCollection) const;

  template <class Collection>
  std::unique_ptr<Collection> copyCollection(const IdentifierHash &hashId,
                                             const Collection *collection) const;

};

#include "IDC_MuonOverlayBase.icc"

#endif
