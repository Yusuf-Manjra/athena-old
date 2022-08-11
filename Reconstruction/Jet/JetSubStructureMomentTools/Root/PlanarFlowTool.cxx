/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureMomentTools/PlanarFlowTool.h"
#include "JetSubStructureUtils/PlanarFlow.h" 

PlanarFlowTool::PlanarFlowTool(std::string name) : 
  JetSubStructureMomentToolsBase(name)
{
}

int PlanarFlowTool::modifyJet(xAOD::Jet &injet) const {
  
  fastjet::PseudoJet jet;
  bool decorate = SetupDecoration(jet,injet);
  double pf_value = -999;
  
  if (decorate) {
    JetSubStructureUtils::PlanarFlow pf;
    pf_value = pf.result(jet);
    ATH_MSG_VERBOSE("Adding jet PlanarFlow: " << pf_value);
  }

  injet.setAttribute(m_prefix+"PlanarFlow", pf_value);
  
  return 0;
}
