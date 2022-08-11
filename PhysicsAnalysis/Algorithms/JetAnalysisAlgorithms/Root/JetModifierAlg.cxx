/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <JetAnalysisAlgorithms/JetModifierAlg.h>

//
// method implementations
//

namespace CP
{
  JetModifierAlg ::
  JetModifierAlg (const std::string& name, 
                     ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
    , m_modifierTool ("JetForwardJvtTool", this)
  {
    declareProperty ("modifierTool", m_modifierTool, "the modifier tool we apply");
  }



  StatusCode JetModifierAlg ::
  initialize ()
  {
    ANA_CHECK (m_modifierTool.retrieve());
    ANA_CHECK (m_jetHandle.initialize (m_systematicsList));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode JetModifierAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      xAOD::JetContainer *jets = nullptr;
      ANA_CHECK (m_jetHandle.getCopy (jets, sys));
      ANA_CHECK (m_modifierTool->modify (*jets));
    }

    return StatusCode::SUCCESS;
  }
}
