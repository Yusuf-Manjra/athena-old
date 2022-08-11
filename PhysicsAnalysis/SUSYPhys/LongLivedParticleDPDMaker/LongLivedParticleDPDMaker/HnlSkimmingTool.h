// -*- C++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////
// HnlSkimmingTool.h  (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_HNLSKIMMINGTOOL_H
#define DERIVATIONFRAMEWORK_HNLSKIMMINGTOOL_H 1

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"

#include "TrigDecisionTool/TrigDecisionTool.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "StoreGate/ReadHandleKey.h"

#include <string>
#include <vector>

namespace DerivationFramework {

  class HnlSkimmingTool : public AthAlgTool, public ISkimmingTool {
   
  public: 
    /** Constructor with parameters */
    HnlSkimmingTool(const std::string& t, const std::string& n, const IInterface* p);

    /** Destructor */
    virtual ~HnlSkimmingTool() = default;
   
    // Athena algtool's Hooks
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
 
    /** Check that the current event passes this filter */
    virtual bool eventPassesFilter() const override;
    bool isGood(const xAOD::Muon& mu) const;

  private:
    // Lepton types
    bool m_isPromptMuon;
    bool m_isDisplacedMuon;

    // Triggers
    ToolHandle<Trig::TrigDecisionTool> m_trigDecisionTool;
    std::vector<std::string> m_triggers;

    // Muons
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonSGKey { this, "MuonContainerKey", "Muons", ""};
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muonIsoDecorKey { this, "MuonIsoDecorKey", "Muons.ptcone30", ""};
    // Prompt muons
    float m_mu1PtMin;
    float m_mu1AbsEtaMax;
    std::vector<int> m_mu1Types;
    int m_mu1IsoType; //http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Event/xAOD/xAODPrimitives/xAODPrimitives/IsolationType.h
    bool m_mu1IsoCutIsRel; //is the isolation cut relative or absolute
    float m_mu1IsoCut; //cut value
    // Displaced muons
    float m_mu2PtMin;
    float m_mu2AbsEtaMax;
    std::vector<int> m_mu2Types;
    int m_mu2IsoType;
    bool m_mu2IsoCutIsRel;
    float m_mu2IsoCut;
    float m_mu2d0Min;

    // Electrons
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronSGKey { this, "ElectronContainerKey", "Electrons", ""};
    SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_electronIsoDecorKey { this, "ElectronIsoDecorKey", "Electrons.ptcone30", ""};
    // Prompt electrons
    float m_el1PtMin;
    float m_el1AbsEtaMax;
    std::string m_el1IDKey;
    int m_el1IsoType; //http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Event/xAOD/xAODPrimitives/xAODPrimitives/IsolationType.h
    bool m_el1IsoCutIsRel; //is the isolation cut relative or absolute
    float m_el1IsoCut; //cut value
    // Displaced electrons
    float m_el2PtMin;
    float m_el2AbsEtaMax;
    int m_el2IsoType;
    bool m_el2IsoCutIsRel;
    float m_el2IsoCut;
    float m_el2d0Min;

    float m_dPhiMin;

    void getPromptMuonCandidates(SG::ReadHandle<DataVector<xAOD::Muon_v1>>& muons,
                                 std::vector<const xAOD::Muon*>& promptMuonCandidates) const;
    void getDisplacedMuonCandidates(SG::ReadHandle<DataVector<xAOD::Muon_v1>>& muons,
                                    std::vector<const xAOD::Muon*>& displacedMuonCandidates) const;
    void getPromptElectronCandidates(SG::ReadHandle<DataVector<xAOD::Electron_v1>>& electrons,
                                     std::vector<const xAOD::Electron*>& promptElectronCandidates) const;
    void getDisplacedElectronCandidates(SG::ReadHandle<DataVector<xAOD::Electron_v1>>& electrons,
                                        std::vector<const xAOD::Electron*>& displacedElectronCandidates) const;
  };
 
}

#endif
