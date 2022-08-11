 /*
 Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
 */

/////////////////////////////////////////////////////////////////
// VHLowTrackJetFilterTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "LongLivedParticleDPDMaker/VHLowTrackJetFilterTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "xAODBTagging/BTaggingContainer.h"
#include "xAODBTagging/BTaggingUtilities.h"


// Constructor
DerivationFramework::VHLowTrackJetFilterTool::VHLowTrackJetFilterTool( const std::string& t,
                                                                      const std::string& n,
                                                                      const IInterface* p ) :
AthAlgTool(t,n,p),
m_nEventsTotal(0),
m_nEventsPass(0),
m_nEventsPassJet(0),
m_nEventsPassElectron(0),
m_nEventsPassMuon(0),
m_nJetsPassAlphaMax(0),
m_nJetsPassCHF(0),
m_debug(true),
m_jetPtCut(0),
m_jetEtaCut(2.1),
m_TrackMinPt(400.0),
m_TrackZ0Max(0.3),
m_TrackD0Max(0.5),
m_AlphaMaxCut(0.05),
m_CHFCut(0.045),
m_nJetsReq(0),
m_electronIDKey("LHMedium"),
m_electronPtCut(0),
m_muonSelectionTool("CP::MuonSelectionTool/MuonSelectionTool"),
m_muonIDKey("Medium"),
m_muonPtCut(0)


{
  declareInterface<DerivationFramework::ISkimmingTool>(this);
  declareProperty("Debug", m_debug);
  
  declareProperty("JetPtCut", m_jetPtCut);
  declareProperty("JetEtaCut", m_jetEtaCut);
  declareProperty("TrackMinPt", m_TrackMinPt);
  declareProperty("TrackZ0Max", m_TrackZ0Max);
  declareProperty("TrackD0Max", m_TrackD0Max);
  declareProperty("JetAlphaMaxCut", m_AlphaMaxCut);
  declareProperty("JetCHFCut", m_CHFCut);
  declareProperty("NJetsRequired", m_nJetsReq);
  
  declareProperty("ElectronIDKey", m_electronIDKey);
  declareProperty("ElectronPtCut", m_electronPtCut);
  
  declareProperty("MuonIDKey", m_muonIDKey);
  declareProperty("MuonPtCut", m_muonPtCut);
  
}

