/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHISTUTILS_TRUTHMUONPLOTORGANIZER_H
#define MUONHISTUTILS_TRUTHMUONPLOTORGANIZER_H

#include "TrkValHistUtils/PlotBase.h"
#include "TrkValHistUtils/ParamPlots.h"
#include "TrkValHistUtils/MSHitPlots.h"
#include "TrkValHistUtils/TruthInfoPlots.h"
#include "TrkValHistUtils/TruthTrkExtrapolationPlots.h"

#include "xAODTruth/TruthParticle.h"
#include <vector>

namespace Muon
{
  enum TRUTHPLOTCLASS {TRUTH_PARAM,TRUTH_INFO,TRUTH_TRKEXTRAP,TRUTH_MSHITS,MAX_TRUTHPLOTCLASS};
  
  class TruthMuonPlotOrganizer:public PlotBase {
  public:
    TruthMuonPlotOrganizer(PlotBase* pParent, const std::string& sDir,std::vector<int> selPlots = {});
    ~TruthMuonPlotOrganizer();
    
  
    std::vector<int> m_selPlots;  
    
    void fill(const xAOD::TruthParticle& truthMu, float weight=1.0);
    
    // Truth plots
    std::unique_ptr<Trk::ParamPlots>                  m_oTruthPlots{};
    std::unique_ptr<Trk::TruthInfoPlots>              m_oTruthInfoPlots{};
    std::unique_ptr<Trk::TruthTrkExtrapolationPlots>  m_oTruthTrkExtrapolationPlots{};
    std::unique_ptr<Trk::MSHitPlots>                  m_oTruthMSHitPlots{};
  };
 
}

#endif
