/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

// $Id: TauObjectCollectionMaker.cxx  $
#include "TopSystematicObjectMaker/TauObjectCollectionMaker.h"
#include "TopConfiguration/TopConfig.h"
#include "TopConfiguration/TreeFilter.h"
#include "TopEvent/EventTools.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODBase/IParticleHelpers.h"
#include "PATInterfaces/SystematicsUtil.h"

namespace top {
  TauObjectCollectionMaker::TauObjectCollectionMaker(const std::string& name) :
    asg::AsgTool(name),
    m_config(nullptr),

    m_specifiedSystematics(),
    m_recommendedSystematics(),

    m_calibrationTool("TauSmearingTool"),
    m_truthMatchingTool("TauAnalysisTools::TauTruthMatchingTool") {
    declareProperty("config", m_config);

    declareProperty("TauSmearingTool", m_calibrationTool);
  }

  StatusCode TauObjectCollectionMaker::initialize() {
    ATH_MSG_INFO(" top::TauObjectCollectionMaker initialize");

    top::check(m_calibrationTool.retrieve(), "Failed to retrieve tau calibration tool");
    if (m_config->isMC())
      top::check(m_truthMatchingTool.retrieve(), "Failed to retrieve tau truth matching tool");

    ///-- Set Systematics Information --///
    const std:: string& syststr = m_config->systematics();
    std::set<std::string> syst;

    if (!m_config->isSystNominal(syststr) && !m_config->isSystAll(syststr)) {
      bool ok = m_config->getSystematicsList(syststr, syst);
      if (!ok) {
        ATH_MSG_ERROR(" top::TauObjectCollectionMaker could not determine systematic list");
        return StatusCode::FAILURE;
      }
      //here the idea is that if the user specifies AllXXX, we leave syst as an empty string, so that all recommended CP
      // systematics are then used
      if (m_config->contains(syst, "AllTaus")) {
        syst.clear();
      }
    }

    specifiedSystematics(syst);

    m_config->systematicsTaus(specifiedSystematics());

    return StatusCode::SUCCESS;
  }

  StatusCode TauObjectCollectionMaker::execute(bool executeNominal) {
    ///-- Get base taus from xAOD --///
    const xAOD::TauJetContainer* xaod(nullptr);

    top::check(evtStore()->retrieve(xaod, m_config->sgKeyTaus()), "Failed to retrieve Taus");

    ///-- Loop over all systematics --///
    for (auto systematic : m_specifiedSystematics) {
      ///-- if executeNominal, skip other systematics (and vice-versa) --///
      if (executeNominal && !m_config->isSystNominal(m_config->systematicName(systematic.hash()))) continue;
      if (!executeNominal && m_config->isSystNominal(m_config->systematicName(systematic.hash()))) continue;

      ///-- Tell tool which systematic to use --///
      top::check(m_calibrationTool->applySystematicVariation(systematic), "Failed to applySystematicVariation");

      ///-- Shallow copy of the xAOD --///
      std::pair< xAOD::TauJetContainer*, xAOD::ShallowAuxContainer* > shallow_xaod_copy = xAOD::shallowCopyContainer(
        *xaod);

      ///-- Loop over the xAOD Container and apply corrections--///
      for (auto tau : *(shallow_xaod_copy.first)) {
        ///-- add the necessary decoration
        if (m_config->isMC())
          m_truthMatchingTool->getTruth(*tau);

        ///-- Apply momentum correction --///
        top::check(m_calibrationTool->applyCorrection(*tau), "Failed to applyCorrection");
      }


      ///-- set links to original objects- needed for MET calculation --///
      bool setLinks = xAOD::setOriginalObjectLink(*xaod, *shallow_xaod_copy.first);
      if (!setLinks) ATH_MSG_ERROR(" Cannot set original object links for taus, MET recalculation may struggle");

      // Save corrected xAOD Container to StoreGate / TStore
      std::string outputSGKey = m_config->sgKeyTaus(systematic.hash());
      std::string outputSGKeyAux = outputSGKey + "Aux.";

      StatusCode save = evtStore()->tds()->record(shallow_xaod_copy.first, outputSGKey);
      StatusCode saveAux = evtStore()->tds()->record(shallow_xaod_copy.second, outputSGKeyAux);
      if (!save || !saveAux) {
        return StatusCode::FAILURE;
      }
    }  // Loop over all systematics

    return StatusCode::SUCCESS;
  }

  StatusCode TauObjectCollectionMaker::printout() {
    ///-- Loop over all systematics --///
    for (auto s : m_specifiedSystematics) {
      const xAOD::TauJetContainer* xaod(nullptr);
      top::check(evtStore()->retrieve(xaod, m_config->sgKeyTaus(s.hash())), "Failed to retrieve Taus");

      ATH_MSG_INFO(" Taus with sgKey = " << m_config->sgKeyTaus(s.hash()));
      for (auto x : *xaod) {
        ATH_MSG_INFO("   TAU pT , eta  = " << x->pt() << " , " << x->eta());
      }
    }

    return StatusCode::SUCCESS;
  }

  void TauObjectCollectionMaker::specifiedSystematics(const std::set<std::string>& specifiedSystematics) {
    ///-- Get the recommended systematics from the tool, in std::vector format --///
    const std::vector<CP::SystematicSet> systList = CP::make_systematics_vector(
      m_calibrationTool->recommendedSystematics());

    for (auto s : systList) {
      
      if(!m_config->getTreeFilter()->filterTree(s.name())) continue; // Applying tree filter
      m_recommendedSystematics.push_back(s);
      if (s.name() == "") {
        m_specifiedSystematics.push_back(s);
      }

      ///-- MC only --///
      if (m_config->isMC()) {
        ///-- Are we only doing Nominal? Did the user specify specific systematics to use? --///
        if (!m_config->isSystNominal(m_config->systematics())) {
          if (specifiedSystematics.size() == 0) {
            m_specifiedSystematics.push_back(s);
          }
          if (specifiedSystematics.size() > 0) {
            for (auto i : specifiedSystematics) {
              TreeFilter filter(i);
              if (!filter.filterTree(s.name())) {
                m_specifiedSystematics.push_back(s);
              }
            }
          }
        }
      }
    }
    m_recommendedSystematics.sort();
    m_recommendedSystematics.unique();
    m_specifiedSystematics.sort();
    m_specifiedSystematics.unique();
  }
}