// Athena initialize
StatusCode DerivationFramework::VHLowTrackJetFilterTool::initialize()
{
     ATH_MSG_VERBOSE("initialize() ...");
     ATH_CHECK(m_electronSGKey.initialize());
     ATH_CHECK(m_electronIsoDecorKey.initialize());
     ATH_CHECK(m_muonSGKey.initialize());
     ATH_CHECK(m_muonIsoDecorKey.initialize());
     ATH_CHECK(m_jetSGKey.initialize());
     ATH_CHECK(m_jetBtagDecorKey.initialize());
     ATH_CHECK(m_eventInfoKey.initialize());
     ATH_CHECK(m_primaryVerticesKey.initialize());
     return StatusCode::SUCCESS;
}
// Athena finalize
StatusCode DerivationFramework::VHLowTrackJetFilterTool::finalize()
{
  ATH_MSG_VERBOSE("finalize() ...");
  ATH_MSG_INFO("Processed "<< m_nEventsTotal <<" events, "<< m_nEventsPass<<" events passed filter ");
  ATH_MSG_INFO("Percent of events passed Jet filter "<< 100.0*(float)m_nEventsPassJet/(float)m_nEventsTotal <<" % ");
  ATH_MSG_INFO("Percent of events passing CHF "<< 100.0*(float)m_nJetsPassCHF/(float)m_nEventsTotal <<" % ");
  ATH_MSG_INFO("Percent of events passing alphaMax "<< 100.0*(float)m_nJetsPassAlphaMax/(float)m_nEventsTotal <<" % ");
  ATH_MSG_INFO("Percent of events passed Electron filter "<< 100.0*(float)m_nEventsPassElectron/(float)m_nEventsTotal <<" % ");
  ATH_MSG_INFO("Percent events passed Muon filter "<< 100.0*(float)m_nEventsPassMuon/(float)m_nEventsTotal <<" % ");
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------------------------

// The filter itself
bool DerivationFramework::VHLowTrackJetFilterTool::eventPassesFilter() const
{
  typedef ElementLink<xAOD::TrackParticleContainer> TrackLink;
  typedef std::vector<TrackLink> TrackLinks;
  
  bool passesEl=false, passesMu=false, passesJet=false;
  m_nEventsTotal++;
  
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey); 
  if( !eventInfo.isValid() ) {
    ATH_MSG_ERROR( "Could not retrieve event info" );
  }
  
  //Vertex Container
  SG::ReadHandle<xAOD::VertexContainer> vertices(m_primaryVerticesKey); 
  if( !eventInfo.isValid() ) {
    ATH_MSG_FATAL("No vertex collection with name PrimaryVertices found in StoreGate!");
    return false;
  }
  
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


  //electron portion
  SG::ReadHandle<xAOD::ElectronContainer> electrons(m_electronSGKey); 
  if( !electrons.isValid() ) {
    ATH_MSG_FATAL("No electron collection with name " << m_electronSGKey << " found in StoreGate!");
    return false;
  }
  

  // loop over the electrons in the container
  for (auto electron : *electrons){

    if(electron->pt()<m_electronPtCut) continue;
    if( (std::abs(electron->caloCluster()->etaBE(2))>1.37 && std::abs(electron->caloCluster()->etaBE(2))<1.52)
       || std::abs(electron->caloCluster()->etaBE(2))>2.47 ) continue;

    if(electron->isolation(xAOD::Iso::topoetcone20)/electron->pt()>0.2) continue;
    
    bool passID=false;
    if (!electron->passSelection(passID, m_electronIDKey)) {
      ATH_MSG_WARNING("Cannot find the electron quality flag " << m_electronIDKey);
      continue;
    }
    if(!passID) continue;
    
    for (auto vertex: *vertices){
      // Select good primary vertex
      if (vertex->vertexType() == xAOD::VxType::PriVtx) {
        const xAOD::TrackParticle* tp = electron->trackParticle() ; //your input track particle from the electron
        float sigd0 =std::abs( xAOD::TrackingHelpers::d0significance( tp, eventInfo->beamPosSigmaX(), eventInfo->beamPosSigmaY(), eventInfo->beamPosSigmaXY() ));
        double delta_z0 = tp->z0() + tp->vz() - vertex->z();
        double theta = tp->theta();
        float z0sintheta = std::abs(delta_z0 * sin(theta));

        if (sigd0>5) continue;
        if (z0sintheta>0.5) continue;
        
        passesEl=true;
        break;
      }
    }

    if (passesEl){
      m_nEventsPassElectron++;
      break;
    }
  }
  
    
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  //muon portion

  SG::ReadHandle<xAOD::MuonContainer> muons(m_muonSGKey); 
  if( !muons.isValid() ) {
    ATH_MSG_FATAL("No muon collection with name " << m_muonSGKey << " found in StoreGate!");
    return false;
  }

  
  for(auto muon : *muons){

    if (muon->pt()<m_muonPtCut) continue;
    if (std::abs(muon->eta())>2.5) continue;
    if (!(m_muonSelectionTool->passedMuonCuts(*muon))) continue;
    if (muon->muonType()!=xAOD::Muon::Combined) continue;
    if (muon->isolation(xAOD::Iso::topoetcone20)/muon->pt()>0.3) continue;
    
    for (auto vertex : *vertices) {	// Select good primary vertex
      if (vertex->vertexType() == xAOD::VxType::PriVtx) {
        const xAOD::TrackParticle* tp = muon->primaryTrackParticle() ; //your input track particle from the electron
        
        double d0sig = xAOD::TrackingHelpers::d0significance( tp, eventInfo->beamPosSigmaX(), eventInfo->beamPosSigmaY(), eventInfo->beamPosSigmaXY() );

        if (std::abs(d0sig)>3) continue;
          
	float delta_z0 = tp->z0() + tp->vz() - vertex->z();
        float theta = tp->theta();
        double z0sintheta = delta_z0 * sin(theta);

        if (std::abs(z0sintheta)>0.5) continue;
       
        passesMu = true;
        break;
      }
    }

    if (passesMu) {
      m_nEventsPassMuon++;
      break;
    }
  }
  
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  //Jet portion
  
  int nJetsPassed=0;
  SG::ReadHandle<xAOD::JetContainer> jets(m_jetSGKey); 
  if( !jets.isValid() ) {
    ATH_MSG_WARNING("No Jet container found, will skip this event");
    return false;
  }
  
  std::vector<const xAOD::Jet*> goodJets;
  for (auto jet : *jets) {

    if (jet->pt() < m_jetPtCut) continue;
    if (std::abs(jet->eta()) > m_jetEtaCut) continue;

    TLorentzVector VJet = TLorentzVector(0.0,0.0,0.0,0.0);
    VJet.SetPtEtaPhiE(jet->pt(), jet->eta(), jet->phi(), jet->e());

    float minDeltaR = 100;

    for (auto electron : *electrons ){

      if (electron->pt()<20000) continue;
      if (std::abs(electron->eta())>2.47) continue;

      bool passLoose=false;
      if (!electron->passSelection(passLoose, "LHLoose")){
	ATH_MSG_WARNING("Cannot find the LHLoose electron quality flag");
	continue;
      }
      if (!passLoose) continue;

      TLorentzVector VElec=electron->p4();
      float deltaR = VJet.DeltaR(VElec);

      if (deltaR<minDeltaR) minDeltaR=deltaR;
    }
    
    if (minDeltaR<0.2) continue;

    goodJets.push_back(jet);

  }
  
  
  int jetNo=0;
  for (auto jet : goodJets){
    jetNo++;
    if (jetNo>=3) break;  //Only consider two leading jets
    
    TLorentzVector CHFNum = TLorentzVector(0.0,0.0,0.0,0.0);
    const xAOD::BTagging *bjet(nullptr);
    bjet = xAOD::BTaggingUtilities::getBTagging( *jet );
    if (!bjet){
      ATH_MSG_WARNING("Btag info unavailable");
      continue;
    }
    TrackLinks assocTracks = bjet->auxdata<TrackLinks>("BTagTrackToJetAssociator");
    
    std::vector<const xAOD::TrackParticle*> goodTracks;
    for (auto track : assocTracks) {
      if (!track.isValid()) continue;
      goodTracks.push_back(*track);
    }
    
    float alpha_max=-999;
    for (auto vertex : *vertices) {
      TLorentzVector alphaDen = TLorentzVector(0.0,0.0,0.0,0.0);
      TLorentzVector alphaNum = TLorentzVector(0.0,0.0,0.0,0.0);
      float alpha;
      
      for(auto track : goodTracks) {
        if (track->pt() < m_TrackMinPt) continue;
        
        TLorentzVector VTrack = TLorentzVector(0.0,0.0,0.0,0.0);
        VTrack.SetPtEtaPhiE(track->pt(),track->eta(), track->phi(), track->e());
        alphaDen=alphaDen+VTrack;
        if (std::abs(track->d0()) > m_TrackD0Max) continue;
        
        float z0 = track->z0() + track->vz() - vertex->z();
        float theta = track->theta();
        if (std::abs(z0*sin(theta)) > m_TrackZ0Max) continue;
        
        alphaNum=alphaNum+VTrack;
      }
      if (alphaDen.Pt()==0) alpha=-999;
      else alpha = alphaNum.Pt()/alphaDen.Pt();
      
      if (alpha > alpha_max) alpha_max=alpha;
    }
    
    CHFNum = TLorentzVector(0.0,0.0,0.0,0.0);
    for(auto track : goodTracks) {
      if (track->pt() < m_TrackMinPt) continue;
      if (std::abs(track->d0()) > m_TrackD0Max) continue;
      
      TLorentzVector VTrack = TLorentzVector(0.0,0.0,0.0,0.0);
      VTrack.SetPtEtaPhiE(track->pt(),track->eta(), track->phi(), track->e());
      CHFNum=CHFNum+VTrack;

    }

    float chf = CHFNum.Pt()/jet->pt();
    
    if (alpha_max < m_AlphaMaxCut) m_nJetsPassAlphaMax++;
    if (chf < m_CHFCut) m_nJetsPassCHF++;
    
    if (chf > m_CHFCut && alpha_max > m_AlphaMaxCut) continue;

    nJetsPassed++;

  }
  
  if (nJetsPassed >= m_nJetsReq){
    passesJet=true;
    m_nEventsPassJet++;
  }
  
  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if (passesJet && (passesEl || passesMu)){
    m_nEventsPass++;
    return true;
  }
  else return false;
}

