/*
   Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

#include "TopObjectSelectionTools/Tau.h"
#include "TopEvent/EventTools.h"

namespace top {
  Tau::Tau() : m_tauSelection("TauAnalysisTools::TauSelectionTool"),
    m_tauSelectionLoose
      ("TauAnalysisTools::TauSelectionToolLoose") {
    top::check(m_tauSelection.retrieve(),
               "Failed to retrieve tau selection tool");
    top::check(m_tauSelectionLoose.retrieve(),
               "Failed to retrieve loose tau selection tool");
  }

  bool Tau::passSelection(const xAOD::TauJet& tau) const {
    // Get the WP related decisions from TauSelectionTool

    // HIGG8D1 derivations (and possibly others) skim away
    // tau tracks with pT<15 GeV. The tau selector tool
    // returns an exception in this case. These taus should
    // NOT be selected so return false if an exception is raised.
    try {
      return static_cast<bool>(m_tauSelection->accept(tau));
    } catch (std::runtime_error& e) {
      // If an exception is raised (possibly due to no tau track)
      // then return false
      return false;
    }
  }

  bool Tau::passSelectionLoose(const xAOD::TauJet& tau) const {
    // get the WP related decisions from TauSelectionTool
    // For the reasoning behind try/catch see above...
    try {
      return static_cast<bool>(m_tauSelectionLoose->accept(tau));
    } catch (std::runtime_error& e) {
      return false;
    }
  }

  void Tau::print(std::ostream& os) const {
    os << "Tau\n";
    os << "    * Taus are now selected using the TauSelectionTool" << "\n";
    os << "    * Either you have supplied a tau configuration file (expert)\n";
    os << "      or set pT, JetIDWP and EleOLR options in your top-xaod\n";
    os << "      configuration file.";
  }
}  // namespace top
