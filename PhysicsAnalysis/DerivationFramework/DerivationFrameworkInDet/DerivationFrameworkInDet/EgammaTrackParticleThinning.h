/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EgammaTrackParticleThinning.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_EGAMMATRACKPARTICLETHINNING_H
#define DERIVATIONFRAMEWORK_EGAMMATRACKPARTICLETHINNING_H

#include <atomic>
#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInDet/TracksInCone.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ThinningHandleKey.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

class EgammaTrackParticleThinning
  : public extends<ExpressionParserUser<AthAlgTool>, IThinningTool>
{
public:
  EgammaTrackParticleThinning(const std::string& t,
                              const std::string& n,
                              const IInterface* p);
  virtual ~EgammaTrackParticleThinning();
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode doThinning() const override;

private:
  mutable std::atomic<unsigned long int> m_ntot = 0;
  mutable std::atomic<unsigned long int> m_ntotGSF = 0;
  mutable std::atomic<unsigned long int> m_npass = 0;
  mutable std::atomic<unsigned long int> m_nGSFPass = 0;
  StringProperty m_streamName{ this,
                               "StreamName",
                               "",
                               "Name of the stream being thinned" };

  SG::ReadHandleKey<xAOD::EgammaContainer> m_egammaKey{ this,
                                                        "SGKey",
                                                        ""
                                                        "" };
  SG::ThinningHandleKey<xAOD::TrackParticleContainer>
    m_inDetSGKey{ this, "InDetTrackParticlesKey", "InDetTrackParticles", "" };

  SG::ThinningHandleKey<xAOD::TrackParticleContainer>
    m_gsfSGKey{ this, "GSFTrackParticlesKey", "GSFTrackParticles", "" };

  Gaudi::Property<std::string> m_selectionString{ this,
                                                  "SelectionString",
                                                  "",
                                                  "" };
  Gaudi::Property<bool> m_bestMatchOnly{ this, "BestMatchOnly", true, "" };
  Gaudi::Property<float> m_coneSize{ this, "ConeSize", -1.0, "" };

  void setPhotonMasks(std::vector<bool>&,
                      std::vector<bool>&,
                      const xAOD::EgammaContainer*,
                      const xAOD::TrackParticleContainer*,
                      const xAOD::TrackParticleContainer*,
                      const bool) const;
  void setPhotonMasks(std::vector<bool>&,
                      std::vector<bool>&,
                      std::vector<const xAOD::Egamma*>&,
                      const xAOD::TrackParticleContainer*,
                      const xAOD::TrackParticleContainer*,
                      const bool) const;
  void setElectronMasks(std::vector<bool>&,
                        std::vector<bool>&,
                        const xAOD::EgammaContainer*,
                        const xAOD::TrackParticleContainer*,
                        const xAOD::TrackParticleContainer*,
                        const bool) const;
  void setElectronMasks(std::vector<bool>&,
                        std::vector<bool>&,
                        std::vector<const xAOD::Egamma*>&,
                        const xAOD::TrackParticleContainer*,
                        const xAOD::TrackParticleContainer*,
                        const bool) const;
};
}

#endif // DERIVATIONFRAMEWORK_EGAMMATRACKPARTICLETHINNING_H
