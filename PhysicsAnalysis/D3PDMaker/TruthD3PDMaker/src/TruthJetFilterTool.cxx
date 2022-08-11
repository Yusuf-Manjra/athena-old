/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TruthD3PDMaker/src/TruthJetFilterTool.cxx
 * @author Renaud Bruneliere <Renaud.Bruneliere@cern.ch>
 * @date Apr, 2010
 * @brief Filter truth particles for building truth jets.
 */


#include "TruthJetFilterTool.h"
#include "GeneratorObjects/McEventCollection.h"
#include "AthenaKernel/errorcheck.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "boost/range/iterator_range_core.hpp"
#include <utility>


using CLHEP::HepLorentzVector;


namespace {


const int PARTONPDGMAX = 43;
const int NPPDGMIN = 1000000;
const int NPPDGMAX = 9999999;
const int PHOTOSMIN = 10000;
const int GEANTMIN = 200000;


} // anonymous namespace


namespace D3PD {


/**
 * @brief Standard Gaudi tool constructor.
 * @param type The name of the tool type.
 * @param name The tool name.
 * @param parent The tool's Gaudi parent.
 */
TruthJetFilterTool::TruthJetFilterTool
  (const std::string& type,
   const std::string& name,
   const IInterface* parent)
    : AthAlgTool (type, name, parent),
      m_resolver (name, evtStore(), m_mcEventsName),
      m_haveSeenAHadron(false),
      m_firstHadronBarcode(0)
{
  declareProperty( "McEvents",       
                   m_mcEventsName = "GEN_AOD",
                   "Name of the input McEventCollection we want to filter" );

  declareProperty( "McEventsOutput", 
                   m_mcEventsOutputName = "GEN_D3PD",
                   "Name of the output McEventCollection which has been filtered" );

  declareProperty( "DoPileup",
                   m_doPileup = false,
                   "If true, include particles from pileup/cavern.");

  declareProperty ("RemoveEmpty",
                   m_removeEmpty = true,
                   "If true, remove empty GenEvent structures.");

  declareProperty ("DoEtIsolations",
                   m_doEtIsolations = false,
                   "Unused, but required by configuration script.");

  declareProperty ("ExcludeWZdecays",
                   m_excludeWZdecays = false,
                   "Do we exclude particles from W/Z decays ?");

  declareProperty ("WritePartons",
                   m_writePartons = true,
                   "Keep partons?");

  declareProperty ("WriteHadrons",
                   m_writeHadrons = true,
                   "Keep hadrons?");

  declareProperty ("WriteGeant",
                   m_writeGeant = false,
                   "Keep geant particles?");

  declareProperty ("ExcludeLeptonsFromTau",
                   m_excludeLeptonsFromTau = false,
                   "Do we exclude leptons from tau decays?");

  declareProperty ("PhotonCone",
                   m_photonCone=-1.,
                   "Exclude FSR photons in cone of X around WZ lepton");
}


/**
 * @brief Standard Gaudi @c queryInterface method.
 */
StatusCode
TruthJetFilterTool::queryInterface( const InterfaceID& riid, void** ppvIf )
{
  if ( riid == ITruthParticleFilterTool::interfaceID() )  {
    *ppvIf = static_cast<ITruthParticleFilterTool*> (this);
    addRef();
    return StatusCode::SUCCESS;
  }

  return AlgTool::queryInterface( riid, ppvIf );
}


/**
 * @brief Standard Gaudi initialize method.
 */
StatusCode TruthJetFilterTool::initialize()
{
  CHECK( m_resolver.initialize<McEventCollection>() );
  return AthAlgTool::initialize();
}


/**
 * @brief Standard Gaudi finalize method.
 */
StatusCode TruthJetFilterTool::finalize()
{
  return AthAlgTool::finalize();
}


/**
 * @brief Standard Gaudi execute method.
 */
StatusCode TruthJetFilterTool::execute()
{
  m_haveSeenAHadron = false;
  m_firstHadronBarcode = 0;
	m_WZleptons.clear();

  // Fetch input collection.
  const McEventCollection* mc_in = nullptr;
  CHECK( evtStore()->retrieve (mc_in, m_resolver.key()) );

  // Create output collection.
  McEventCollection* mc_out = new McEventCollection;
  CHECK( evtStore()->record (mc_out, m_mcEventsOutputName) );
  CHECK( evtStore()->setConst (mc_out) );

  // Copy and filter.
  CHECK( buildMcAod (mc_in, mc_out) );
  return StatusCode::SUCCESS;
}


/**
 *  This method will check the validity of the input McEventCollection 
 *  and build a filtered one from the strategy implemented by the 
 *  concrete tool.
 *  It is not const to allow derived tools to build statistics during
 *  the filtering process.
 */
StatusCode
TruthJetFilterTool::buildMcAod (const McEventCollection* mc_in, McEventCollection* mc_out)
{
  // Loop over GenEvent's.
  mc_out->reserve (mc_in->size());
  for (const HepMC::GenEvent* ev_in : *mc_in) {
    if (!ev_in) continue;

    // Copy the GenEvent.
    HepMC::GenEvent* ev_out = HepMC::copyemptyGenEvent (ev_in);

    // Copy and filter the contents.
    CHECK( filterEvent (ev_in, ev_out) );

    // Maybe throw out empty GenEvent's.
#ifdef HEPMC3
    if (m_removeEmpty && ev_out->particles().empty())
      delete ev_out;
    else
      mc_out->push_back (ev_out);
#else
    if (m_removeEmpty && ev_out->particles_empty())
      delete ev_out;
    else
      mc_out->push_back (ev_out);
#endif

    // If we don't want pileup, only do the first GenEvent.
    if (!m_doPileup)
      break;
  }
  return StatusCode::SUCCESS;
}


/**
 * @brief Filter a single @c GenEvent.
 */
StatusCode
TruthJetFilterTool::filterEvent (const HepMC::GenEvent* ev_in,
				 HepMC::GenEvent* ev_out)
{
  // Loop over particles.
  // (range-based for doesn't work here because particle_const_iterator
  // isn't consistent in the use of const...)
  for (auto  ip: *ev_in) {
    // Copy the particle if we want to keep it.
    if (acceptParticle (ip)) CHECK( addParticle (ip, ev_out) );
  }
  return StatusCode::SUCCESS;
}


/**
 * @brief Add a @c GenParticle (and its production vertex) to a @c GenEvent.
 */
StatusCode
TruthJetFilterTool::addParticle (HepMC::ConstGenParticlePtr  p, HepMC::GenEvent* ev)
{
  // Add parent vertex if it exists.  Otherwise, add decay vertex.
  if (p->production_vertex())
    CHECK( addVertex (p->production_vertex(), ev) );
  else if (p->end_vertex())
    CHECK( addVertex (p->end_vertex(), ev) );
  else {
    REPORT_MESSAGE (MSG::ERROR) << "Encountered GenParticle with no vertices!";
    return StatusCode::FAILURE;
  }

#ifdef HEPMC3
  // Find the particle in the event.
  // If it doesn't exist yet, copy it.
  HepMC::GenParticlePtr pnew = HepMC::barcode_to_particle (ev,HepMC::barcode(p));
  if (!pnew) pnew = std::make_shared<HepMC3::GenParticle> (*p);
  // Add ourself to our vertices.
  if (p->production_vertex()) {
    HepMC::GenVertexPtr v = HepMC::barcode_to_vertex (ev, HepMC::barcode(p->production_vertex()));
    if (v) v->add_particle_out (pnew);
  }
  if (p->end_vertex()) {
    HepMC::GenVertexPtr v = HepMC::barcode_to_vertex (ev, HepMC::barcode(p->end_vertex()));
    if (v) v->add_particle_in (pnew);
  }
#else
  // Find the particle in the event.
  // If it doesn't exist yet, copy it.
  HepMC::GenParticle* pnew = ev->barcode_to_particle (p->barcode());
  if (!pnew)
    pnew = new HepMC::GenParticle (*p);

  // Add ourself to our vertices.
  if (p->production_vertex()) {
    HepMC::GenVertex* v =
      ev->barcode_to_vertex (p->production_vertex()->barcode());
    if (v)
      v->add_particle_out (pnew);
  }

  if (p->end_vertex()) {
    HepMC::GenVertex* v =
      ev->barcode_to_vertex (p->end_vertex()->barcode());
    if (v)
      v->add_particle_in (pnew);
  }
#endif

  return StatusCode::SUCCESS;
}


/**
 * @brief Add a @c GenVertex to a @c GenEvent.
 */
StatusCode
TruthJetFilterTool::addVertex (HepMC::ConstGenVertexPtr v,HepMC::GenEvent* ev)
{
#ifdef HEPMC3
  // See if this vertex has already been copied.
  HepMC::GenVertexPtr vnew = HepMC::barcode_to_vertex (ev,HepMC::barcode(v));
  if (!vnew) {
    // No ... make a new one.
    vnew = HepMC::newGenVertexPtr();
    ev->add_vertex (vnew);
    vnew->set_position (v->position());
    vnew->set_status (v->status());
    HepMC::suggest_barcode (vnew,HepMC::barcode(v));
    //AV: here should be code to copy the weights, but these are never used. Skip. Please don't remove this comment. vnew->weights() = v->weights();
    // Fill in the existing relations of the new vertex.
    for (auto  p : v->particles_in())
    {
      HepMC::GenParticlePtr pnew = HepMC::barcode_to_particle (ev,HepMC::barcode(p));
      if (pnew) vnew->add_particle_in (pnew);
    }

    for (auto p : v->particles_out())
    {
      HepMC::GenParticlePtr pnew = HepMC::barcode_to_particle (ev,HepMC::barcode(p));
      if (pnew) vnew->add_particle_out (pnew);
    }
  }
#else
  // See if this vertex has already been copied.
  HepMC::GenVertex* vnew = ev->barcode_to_vertex (v->barcode());
  if (!vnew) {
    // No ... make a new one.
    vnew = new HepMC::GenVertex;
    vnew->set_position (v->position());
    vnew->set_id (v->id());
    vnew->suggest_barcode (v->barcode());
    vnew->weights() = v->weights();
    ev->add_vertex (vnew);

    // Fill in the existing relations of the new vertex.
    for (const HepMC::GenParticle* p :
           boost::make_iterator_range (v->particles_in_const_begin(),
                                       v->particles_in_const_end()))
    {
      HepMC::GenParticle* pnew = ev->barcode_to_particle (p->barcode());
      if (pnew)
        vnew->add_particle_in (pnew);
    }

    for (const HepMC::GenParticle* p :
           boost::make_iterator_range (v->particles_out_const_begin(),
                                       v->particles_out_const_end()))
    {
      HepMC::GenParticle* pnew = ev->barcode_to_particle (p->barcode());
      if (pnew)
        vnew->add_particle_out (pnew);
    }
  }
#endif

  return StatusCode::SUCCESS;
}


/**
 * @brief Test to find leptons from tau decays.
 */
bool TruthJetFilterTool::isLeptonFromTau(HepMC::ConstGenParticlePtr part) const{

  int pdg = part->pdg_id();

  if(std::abs(pdg) != 11 &&
     std::abs(pdg) != 12 &&
     std::abs(pdg) != 13 &&
     std::abs(pdg) != 14 &&
     std::abs(pdg) != 15 &&
     std::abs(pdg) != 16) return false; // all leptons including tau.

  HepMC::ConstGenVertexPtr prod = part->production_vertex();
  if(!prod) return false; // no parent.
#ifdef HEPMC3
  // Loop over the parents of this particle.
  for(auto itrParent: prod->particles_in()){
    int parentId = itrParent->pdg_id();
    if(std::abs(parentId) == 15) {
      ATH_MSG_DEBUG("Particle with pdgId = " << pdg << ", matched to tau");
      return true; // Has tau parent
    }
    if(parentId == pdg) { // Same particle just a different MC status
      // Go up the generator record until a tau is found or not. 
      // Note that this requires a connected *lepton* chain, while calling
      //  isFromTau would allow leptons from hadrons from taus
      if(isLeptonFromTau(itrParent)) {
        return true;
      }
    }
  }
#else
  // Loop over the parents of this particle.
  auto itrParent = prod->particles_in_const_begin();
  auto endParent = prod->particles_in_const_end();
  for(;itrParent!=endParent; ++itrParent){
    int parentId = (*itrParent)->pdg_id();
    if(std::abs(parentId) == 15) {
      ATH_MSG_DEBUG("Particle with pdgId = " << pdg << ", matched to tau");
      return true; // Has tau parent
    }

    if(parentId == pdg) { // Same particle just a different MC status
      // Go up the generator record until a tau is found or not. 
      // Note that this requires a connected *lepton* chain, while calling
      //  isFromTau would allow leptons from hadrons from taus
      if(isLeptonFromTau(*itrParent)) {
        return true;
      }
    }
  }
#endif

  return false;
}


/**
 * @brief Test to see if we want to keep a particle.
 */
bool
TruthJetFilterTool::acceptParticle (HepMC::ConstGenParticlePtr p)
{
  bool ok = false;
  int pdg_id = std::abs (p->pdg_id());
  int status = p->status();
  int barcode = HepMC::barcode(p);

	if (barcode > GEANTMIN && !m_writeGeant)
		return false;

	if (m_excludeWZdecays) {
		if (pdg_id == 23 || pdg_id == 24) return false;
		HepMC::ConstGenVertexPtr vprod = p->production_vertex();
		HepMC::ConstGenVertexPtr oldVprod = vprod;
		
		if (vprod && HepMC::particles_in_size(vprod) > 0) {
			int mom_pdg_id = pdg_id;
			// Ascend decay chain looking for when actual decay occurs (not jsut evolution of particle)
			while (pdg_id == mom_pdg_id) {
#ifdef HEPMC3
				HepMC::ConstGenParticlePtr mother = vprod->particles_in().at(0);
#else
				HepMC::ConstGenParticlePtr mother = *(vprod->particles_in_const_begin());
#endif
				if (mother) {
					mom_pdg_id = std::abs(mother->pdg_id());
				} else break;
				if (pdg_id != mom_pdg_id) break;

				// Protect against strange infinite reference in sherpa samples
				if(oldVprod==mother->production_vertex()) break;
				oldVprod = vprod;

				vprod = mother->production_vertex();
				if (!vprod ||  HepMC::particles_in_size(vprod) == 0) break;
			} // End while loop

			// The following vertex-based identification of W/Z's is needed for SHERPA
			// samples where the W/Z particle is not explicitly in the particle record.
			// At this point if we have a valid vertex, it should be a true decay vertex.
			// If it is a W or Z then two of those decay products should be lepton/neutrino
			int nDecay=0;
			// Prompt W/Z's should come from a vertex with more than one incoming particle
			// This suppresses fave Z's from conversions
			if (vprod &&  HepMC::particles_in_size(vprod) >1)
			{
				for(auto daughter: *vprod)
				{
					if((std::abs(daughter->pdg_id())>10 && std::abs(daughter->pdg_id())<17) ) nDecay++;
				}
			}
			bool isWZ = (nDecay==2);
			if (mom_pdg_id == 23 || mom_pdg_id == 24 || 
                mom_pdg_id==1000011 || mom_pdg_id==1000013 || mom_pdg_id==1000015 || // LH sleptons
                mom_pdg_id==2000011 || mom_pdg_id==2000013 || mom_pdg_id==2000015 || // RH sleptons
                mom_pdg_id==1000012 || mom_pdg_id==1000014 || mom_pdg_id==1000016 || // Sneutrinos
                mom_pdg_id==1000023 || mom_pdg_id==1000024 || mom_pdg_id==1000025 || mom_pdg_id==1000035 || // Gauginos
                isWZ) { // paricle decends from W or Z
				//  Save lepton reference for comparison to FSR photons (only for muons and electrons)
				if(p->status()==1 && (std::abs(p->pdg_id())==11 || std::abs(p->pdg_id())==13) ) m_WZleptons.push_back(p);

				// Only exclude photons within deltaR of leptons (if m_photonCone<0, exclude all photons)
				if(std::abs (p->pdg_id()) == 22 && m_photonCone>0)
				{
					for(auto lep: m_WZleptons) {
						double deltaR = HepLorentzVector(p->momentum().px(),p->momentum().py(),p->momentum().pz(),p->momentum().e())
							.deltaR(
									HepLorentzVector(lep->momentum().px(),lep->momentum().py(),lep->momentum().pz(),lep->momentum().e()));
						// if photon within deltaR of lepton, remove along with lepton
						if( deltaR < m_photonCone ) return false;
					}
				}
				else // not a photon so exclude
					return false;
			}
		}
	}

	// is this a lepton from a tau decay?
	if (m_excludeLeptonsFromTau && isLeptonFromTau(p) ) {
		return false;
	}

	// are we at parton/hadron level?
	if ( status!=3 && pdg_id > PARTONPDGMAX && !m_haveSeenAHadron ) {
		m_haveSeenAHadron = true;
		m_firstHadronBarcode = barcode;
	}

	// OK if we select partons and are at beginning of event record
	if( m_writePartons /*&& !m_haveSeenAHadron */ &&
			(pdg_id <= PARTONPDGMAX || (pdg_id >= NPPDGMIN && pdg_id <= NPPDGMAX) ))
		ok = true;
	
	//  OK if we should select hadrons and are in hadron range 
  if( m_writeHadrons && m_haveSeenAHadron && barcode < PHOTOSMIN )
    ok = true;
 
  // PHOTOS range: check whether photons come from parton range or 
  // hadron range
  int motherBarcode = 999999999;
  if( barcode > PHOTOSMIN && barcode < GEANTMIN && p->production_vertex() ) {
    HepMC::ConstGenVertexPtr vprod = p->production_vertex();
    if ( HepMC::particles_in_size(vprod) > 0) {
#ifdef HEPMC3
      HepMC::ConstGenParticlePtr mother = vprod->particles_in().at(0);
#else
      const HepMC::GenParticle* mother = *vprod->particles_in_const_begin();
#endif
      if (mother)
        motherBarcode = HepMC::barcode(mother);
    }

    if( m_writePartons && motherBarcode < m_firstHadronBarcode )
      ok = true;
    if( m_writeHadrons && motherBarcode >= m_firstHadronBarcode )
      ok = true;
  }

  // OK if we should select G4 particles and are in G4 range
  if( m_writeGeant && barcode > GEANTMIN )
    ok = true;

  return ok;
}


} // namespace D3PD
