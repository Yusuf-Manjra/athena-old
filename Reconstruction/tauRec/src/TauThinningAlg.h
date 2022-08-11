/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration.
*/

#ifndef TAUREC_TAUTHINNINGALG_H
#define TAUREC_TAUTHINNINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODPFlow/PFOContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODParticleEvent/ParticleContainer.h"
#include "StoreGate/ThinningHandleKey.h"
#include "GaudiKernel/SystemOfUnits.h"

/**
 * @brief Thin taus below a minimum pt threshold
 *
 * Thinning algorithm that removes entries from all tau-related containers for tau candidates below a minimum pt threshold
 */
class TauThinningAlg : public AthReentrantAlgorithm
{
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

 public:

  /**
   * @brief Gaudi initialize method.
   */
  virtual StatusCode initialize() override;

  /**
   * @brief Execute the algorithm.
   * @param ctx Current event context.
   */
  virtual StatusCode execute(const EventContext& ctx) const override;


 private:

  // whether tau vertex correction is applied in tau reconstruction
  Gaudi::Property<bool> m_doVertexCorrection
    { this, "VertexCorrection", true, "Tau vertex correction" };

  // Name of the stream being thinned
  StringProperty m_streamName
    { this, "StreamName", "StreamAOD", "Name of the stream being thinned" };

  // Tau container to thin
  SG::ThinningHandleKey<xAOD::TauJetContainer> m_taus
    { this, "Taus", "TauJets", "Tau container to thin" };

  // Tau track container to thin
  SG::ThinningHandleKey<xAOD::TauTrackContainer> m_tauTracks
    { this, "TauTracks", "TauTracks", "Tau track container to thin" };

  // Tau neutral PFO container to thin
  SG::ThinningHandleKey<xAOD::PFOContainer> m_neutralPFOs
    { this, "TauNeutralPFOs", "TauNeutralParticleFlowObjects", "Tau neutral PFO container to thin" };

  // Tau pi0 cluster container to thin
  SG::ThinningHandleKey<xAOD::CaloClusterContainer> m_pi0clusters
    { this, "TauPi0Clusters", "TauPi0Clusters", "Tau pi0 cluster container to thin" };

  // Tau pi0 cluster cell link container to thin
  SG::ThinningHandleKey<CaloClusterCellLinkContainer> m_pi0CellLinks
    { this, "Pi0CellLinks", "TauPi0Clusters_links", "Tau pi0 cluster cell link container to thin" };

  // Tau final pi0 container to thin
  SG::ThinningHandleKey<xAOD::ParticleContainer> m_finalPi0s
    { this, "TauFinalPi0s", "TauFinalPi0s", "Tau final pi0 container to thin" };

  // Tau shot PFO container to thin
  SG::ThinningHandleKey<xAOD::PFOContainer> m_shotPFOs
    { this, "TauShotPFOs", "TauShotParticleFlowObjects", "Tau shot PFO container to thin" };

  // Tau shot cluster container to thin
  SG::ThinningHandleKey<xAOD::CaloClusterContainer> m_shotclusters
    { this, "TauShotClusters", "TauShotClusters", "Tau shot cluster container to thin" };

  // Tau shot cluster cell link container to thin
  SG::ThinningHandleKey<CaloClusterCellLinkContainer> m_shotCellLinks
    { this, "ShotCellLinks", "TauShotClusters_links", "Tau shot cluster cell link container to thin" };

  // Tau hadronic PFO container to thin
  SG::ThinningHandleKey<xAOD::PFOContainer> m_hadronicPFOs
    { this, "TauHadronicPFOs", "TauHadronicParticleFlowObjects", "Tau hadronic PFO container to thin" };
  
  // Tau secondary vertex container to thin
  SG::ThinningHandleKey<xAOD::VertexContainer> m_secondaryVertices
    { this, "TauSecondaryVertices", "TauSecondaryVertices", "Tau secondary vertex container to thin" };

  // Cell container to thin
  SG::ThinningHandleKey<CaloCellContainer> m_cells
    { this, "Cells", "AllCalo", "Cell container to thin" };

  // Tau cluster cell link container to thin
  SG::ThinningHandleKey<CaloClusterCellLinkContainer> m_tauCellLinks
    { this, "TauCellLinks", "CaloCalTopoClusters_links", "Tau cluster cell link container to thin" };

};

#endif // TAUREC_TAUTHINNINGALG_H
