/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#ifndef SCALEFACTORCALCULATOR_H_
#define SCALEFACTORCALCULATOR_H_

/**
 * @author John Morris <john.morris@cern.ch>
 *
 * @brief ScaleFactorCalculator
 *   Scale factors for all objects
 *
 * $Revision: 766520 $
 * $Date: 2016-08-04 17:39:21 +0100 (Thu, 04 Aug 2016) $
 *
 **/

// system include(s):
#include <vector>
#include <memory>

// Framework include(s):
#include "AsgTools/AsgTool.h"

// PMG Tools
#include "PMGTools/PMGSherpa22VJetsWeightTool.h"
#include "PMGAnalysisInterfaces/IPMGTruthWeightTool.h"

// Local include(s):
#include "TopCorrections/PhotonScaleFactorCalculator.h"
#include "TopCorrections/ElectronScaleFactorCalculator.h"
#include "TopCorrections/FwdElectronScaleFactorCalculator.h"
#include "TopCorrections/MuonScaleFactorCalculator.h"
#include "TopCorrections/TauScaleFactorCalculator.h"
#include "TopCorrections/JetScaleFactorCalculator.h"
#include "TopCorrections/BTagScaleFactorCalculator.h"
#include "TopCorrections/PileupScaleFactorCalculator.h"
#include "TopCorrections/GlobalLeptonTriggerCalculator.h"

namespace top {
  class TopConfig;

  class ScaleFactorCalculator final: public asg::AsgTool {
  public:
    explicit ScaleFactorCalculator(const std::string& name);
    virtual ~ScaleFactorCalculator() {}

    ScaleFactorCalculator(const ScaleFactorCalculator& rhs) = delete;
    ScaleFactorCalculator(ScaleFactorCalculator&& rhs) = delete;
    ScaleFactorCalculator& operator = (const ScaleFactorCalculator& rhs) = delete;

    StatusCode initialize();
    StatusCode initialize_nominal_MC_weight(); // determine what MC weight to use as nominal
    StatusCode execute();

    StatusCode executePileup();
    float pileupWeight() const;
    float mcEventWeight() const;
  private:
    std::shared_ptr<top::TopConfig> m_config;

    std::unique_ptr<top::PhotonScaleFactorCalculator>   m_photonSF;
    std::unique_ptr<top::ElectronScaleFactorCalculator> m_electronSF;
    std::unique_ptr<top::FwdElectronScaleFactorCalculator> m_fwdElectronSF;
    std::unique_ptr<top::MuonScaleFactorCalculator>     m_muonSF;
    std::unique_ptr<top::TauScaleFactorCalculator>      m_tauSF;
    std::unique_ptr<top::JetScaleFactorCalculator>      m_jetSF;
    std::unique_ptr<top::BTagScaleFactorCalculator>     m_btagSF;
    std::unique_ptr<top::PileupScaleFactorCalculator>   m_pileupSF;
    ToolHandle<PMGTools::PMGSherpa22VJetsWeightTool>    m_sherpa_22_reweight_tool;
    std::unique_ptr<top::GlobalLeptonTriggerCalculator> m_globalLeptonTriggerSF;
    ToolHandle<PMGTools::IPMGTruthWeightTool>           m_pmg_truth_weight_tool;

    std::string m_nominal_weight_name;
  };
}  // namespace top

#endif
