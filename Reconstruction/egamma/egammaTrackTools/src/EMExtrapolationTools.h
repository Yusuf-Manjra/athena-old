/*
   Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EGAMMATRACKTOOLS_EMEXTRAPOLATIONTOOLS_H
#define EGAMMATRACKTOOLS_EMEXTRAPOLATIONTOOLS_H
/**
  @class EMExtrapolationTools
  Tools for track extrapolation to the calorimeter
  @author Thomas Koffas, Christos Anastopoulos
  */
/********************************************************************
NAME:     EMExtrapolationTools.h
PACKAGE:  offline/Reconstruction/egammaTrackTools
AUTHORS:  T. Koffas, C.Anastopoulos
PURPOSE:  Tool which propagate track and vertices to the calorimeter cluster
********************************************************************/

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "egammaInterfaces/IEMExtrapolationTools.h"

#include "TrkNeutralParameters/NeutralParameters.h"
#include "TrkParameters/TrackParameters.h"
#include "xAODCaloEvent/CaloClusterFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/VertexFwd.h"

#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "TrkCaloExtension/CaloExtensionCollection.h"
#include "TrkCaloExtension/CaloExtensionHelpers.h"
#include "TrkExInterfaces/IExtrapolator.h"

class TRT_ID;
class CaloDepthTool;

namespace Trk {
class INeutralParticleParameterCalculator;
}

class EMExtrapolationTools final
  : virtual public IEMExtrapolationTools
  , public AthAlgTool
{

public:
  /** @brief Constructor with parameters */
  EMExtrapolationTools(const std::string& type,
                       const std::string& name,
                       const IInterface* parent);
  /** @brief Destructor */
  virtual ~EMExtrapolationTools();

  /** @brief initialize method */
  virtual StatusCode initialize() override final;

  virtual std::pair<std::vector<CaloSampling::CaloSample>,
                    std::vector<std::unique_ptr<Trk::Surface>>>
  getClusterLayerSurfaces(
    const xAOD::CaloCluster& cluster,
    const CaloDetDescrManager& caloDD) const override final;
  /**   get eta, phi, deltaEta, and deltaPhi at the four calorimeter
   *    layers given the Trk::ParametersBase.  */
  virtual StatusCode getMatchAtCalo(
    const EventContext& ctx,
    const xAOD::CaloCluster& cluster,
    const xAOD::TrackParticle& trkPB,
    const std::vector<CaloSampling::CaloSample>& samples,
    const std::vector<std::unique_ptr<Trk::Surface>>& surfaces,
    std::array<double, 4>& eta,
    std::array<double, 4>& phi,
    std::array<double, 4>& deltaEta,
    std::array<double, 4>& deltaPhi,
    unsigned int extrapFrom = fromPerigee) const override final;

  /** test for vertex-to-cluster match given also the positions
    at the calorimeter from the vertex extrapolation  **/
  virtual bool matchesAtCalo(const xAOD::CaloCluster* cluster,
                             const xAOD::Vertex* vertex,
                             float etaAtCalo,
                             float phiAtCalo) const override final;

  /** get eta, phi at EM2 given a vertex which is converted to
    NeutralParameters. Return false if the extrapolation fails **/
  virtual bool getEtaPhiAtCalo(const EventContext& ctx,
                               const xAOD::Vertex* vertex,
                               float* etaAtCalo,
                               float* phiAtCalo) const override final;

  /** get eta, phi at EM2 given NeutralParameters.
    Return false if the extrapolation fails **/
  virtual bool getEtaPhiAtCalo(const EventContext& ctx,
                               const Trk::TrackParameters* trkPar,
                               float* etaAtCalo,
                               float* phiAtCalo) const override final;

  /** get the momentum of the i-th trackParticle assiciated to the vertex
   * at vertex (designed for conversions) **/
  Amg::Vector3D getMomentumAtVertex(const EventContext& ctx,
                                    const xAOD::Vertex&,
                                    unsigned int) const override final;

  /** get sum of the momenta at the vertex (designed for conversions).
   * Retrieve from auxdata if available and \<reuse\> is true **/
  Amg::Vector3D getMomentumAtVertex(const EventContext& ctx,
                                    const xAOD::Vertex&,
                                    bool reuse = true) const override final;

private:
  /** @brief Return +/- 1 (2) if track is in positive/negative TRT barrel
   * (endcap) **/
  int getTRTsection(const xAOD::TrackParticle* trkPB) const;

  ToolHandle<Trk::IParticleCaloExtensionTool> m_ParticleCaloExtensionTool{
    this,
    "CaloExtensionTool",
    "Trk::ParticleCaloExtensionTool/EMParticleCaloExtensionTool"
  };

  ToolHandle<Trk::IExtrapolator> m_extrapolator{
    this,
    "Extrapolator",
    "Trk::Extrapolator/AtlasExtrapolator"
  };

  // vertex-to-cluster match cuts used in matchesAtCalo
  Gaudi::Property<double> m_narrowDeltaPhi{ this, "NarrowDeltaPhi", 0.05 };
  Gaudi::Property<double> m_narrowDeltaPhiTRTbarrel{ this,
                                                     "NarrowDeltaPhiTRTbarrel",
                                                     0.02 };
  Gaudi::Property<double> m_narrowDeltaPhiTRTendcap{ this,
                                                     "NarrowDeltaPhiTRTendcap",
                                                     0.02 };
  Gaudi::Property<double> m_narrowDeltaEta{ this, "NarrowDeltaEta", 0.05 };
  Gaudi::Property<double> m_TRTbarrelDeltaEta{ this,
                                               "TRTbarrelDeltaEta",
                                               0.35 };
  Gaudi::Property<double> m_TRTendcapDeltaEta{ this, "TRTendcapDeltaEta", 0.2 };

  // ID TRT helper
  const TRT_ID* m_trtId;
  Gaudi::Property<bool> m_enableTRT{ this, "EnableTRT", true };
};

#endif
