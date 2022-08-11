/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUDQA_EFFICIENCYPTPLOTS_H
#define TAUDQA_EFFICIENCYPTPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "ParamPlots.h"
#include "xAODTau/TauJet.h"

namespace Tau{

class EfficiencyPtPlots: public PlotBase {
  public:
    EfficiencyPtPlots(PlotBase *pParent, const std::string& sDir, std::string sTauJetContainerName);
    virtual ~EfficiencyPtPlots();

    void fill(const xAOD::TauJet& tau, float weight);

    TProfile* m_eff_pt_jetRNNloose;
    TProfile* m_eff_pt_jetRNNmed;
    TProfile* m_eff_pt_jetRNNtight;
    TProfile* m_eff_pt_jetRNNlooseHighPt = nullptr;
    TProfile* m_eff_pt_jetRNNmedHighPt = nullptr;
    TProfile* m_eff_pt_jetRNNtightHighPt = nullptr;

    TProfile* m_eff_jetRNNloose;
    TProfile* m_eff_jetRNNmed;
    TProfile* m_eff_jetRNNtight;

  private:
    void initializePlots();
    std::string m_sTauJetContainerName;
};

}

#endif
