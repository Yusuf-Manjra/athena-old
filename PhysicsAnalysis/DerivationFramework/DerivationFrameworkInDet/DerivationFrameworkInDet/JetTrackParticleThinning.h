/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// JetTrackParticleThinning.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_JETTRACKPARTICLETHINNING_H
#define DERIVATIONFRAMEWORK_JETTRACKPARTICLETHINNING_H

#include <string>
#include <atomic>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/ThinningHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

  enum EJetTrPThinningParser { kJetSelection, kTrackThinning, kNumJetTPThinningParser};
  class JetTrackParticleThinning : public extends<ExpressionParserUser<AthAlgTool, kNumJetTPThinningParser>, IThinningTool> {
    public: 
      JetTrackParticleThinning(const std::string& t, const std::string& n, const IInterface* p);
      virtual ~JetTrackParticleThinning();
      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
      virtual StatusCode doThinning() const override;

    private:
      mutable std::atomic<unsigned int> m_ntot  {};
      mutable std::atomic<unsigned int> m_npass {};
      SG::ReadHandleKey<xAOD::JetContainer> m_jetKey
        { this, "JetKey", "", ""};

      StringProperty m_streamName
        { this, "StreamName", "", "Name of the stream being thinned" };
      SG::ThinningHandleKey<xAOD::TrackParticleContainer> m_inDetSGKey
        { this, "InDetTrackParticlesKey", "InDetTrackParticles", "" };
      StringProperty m_selectionString
        { this, "SelectionString", "", "" };
      StringProperty m_trackSelectionString
        { this, "TrackSelectionString", "", "" };
  }; 
}

#endif // DERIVATIONFRAMEWORK_JETTRACKPARTICLETHINNING_H
