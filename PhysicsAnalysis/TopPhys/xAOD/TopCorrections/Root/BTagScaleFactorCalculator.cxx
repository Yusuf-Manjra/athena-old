/*
   Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

// $Id: BTagScaleFactorCalculator.cxx 809570 2017-08-18 13:15:17Z iconnell $
#include "TopCorrections/BTagScaleFactorCalculator.h"
#include "TopCorrections/TopCorrectionsTools.h"
#include "TopConfiguration/TopConfig.h"
#include "TopEvent/EventTools.h"

#include "xAODJet/JetContainer.h"
#include <cmath>
#include <algorithm>
#include <functional>

// For debug function
#include "AsgMessaging/StatusCode.h"
#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicVariation.h"
#include "CalibrationDataInterface/CalibrationDataInterfaceROOT.h"
#include "PathResolver/PathResolver.h"

namespace top {
  BTagScaleFactorCalculator::BTagScaleFactorCalculator(const std::string& name) :
    asg::AsgTool(name),
    m_config(nullptr),
    m_nominal(CP::SystematicSet()) {
    declareProperty("config", m_config);
  }

  StatusCode BTagScaleFactorCalculator::initialize() {
    ATH_MSG_INFO(" top::BTagScaleFactorCalculator initialize");

    // for calo jets
    for (const std::string& WP : m_config->bTagWP_calib()) {
      m_btagSelTools[WP] = "BTaggingSelectionTool_" + WP + "_" + m_config->sgKeyJets();
      top::check(m_btagSelTools[WP].retrieve(), "Failed to retrieve b-tagging Selection tool");
      m_btagEffTools[WP] = "BTaggingEfficiencyTool_" + WP + "_" + m_config->sgKeyJets();
      top::check(m_btagEffTools[WP].retrieve(), "Failed to retrieve b-tagging Efficiency tool");
      m_systs[WP] = m_btagEffTools[WP]->affectingSystematics();
      std::set<std::string> base_names = m_systs[WP].getBaseNames();
      m_config->setBTaggingSFSysts(WP, base_names);
    }
    if (m_config->useTrackJets()) {
      // for track jets
      for (const std::string& WP : m_config->bTagWP_calib_trkJet()) {
        m_trkjet_btagSelTools[WP] = "BTaggingSelectionTool_" + WP + "_" + m_config->sgKeyTrackJets();
        top::check(m_trkjet_btagSelTools[WP].retrieve(), "Failed to retrieve b-tagging Selection tool");
        m_trkjet_btagEffTools[WP] = "BTaggingEfficiencyTool_" + WP + "_" + m_config->sgKeyTrackJets();
        top::check(m_trkjet_btagEffTools[WP].retrieve(), "Failed to retrieve b-tagging Efficiency tool");
        m_trkjet_systs[WP] = m_trkjet_btagEffTools[WP]->affectingSystematics();
        std::set<std::string> base_names = m_trkjet_systs[WP].getBaseNames();
        m_config->setBTaggingSFSysts(WP, base_names, true);
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode BTagScaleFactorCalculator::execute() {
    top::check(apply(m_config->systSgKeyMapJets(false)),
               "Failed to apply btagging SFs");
    if (m_config->useTrackJets()) top::check(apply(m_config->systSgKeyMapTrackJets(), true),
                                             "Failed to apply track jet btagging SFs");

    return StatusCode::SUCCESS;
  }

  StatusCode BTagScaleFactorCalculator::apply(const std::shared_ptr<std::unordered_map<std::size_t,
                                                                                       std::string> >& jet_syst_collections,
                                              bool use_trackjets) {
    ///-- Loop over all jet collections --///
    ///-- Lets assume that we're not doing ElectronInJet subtraction --///
    for (auto currentSystematic : *jet_syst_collections) {
      const xAOD::JetContainer* jets(nullptr);
      top::check(evtStore()->retrieve(jets, currentSystematic.second), "failed to retrieve jets");

      ///-- Tell the SF tools to use the nominal systematic --///
      /// -- Loop over all jets in each collection --///
      for (auto jetPtr : *jets) {
        bool passSelection(false);
        if (jetPtr->isAvailable<char>("passPreORSelection")) {
          if (jetPtr->auxdataConst<char>("passPreORSelection") == 1) {
            passSelection = true;
          }
        }
        if (jetPtr->isAvailable<char>("passPreORSelectionLoose")) {
          if (jetPtr->auxdataConst<char>("passPreORSelectionLoose") == 1) {
            passSelection = true;
          }
        }

        if (passSelection) {
          // now loop over all available WPs

          for (const std::string& tagWP : (use_trackjets ? m_config->bTagWP_calib_trkJet() : m_config->bTagWP_calib())) {
            ToolHandle<IBTaggingEfficiencyTool>& btageff =
              use_trackjets ? m_trkjet_btagEffTools[tagWP] : m_btagEffTools[tagWP];
            ToolHandle<IBTaggingSelectionTool>& btagsel =
              use_trackjets ? m_trkjet_btagSelTools[tagWP] : m_btagSelTools[tagWP];
            CP::SystematicSet& sysSet = use_trackjets ? m_trkjet_systs[tagWP] : m_systs[tagWP];

            // need now the DSID to find out which shower was used in the sample
            unsigned int MapIndex = m_config->getMapIndex();

            btageff->setMapIndex("Light", MapIndex);
            btageff->setMapIndex("C", MapIndex);
            btageff->setMapIndex("B", MapIndex);
            btageff->setMapIndex("T", MapIndex);

            // Check if this jet collection systematic matches with one removed from the EV decomposition
            // (TopCorrectionsTools)
            std::string bTagSystName = top::bTagNamedSystCheck(m_config, currentSystematic.second, tagWP, use_trackjets, false);
            // If this string is not empty, we need to search and find the appropriate systematic set to apply
            if (bTagSystName != "") {
              CP::SystematicSet bTagSyst;
              bTagSyst.insert(sysSet.getSystematicByBaseName(bTagSystName));
              top::check(btageff->applySystematicVariation(bTagSyst),
                         "Failed to set new b-tagging SF to a shifted systematic set : " + bTagSystName);
            } else {
              top::check(btageff->applySystematicVariation(m_nominal),
                         "Failed to set new b-tagging SF to nominal");
            }

            float btag_SF(1.0);
            bool isTagged = false;//unused in case of Continuous
            if (std::fabs(jetPtr->eta()) <= 2.5) {
              if (tagWP.find("Continuous") == std::string::npos) {
                isTagged = static_cast<bool>(btagsel->accept(*jetPtr));
                if (isTagged) top::check(btageff->getScaleFactor(*jetPtr, btag_SF),
                                         "Failed to get nominal b-tagging SF");
                else top::check(btageff->getInefficiencyScaleFactor(*jetPtr, btag_SF),
                                "Failed to get nominal b-tagging SF");
              } else {
                top::check(btageff->getScaleFactor(*jetPtr, btag_SF),
                           "Failed to get nominal Continuous b-tagging SF");
              }
            }
            jetPtr->auxdecor<float>("btag_SF_" + tagWP + "_nom") = btag_SF;

            ///-- For nominal calibration, vary the SF systematics --///
            if (currentSystematic.first == m_config->nominalHashValue()) {
              for (const auto& variation : sysSet) {
                btag_SF = 1.;
                CP::SystematicSet syst_set;
                syst_set.insert(variation);
                top::check(btageff->applySystematicVariation(syst_set),
                           "Failed to set new b-tagging systematic variation " + syst_set.name());
                if (std::fabs(jetPtr->eta()) <= 2.5) {
                  if (tagWP.find("Continuous") == std::string::npos) {
                    if (isTagged) top::check(btageff->getScaleFactor(*jetPtr, btag_SF),
                                             "Failed to get b-tagging SF for variation " + syst_set.name());
                    else top::check(btageff->getInefficiencyScaleFactor(*jetPtr, btag_SF),
                                    "Failed to get b-tagging SF for variation " + syst_set.name());
                  } else {
                    top::check(btageff->getScaleFactor(*jetPtr, btag_SF),
                               "Failed to get Continuous b-tagging SF for variation " + syst_set.name());
                  }
                }
                jetPtr->auxdecor<float>("btag_SF_" + tagWP + "_" + variation.name()) = btag_SF;
              } // loop through b-tagging systematic variations
            } // Calibration systematic is nominal, so calculate SF systematics
          }
        }
      }
    }


    return StatusCode::SUCCESS;
  }

  StatusCode BTagScaleFactorCalculator::debug() {
    ATH_MSG_INFO("BTagScaleFactorCalculator::debug function");
    // Use after package is initialised otherwise vectors will be empty
    for (const std::string& tagWP :  m_config->bTagWP()) {
      ATH_MSG_INFO("Tagger working point : " << tagWP);
      ToolHandle<IBTaggingEfficiencyTool>& btageff = m_btagEffTools[tagWP];
      // Retrieve tool
      top::check(btageff.retrieve(), "Failed to retrieve tool");
      // Retrieve list of systematics included
      CP::SystematicSet systs = btageff->affectingSystematics();
      CP::SystematicSet recsysts = btageff->recommendedSystematics();

      ATH_MSG_INFO("-----------------------------------------------------------------------");

      const std::map<CP::SystematicVariation,
                     std::vector<std::string> > allowed_variations = btageff->listSystematics();

      ATH_MSG_INFO("Allowed systematics variations for tool " << btageff->name());
      for (auto var : allowed_variations) {
        std::string flvs = "";
        for (auto flv : var.second) flvs += flv;
        ATH_MSG_INFO(" (" << flvs << ") - " << var.first);
      }

      // Now use the new functions added into xAODBTaggingEfficiency-00-00-39
      // std::map<std::string, std::vector<std::string> >
      ATH_MSG_INFO("-----------------------------------------------------------------------");
      ATH_MSG_INFO("List of b-tagging scale factor systematics");
      std::map<std::string,
               std::vector<std::string> >  listOfScaleFactorSystematics = btageff->listScaleFactorSystematics(false);
      for (auto var : listOfScaleFactorSystematics) {
        ATH_MSG_INFO("Jet flavour : " << var.first);
        std::vector<std::string> systs = var.second;
        std::sort(systs.begin(), systs.end());
        for (auto sys : systs) {
          ATH_MSG_INFO(" (" << var.first << ") - " << sys);
        }
        ATH_MSG_INFO(" ");
      }

      ATH_MSG_INFO("List of (named) b-tagging scale factor systematics");
      listOfScaleFactorSystematics = btageff->listScaleFactorSystematics(true);
      for (auto var : listOfScaleFactorSystematics) {
        ATH_MSG_INFO("Jet flavour : " << var.first);
        std::vector<std::string> systs = var.second;
        std::sort(systs.begin(), systs.end());
        for (auto sys : systs) {
          ATH_MSG_INFO(" (" << var.first << ") - " << sys);
        }
        ATH_MSG_INFO(" ");
      }
      ATH_MSG_INFO("-----------------------------------------------------------------------");
    }
    return StatusCode::SUCCESS;
  }
}
