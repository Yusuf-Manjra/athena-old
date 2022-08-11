/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IEMConversionBuilder.h, (c) ATLAS Detector software 2008
///////////////////////////////////////////////////////////////////

#ifndef EGAMMAINTERFACES_IEMCONVERSIONBUILDER_H
#define EGAMMAINTERFACES_IEMCONVERSIONBUILDER_H

/** @class IEMConversionBuilder
  Interface for the Reconstruction/egamma/egammaRec/EMConversionBuilder

  @author Frederic Derue derue@lpnhe.in2p3.fr

CREATED : 01/09/2008
MODIFIED :
*/

// INCLUDE HEADER FILES:
#include "egammaInterfaces/IegammaBaseTool.h"
#include "xAODTracking/VertexContainerFwd.h"
#include "GaudiKernel/EventContext.h"
// Forward declarations
class egammaRec;

static const InterfaceID IID_IEMConversionBuilder("IEMConversionBuilder", 1, 0);

class IEMConversionBuilder : virtual public IAlgTool
{

 public:

  /** @brief Virtual destructor*/
  virtual ~IEMConversionBuilder() {};

  /** @brief AlgTool interface methods */
  static const InterfaceID& interfaceID();

  /** @brief initialize method*/
  virtual StatusCode initialize() = 0;
  /** @brief execute method*/
  virtual StatusCode executeRec(const EventContext& ctx,egammaRec* egRec) const =  0;

};

inline const InterfaceID& IEMConversionBuilder::interfaceID()
{
  return IID_IEMConversionBuilder;
}

#endif









