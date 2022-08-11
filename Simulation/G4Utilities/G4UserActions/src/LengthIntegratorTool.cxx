/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "LengthIntegratorTool.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  LengthIntegratorTool::LengthIntegratorTool(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent)
    : UserActionToolBase<LengthIntegrator>(type, name, parent),
      m_hSvc("THistSvc", name)
  {
    declareProperty("HistoSvc", m_hSvc);
    declareProperty("CreateHistograms", m_doHistos=true);
    
  }

  //---------------------------------------------------------------------------
  // Initialize - temporarily here for debugging
  //---------------------------------------------------------------------------
  StatusCode LengthIntegratorTool::initialize()
  {
    ATH_MSG_DEBUG("initialize");

    ATH_CHECK( m_hSvc.retrieve() );

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Create the action on request
  //---------------------------------------------------------------------------
  std::unique_ptr<LengthIntegrator>
  LengthIntegratorTool::makeAndFillAction(G4AtlasUserActions& actionList)
  {
    ATH_MSG_DEBUG("Making a LengthIntegrator action");
    auto action = std::make_unique<LengthIntegrator>( m_hSvc.name(), m_doHistos );
    actionList.eventActions.push_back( action.get() );
    actionList.steppingActions.push_back( action.get() );
    return action;
  }

}
