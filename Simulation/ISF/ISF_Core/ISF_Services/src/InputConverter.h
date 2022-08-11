/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// InputConverter.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_INPUTCONVERTER_H
#define ISF_INPUTCONVERTER_H 1

// STL includes
#include <string>
// ISF include
#include "ISF_Interfaces/IInputConverter.h"
// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthService.h"

#include "BarcodeEvent/Barcode.h"

// forward declarations
namespace Barcode {
  class IBarcodeSvc;
}
namespace HepPDT {
  class ParticleDataTable;
}
#include "AtlasHepMC/GenEvent_fwd.h"
#include "AtlasHepMC/GenParticle_fwd.h"

class IPartPropSvc;
class McEventCollection;
namespace ISFTesting {
  class InputConverter_test;
}
namespace ISF {
  class ISFParticle;
  class IGenParticleFilter;
}

class G4ParticleDefinition;
class G4PrimaryParticle;
class G4VSolid;

namespace ISF {

  /** @class InputConverter

      Convert simulation input collections to ISFParticles for subsequent ISF simulation.

      @author Elmar.Ritsch -at- cern.ch
     */
  class InputConverter final: public extends<AthService, IInputConverter> {

    // allow test to access private data
    friend ISFTesting::InputConverter_test;

  public:
    InputConverter(const std::string& name, ISvcLocator* svc);
    virtual ~InputConverter();

    /** Athena algtool Hooks */
    virtual StatusCode  initialize() override final;
    virtual StatusCode  finalize() override final;

    /** Convert selected particles from the given McEventCollection into ISFParticles
        and push them into the given ISFParticleContainer */
    virtual StatusCode convert(const McEventCollection& inputGenEvents,
                               ISF::ISFParticleContainer& simParticles,
                               EBC_EVCOLL kindOfCollection=EBC_MAINEVCOLL) const override final;

    /** */
    virtual StatusCode convertHepMCToG4Event(McEventCollection& inputGenEvents,
                                             G4Event*& outputG4Event,
                                             EBC_EVCOLL kindOfCollection=EBC_MAINEVCOLL) const override final;

    /** Converts vector of ISF::ISFParticles to G4Event */
    G4Event* ISF_to_G4Event(const std::vector<const ISF::ISFParticle*>& isp, HepMC::GenEvent *genEvent, bool useHepMC=false) const override final;

  private:

    const G4ParticleDefinition* getG4ParticleDefinition(int pdgcode) const;

#ifdef HEPMC3
    G4PrimaryParticle* getG4PrimaryParticle(HepMC::ConstGenParticlePtr gp) const;
#else
    G4PrimaryParticle* getG4PrimaryParticle(const HepMC::GenParticle& gp) const;
#endif

    G4PrimaryParticle* getG4PrimaryParticle(const ISF::ISFParticle& isp, bool useHepMC) const;

    void addG4PrimaryVertex(G4Event* g4evt, const ISF::ISFParticle& isp, bool useHepMC) const;

    /** Tests whether the given ISFParticle is within the Geant4 world volume */
    bool isInsideG4WorldVolume(const ISF::ISFParticle& isp, const G4VSolid* worldSolid) const;

    /** get right GenParticle mass */
#ifdef HEPMC3
    double getParticleMass(HepMC::ConstGenParticlePtr p) const;
#else
    double getParticleMass(const HepMC::GenParticle& p) const;
#endif

    /** get all generator particles which pass filters */
#ifdef HEPMC3
    std::vector<HepMC::ConstGenParticlePtr > getSelectedParticles(const HepMC::GenEvent& evnt, bool legacyOrdering=false) const;
#else
    std::vector<HepMC::GenParticlePtr > getSelectedParticles(const HepMC::GenEvent& evnt, bool legacyOrdering=false) const;
#endif

    /** check if the given particle passes all filters */
#ifdef HEPMC3
    bool passesFilters(HepMC::ConstGenParticlePtr p) const;
#else
    bool passesFilters(const HepMC::GenParticle& p) const;
#endif

    /** convert GenParticle to ISFParticle */
#ifdef HEPMC3
    ISF::ISFParticle* convertParticle(HepMC::ConstGenParticlePtr genPartPtr, EBC_EVCOLL kindOfCollection=EBC_MAINEVCOLL) const;
#else
    ISF::ISFParticle* convertParticle(HepMC::GenParticlePtr genPartPtr, EBC_EVCOLL kindOfCollection=EBC_MAINEVCOLL) const;
#endif

    /** ParticlePropertyService and ParticleDataTable */
    ServiceHandle<IPartPropSvc>           m_particlePropSvc;          //!< particle properties svc to retrieve PDT
    const HepPDT::ParticleDataTable      *m_particleDataTable;        //!< PDT used to look up particle masses

    bool                                  m_useGeneratedParticleMass; //!< use GenParticle::generated_mass() in simulation

    ToolHandleArray<IGenParticleFilter>   m_genParticleFilters;       //!< HepMC::GenParticle filters

    bool                                  m_quasiStableParticlesIncluded; //<! will quasi-stable particles be included in the simulation

    ServiceHandle<Barcode::IBarcodeSvc>   m_barcodeSvc;                 //!< The ISF Barcode service
    Barcode::ParticleBarcode              m_barcodeGenerationIncrement; //!< to be retrieved from ISF Barcode service

  };

}


#endif //> !ISF_INPUTCONVERTER_H
