/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @author Elmar Ritsch
 * @date October 2016
 * @brief A generic particle filter tool for HepMC::GenParticle types
 */


// class header include
#include "GenParticleGenericFilter.h"

// HepMC includes
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/SimpleVector.h"

// STL includes
#include <limits>
#include <algorithm>

/** Constructor **/
ISF::GenParticleGenericFilter::GenParticleGenericFilter( const std::string& t,
                                                         const std::string& n,
                                                         const IInterface* p )
  : base_class(t,n,p),
    m_minEta(std::numeric_limits<decltype(m_minEta)>::lowest()),
    m_maxEta(std::numeric_limits<decltype(m_maxEta)>::max()),
    m_minPhi(-M_PI),
    m_maxPhi(M_PI),
    m_minMom(std::numeric_limits<decltype(m_minMom)>::lowest()),
    m_maxMom(std::numeric_limits<decltype(m_maxMom)>::max()),
    m_pdgs(),
    m_maxApplicableRadius(std::numeric_limits<decltype(m_maxApplicableRadius)>::max())
{
    // different cut parameters
    declareProperty("MinEta",
                    m_minEta,
                    "Minimum Particle Pseudorapidity");
    declareProperty("MaxEta",
                    m_maxEta,
                    "Maximum Particle Pseudorapidity");
    declareProperty("MinPhi",
                    m_minPhi,
                    "Minimum Particle Phi");
    declareProperty("MaxPhi",
                    m_maxPhi,
                    "Maximum Particle Phi");
    declareProperty("MinMom",
                    m_minMom,
                    "Minimum Particle Momentum");
    declareProperty("MaxMom",
                    m_maxMom,
                    "Maximum Particle Momentum");
    declareProperty("ParticlePDG",
                    m_pdgs,
                    "List of accepted particle PDG IDs (any accepted if empty)");
    declareProperty("MaxApplicableRadius",
                    m_maxApplicableRadius,
                    "Only particles with ProductionVertexRadius<MaxApplicableRadius may get filtered out");
}


/** Athena algtool's Hooks */
StatusCode  ISF::GenParticleGenericFilter::initialize()
{
    ATH_MSG_VERBOSE("initialize() ...");
    ATH_MSG_VERBOSE("initialize() successful");
    return StatusCode::SUCCESS;
}


/** Athena algtool's Hooks */
StatusCode  ISF::GenParticleGenericFilter::finalize()
{
    ATH_MSG_VERBOSE("finalize() ...");
    ATH_MSG_VERBOSE("finalize() successful");
    return StatusCode::SUCCESS;
}


/** Returns whether the given particle passes all cuts or not */
#ifdef HEPMC3
bool ISF::GenParticleGenericFilter::pass(HepMC::ConstGenParticlePtr particle) const
{
  bool pass = true;
  HepMC::ConstGenVertexPtr productionVertex = particle?particle->production_vertex():nullptr;
  if (!productionVertex || productionVertex->position().perp()<=m_maxApplicableRadius) {
      pass = check_cuts_passed(particle);
  }
  const auto momentum = particle->momentum();
  ATH_MSG_VERBOSE( "GenParticle '" << particle << "' with "
                   << (productionVertex ? "pos: r=" + std::to_string(productionVertex->position().perp()) : "")
                   << ", mom: eta=" << momentum.eta() << " phi=" << momentum.phi()
                   << " did " << (pass ? "" : "NOT ")
                   << "pass the cuts.");
  return pass;
}
#else
bool ISF::GenParticleGenericFilter::pass(const HepMC::GenParticle& particle) const
{
  bool pass = true;
  HepMC::ConstGenVertexPtr productionVertex = particle.production_vertex();
  if (!productionVertex || productionVertex->position().perp()<=m_maxApplicableRadius) {
      pass = check_cuts_passed(particle);
  }
  const auto momentum = particle.momentum();
  ATH_MSG_VERBOSE( "GenParticle '" << particle << "' with "
                   << (productionVertex ? "pos: r=" + std::to_string(productionVertex->position().perp()) : "")
                   << ", mom: eta=" << momentum.eta() << " phi=" << momentum.phi()
                   << " did " << (pass ? "" : "NOT ")
                   << "pass the cuts.");
  return pass;
}
#endif


/** Check whether the given particle passes all configure cuts or not */
#ifdef HEPMC3
bool ISF::GenParticleGenericFilter::check_cuts_passed(HepMC::ConstGenParticlePtr particle) const {
  const auto momentum = particle?particle->momentum():HepMC::FourVector(0,0,0,0);
  int pdg = particle?particle->pdg_id():0;
#else
bool ISF::GenParticleGenericFilter::check_cuts_passed(const HepMC::GenParticle &particle) const {
  const auto& momentum = particle.momentum();
  int pdg = particle.pdg_id();
#endif
  double mom = std::sqrt(momentum.x()*momentum.x()+momentum.y()*momentum.y()+momentum.z()*momentum.z());
  double eta = momentum.eta();
  double phi = momentum.phi();

  // check the particle pdg code
  if( m_pdgs.size() && std::find(std::begin(m_pdgs), std::end(m_pdgs), pdg) == std::end(m_pdgs) ) {
    return false;
  }

  // check the momentum cuts
  if (mom<m_minMom) {
    return false;
  }
  if (mom>m_maxMom) {
    return false;
  }

  // check the eta cuts
  if (eta<m_minEta || eta>m_maxEta) {
    return false;
  }

  // check the phi cuts
  if (phi<m_minPhi || phi>m_maxPhi) {
    return false;
  }

  // all cuts passed
  return true;
}
