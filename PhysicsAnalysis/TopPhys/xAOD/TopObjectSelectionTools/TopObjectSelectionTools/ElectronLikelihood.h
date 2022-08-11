/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TOPOBJECTSELECTIONTOOLSELECTRONLIKELIHOOD_H_
#define TOPOBJECTSELECTIONTOOLSELECTRONLIKELIHOOD_H_

#include "TopObjectSelectionTools/ElectronSelectionBase.h"
#include "TopObjectSelectionTools/IsolationTools.h"
#include "EgammaAnalysisInterfaces/IAsgDeadHVCellRemovalTool.h"
#include "AsgTools/AnaToolHandle.h"
#include <memory>

namespace top {
  /**
   * @brief To select electrons based on the "likelihood" definition.
   */
  class ElectronLikelihood: public top::ElectronSelectionBase {
  public:
    /**
     * @brief Cut on likelihood electrons
     *
     * @param ptcut The minimum pT electrons should have
     * @param vetoCrack Do you want to veto 1.37 < |cluster_eta| < 1.52?
     * @param operatingPoint Likelihood operating point for the main object
     * definition
     * @param operatingPointLoose Likelihood operating point for the loose
     * objects (fakes estimates?)
     * @param isolation The isolation tool the user wants to use. If you don't
     * want any isolation cuts to be applied then leave this as a nullptr.
     * @param d0SigCut The maximum d0 significance cut
     * @param delta_z0 The maximum |delta z0 sin(theta)| cut
     * @param applyTTVACut Whether to apply cuts on d0 and z0
     * @param applyChargeIDCut Whether to apply charge identification selector tool
     */
    ElectronLikelihood(const double ptcut, const bool vetoCrack, const std::string& operatingPoint,
                       const std::string& operatingPointLoose, StandardIsolation* isolation,
                       const bool applyChargeIDCut);

    ElectronLikelihood(const double ptcut, const bool vetoCrack, const std::string& operatingPoint,
                       const std::string& operatingPointLoose, StandardIsolation* isolation,
                       const bool applyTTVACut, const bool applyChargeIDCut);

    ElectronLikelihood(const double ptcut, const bool vetoCrack, const std::string& operatingPoint,
                       const std::string& operatingPointLoose, StandardIsolation* isolation,
                       const double d0SigCut, const double delta_z0, const bool applyTTVACut,
                       const bool applyChargeIDCut);

    virtual ~ElectronLikelihood() {}
    /**
     * @brief Selection for the main analysis (i.e. tight object definitions).
     *
     * @param el The electron in question
     * @return True if it passes the selection, false otherwise.
     */
    virtual bool passSelection(const xAOD::Electron& el) const override;

    /**
     * @brief Selection for the loose object definitions (e.g. in fakes
     * estimates).
     *
     * @param el The electron in question
     * @return True if it passes the selection, false otherwise.
     */
    virtual bool passSelectionLoose(const xAOD::Electron& el) const override;

    /**
     * @brief The BLayer cuts for the loose lepton LH WP.
     *
     * @param el
     * @return True if passes, false otherwise
     */
    virtual bool passBLayerCuts(const xAOD::Electron& el) const;


    /**
     * @brief The track-to-vertex association (TTVA) cuts.
     *
     * @param el
     * @return True if passes, false otherwise
     */
    virtual bool passTTVACuts(const xAOD::Electron& el) const;

    /**
     * @brief The charge flip selector cut
     *
     * @param el
     * @return True if passes, false otherwise
     */
    virtual bool passChargeIDCut(const xAOD::Electron& el) const;

    /**
     * @brief Why would you not want to know what you're running?
     *
     * @param os Print details of the cuts to the output stream.
     */
    virtual void print(std::ostream& os) const override;
  protected:
    /**
     * @brief The loose and tight selections are the same, except they have different
     * use isolation and a different likelihood tool.
     *
     * @param el The electron in question.
     * @param likelihoodTool The tool to use (m_likelihood, or m_likelihoodLoose)
     * @return True if it's good, False if it's not
     */
    bool passSelectionNoIsolation(const xAOD::Electron& el, const std::string& operatingPoint_DF,
                                  const std::string& operatingPoint) const;

    ///Minimum pT that electrons should have
    double m_ptcut;

    ///Veto the crack region?
    bool m_vetoCrack;

    /// TTVA cuts
    double m_d0SigCut;
    double m_delta_z0;

    std::string m_operatingPoint;
    std::string m_operatingPointLoose;
    std::string m_operatingPoint_DF;
    std::string m_operatingPointLoose_DF;

    ///The isolation tool, or nullptr if we don't want isolation
    std::unique_ptr<top::StandardIsolation> m_isolation;

    /// decide if TTVA cut should be applied at all
    bool m_applyTTVACut;

    /// decide to apply the charge ID selector tool
    bool m_applyChargeIDCut;

    // Removing electron clusters from EMEC bad HV regions
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/EGammaIdentificationRun2#Removal_of_Electron_Photon_clust
    asg::AnaToolHandle<IAsgDeadHVCellRemovalTool> m_deadHVTool;
  };
}

#endif
