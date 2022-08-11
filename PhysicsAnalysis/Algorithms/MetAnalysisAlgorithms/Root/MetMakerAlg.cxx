/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <MetAnalysisAlgorithms/MetMakerAlg.h>

#include <xAODMissingET/MissingETAuxContainer.h>
#include <xAODMissingET/MissingETAssociationHelper.h>

//
// method implementations
//

namespace CP
{
  MetMakerAlg ::
  MetMakerAlg (const std::string& name, 
                     ISvcLocator* pSvcLocator)
    : AnaAlgorithm (name, pSvcLocator)
    , m_makerTool ("METMaker", this)
    , m_systematicsTool ("", this)
  {
    declareProperty ("makerTool", m_makerTool, "the METMaker tool we apply");
    declareProperty ("systematicsTool", m_systematicsTool, "the systematics tool we apply");
    declareProperty ("metCore", m_metCoreName, "the name of the core MissingETContainer");
    declareProperty ("metAssociation", m_metAssociationName, "the name of the core MissingETContainer");
    declareProperty ("electronsKey", m_electronsKey, "the key for the electrons");
    declareProperty ("photonsKey", m_photonsKey, "the key for the photons");
    declareProperty ("muonsKey", m_muonsKey, "the key for the muons");
    declareProperty ("tausKey", m_tausKey, "the key for the taus");
    declareProperty ("jetsKey", m_jetsKey, "the key for jets");
    declareProperty ("softTermKey", m_softTermKey, "the soft term key");
    declareProperty ("doTrackMet", m_doTrackMet, "whether to use track-met instead of jet-met");
    declareProperty ("doJetJVT", m_doJetJVT, "whether to do jet JVT");
  }



  StatusCode MetMakerAlg ::
  initialize ()
  {
    ANA_CHECK (m_makerTool.retrieve());

    for (auto* handle : {&m_electronsHandle, &m_photonsHandle,
                         &m_muonsHandle, &m_tausHandle, &m_invisHandle}) {
      ANA_CHECK (handle->initialize (m_systematicsList, SG::AllowEmpty));
    }
    ANA_CHECK (m_jetsHandle.initialize (m_systematicsList));
    ANA_CHECK (m_metHandle.initialize (m_systematicsList));

    if (!m_systematicsTool.empty())
    {
      ANA_CHECK (m_systematicsTool.retrieve());
      ANA_CHECK (m_systematicsList.addSystematics (*m_systematicsTool));
    }

    ANA_CHECK (m_systematicsList.initialize());

    return StatusCode::SUCCESS;
  }



  StatusCode MetMakerAlg ::
  execute ()
  {
    const xAOD::MissingETContainer* metcore {nullptr};
    ANA_CHECK (evtStore()->retrieve(metcore, m_metCoreName));

    const xAOD::MissingETAssociationMap* metMap {nullptr};
    ANA_CHECK (evtStore()->retrieve(metMap, m_metAssociationName));

    // Helper keeps track of object selection flags for this map
    xAOD::MissingETAssociationHelper metHelper(&(*metMap));

    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      auto met = std::make_unique<xAOD::MissingETContainer> ();
      auto aux = std::make_unique<xAOD::MissingETAuxContainer> ();
      met->setStore (aux.get());

      metHelper.resetObjSelectionFlags();

      if (m_invisHandle) {
        const xAOD::IParticleContainer* invisible = nullptr;
        ATH_CHECK( m_invisHandle.retrieve(invisible, sys) );
        ATH_CHECK( m_makerTool->markInvisible(invisible, metHelper, met.get() ) );
      }

      // Lambda helping with calculating the MET terms coming from the leptons
      // (and photons).
      auto processParticles =
        [&] (SysReadHandle<xAOD::IParticleContainer>& handle,
             xAOD::Type::ObjectType type,
             const std::string& term) -> StatusCode {
          if (!handle) {
            return StatusCode::SUCCESS;
          }
          const xAOD::IParticleContainer* particles = nullptr;
          ANA_CHECK (handle.retrieve (particles, sys));
          ANA_CHECK (m_makerTool->rebuildMET (term, type, met.get(),
                                              particles, metHelper));
          return StatusCode::SUCCESS;
        };

      // Calculate the terms coming from the user's selected objects.
      ANA_CHECK (processParticles (m_electronsHandle, xAOD::Type::Electron,
                                   m_electronsKey));
      ANA_CHECK (processParticles (m_photonsHandle, xAOD::Type::Photon,
                                   m_photonsKey));
      ANA_CHECK (processParticles (m_tausHandle, xAOD::Type::Tau, m_tausKey));
      ANA_CHECK (processParticles (m_muonsHandle, xAOD::Type::Muon,
                                   m_muonsKey));

      const xAOD::JetContainer *jets {nullptr};
      ANA_CHECK (m_jetsHandle.retrieve (jets, sys));
	
      if (m_doTrackMet)
      {
        ANA_CHECK (m_makerTool->rebuildTrackMET (m_jetsKey, m_softTermKey, met.get(), jets, metcore, metHelper, m_doJetJVT));
      } else
      {
        ANA_CHECK (m_makerTool->rebuildJetMET (m_jetsKey, m_softTermKey, met.get(), jets, metcore, metHelper, m_doJetJVT));
      }

      // Systematics
      if (!m_systematicsTool.empty())
      {
        ANA_CHECK (m_systematicsTool->applySystematicVariation (sys));

        xAOD::MissingET *softTerm = (*met)[m_softTermKey];
        if (softTerm == nullptr)
        {
          ANA_MSG_ERROR ("failed to find MET soft-term \"" << m_softTermKey << "\"");
          return StatusCode::FAILURE;
        }

        // This returns a `CorrectionCode`, so in principle this could
        // return an `OutOfValidity` result, but I have no idea what
        // that would mean or how to handle it, so I'm implicitly
        // converting it into a `FAILURE` instead.
        ANA_CHECK (m_systematicsTool->applyCorrection (*softTerm, metHelper));
      }

      ANA_CHECK (m_metHandle.record (std::move (met), std::move (aux), sys));
    }

    return StatusCode::SUCCESS;
  }
}
