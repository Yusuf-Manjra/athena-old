/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "MomentumConservationTool.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  MomentumConservationTool::MomentumConservationTool(const std::string& type,
                                                     const std::string& name,
                                                     const IInterface* parent)
    : UserActionToolBase<MomentumConservation>(type, name, parent)
  {
  }

  //---------------------------------------------------------------------------
  std::unique_ptr<MomentumConservation>
  MomentumConservationTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Making a MomentumConservation action");
    auto action = std::make_unique<MomentumConservation>();
    actionList.eventActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

} // namespace G4UA
