/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "PFOHistUtils/FlowElement_LinkerPlots.h"
#include <iostream>
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "AthLinks/ElementLink.h"

namespace PFO {

  FlowElement_LinkerPlots::FlowElement_LinkerPlots(PlotBase* pParent, std::string sDir, std::string sFEContainerName, bool doNeutralFE): 
    PlotBase(pParent, sDir),
    m_sFEContainerName(sFEContainerName),
    m_doNeutralFE(doNeutralFE)
    {
      m_CFE_tau_dR=nullptr;
      m_CFE_tau_NMatchedTau=nullptr;
      m_CFE_tau_tauNMatchedFE=nullptr;

      m_NFE_tau_dR=nullptr;
      m_NFE_tau_NMatchedTau=nullptr;
      m_NFE_tau_tauNMatchedFE=nullptr;


      m_CFE_muon_dR=nullptr;
      m_CFE_muon_NMatchedMuon=nullptr;
      m_CFE_muon_muonNMatchedFE=nullptr;
      m_CFE_muon_largeDR_debug_author=nullptr;
      m_CFE_muon_largeDR_debug_type=nullptr;

      m_NFE_muon_dR=nullptr;
      m_NFE_muon_NMatchedMuon=nullptr;
      m_NFE_muon_muonNMatchedFE=nullptr;
      
      
      m_CFE_photon_dR=nullptr;
      m_CFE_photon_NMatchedPhoton=nullptr;
      m_CFE_photon_photonNMatchedFE=nullptr;

      m_NFE_photon_dR=nullptr;
      m_NFE_photon_NMatchedPhoton=nullptr;
      m_NFE_photon_photonNMatchedFE=nullptr;


      m_CFE_electron_dR=nullptr;
      m_CFE_electron_NMatchedElectron=nullptr;
      m_CFE_electron_electronNMatchedFE=nullptr;

      m_NFE_electron_dR=nullptr;
      m_NFE_electron_NMatchedElectron=nullptr;
      m_NFE_electron_electronNMatchedFE=nullptr;
      

    }

  void FlowElement_LinkerPlots::initializePlots(){
    // init only the histograms we're going to use (m_doNeutralFE is the switch)
    //tau
    if(m_sFEContainerName==""){ // skip init if container not used.
      return;
    }
    if(!m_doNeutralFE){
      m_CFE_tau_dR=Book1D("_CFE_tau_dR",m_sFEContainerName+"_CFE_tau_dR",40,0,5); 
      m_CFE_tau_NMatchedTau=Book1D("_CFE_tau_NMatchedTau",m_sFEContainerName+"_CFE_tau_NMatchedTau",20,0,20);
      m_CFE_tau_tauNMatchedFE=Book1D("_CFE_tau_tauNMatchedFE",m_sFEContainerName+"_CFE_tau_tauNMatchedFE",20,0,20);
    }
    else{
      m_NFE_tau_dR=Book1D("_NFE_tau_dR",m_sFEContainerName+"_NFE_tau_dR",40,0,5); 
      m_NFE_tau_NMatchedTau=Book1D("_NFE_tau_NMatchedTau",m_sFEContainerName+"_NFE_tau_NMatchedTau",20,0,20);
      m_NFE_tau_tauNMatchedFE=Book1D("_NFE_tau_tauNMatchedFE",m_sFEContainerName+"_NFE_tau_tauNMatchedFE",20,0,20);
    }
    //electron
    if(!m_doNeutralFE){
      m_CFE_electron_dR=Book1D("_CFE_electron_dR",m_sFEContainerName+"_CFE_electron_dR",40,0,5); 
      m_CFE_electron_NMatchedElectron=Book1D("_CFE_electron_NMatchedElectron",m_sFEContainerName+"_CFE_electron_NMatchedElectron",20,0,20);
      m_CFE_electron_electronNMatchedFE=Book1D("_CFE_electron_electronNMatchedFE",m_sFEContainerName+"_CFE_electron_electronNMatchedFE",20,0,20);
    }
    else{
      m_NFE_electron_dR=Book1D("_NFE_electron_dR",m_sFEContainerName+"_NFE_electron_dR",40,0,5); 
      m_NFE_electron_NMatchedElectron=Book1D("_NFE_electron_NMatchedElectron",m_sFEContainerName+"_NFE_electron_NMatchedElectron",20,0,20);
      m_NFE_electron_electronNMatchedFE=Book1D("_NFE_electron_electronNMatchedFE",m_sFEContainerName+"_NFE_electron_electronNMatchedFE",20,0,20);
    }
    //muon
    if(!m_doNeutralFE){
      m_CFE_muon_dR=Book1D("_CFE_muon_dR",m_sFEContainerName+"_CFE_muon_dR",40,0,5); 
      m_CFE_muon_NMatchedMuon=Book1D("_CFE_muon_NMatchedMuon",m_sFEContainerName+"_CFE_muon_NMatchedMuon",20,0,20);
      m_CFE_muon_muonNMatchedFE=Book1D("_CFE_muon_muonNMatchedFE",m_sFEContainerName+"_CFE_muon_muonNMatchedFE",20,0,20);
      m_CFE_muon_largeDR_debug_author=Book1D("_CFE_muon_largeDR_debug_author",m_sFEContainerName+"_CFE_muon_largeDR_debug_author",20,0,20);
      m_CFE_muon_largeDR_debug_type=Book1D("_CFE_muon_largeDR_debug_type",m_sFEContainerName+"_CFE_muon_largeDR_debug_type",20,0,20);
    }
    else{
      m_NFE_muon_dR=Book1D("_NFE_muon_dR",m_sFEContainerName+"_NFE_muon_dR",40,0,5); 
      m_NFE_muon_NMatchedMuon=Book1D("_NFE_muon_NMatchedMuon",m_sFEContainerName+"_NFE_muon_NMatchedMuon",20,0,20);
      m_NFE_muon_muonNMatchedFE=Book1D("_NFE_muon_muonNMatchedFE",m_sFEContainerName+"_NFE_muon_muonNMatchedFE",20,0,20);
    }
    //photon
    if(!m_doNeutralFE){
      m_CFE_photon_dR=Book1D("_CFE_photon_dR",m_sFEContainerName+"_CFE_photon_dR",40,0,5); 
      m_CFE_photon_NMatchedPhoton=Book1D("_CFE_photon_NMatchedPhoton",m_sFEContainerName+"_CFE_photon_NMatchedPhoton",20,0,20);
      m_CFE_photon_photonNMatchedFE=Book1D("_CFE_photon_photonNMatchedFE",m_sFEContainerName+"_CFE_photon_photonNMatchedFE",20,0,20);
    }
    else{
      m_NFE_photon_dR=Book1D("_NFE_photon_dR",m_sFEContainerName+"_NFE_photon_dR",40,0,5); 
      m_NFE_photon_NMatchedPhoton=Book1D("_NFE_photon_NMatchedPhoton",m_sFEContainerName+"_NFE_photon_NMatchedPhoton",20,0,20);
      m_NFE_photon_photonNMatchedFE=Book1D("_NFE_photon_photonNMatchedFE",m_sFEContainerName+"_NFE_photon_photonNMatchedFE",20,0,20);
    }
  }
  void FlowElement_LinkerPlots::fill(const xAOD::FlowElement& FE, const xAOD::EventInfo& eventInfo){
    // methods work for both CFE and NFE since the auxvars are named the same
    SG::AuxElement::ConstAccessor< std::vector < ElementLink< xAOD::MuonContainer > > > acc_muon_FE_Link("FE_MuonLinks");
    SG::AuxElement::ConstAccessor< std::vector < ElementLink< xAOD::PhotonContainer > > > acc_photon_FE_Link("FE_PhotonLinks");
    SG::AuxElement::ConstAccessor< std::vector < ElementLink< xAOD::ElectronContainer > > > acc_electron_FE_Link("FE_ElectronLinks");
    SG::AuxElement::ConstAccessor< std::vector < ElementLink< xAOD::TauJetContainer > > > acc_tau_FE_Link("FE_TauLinks");
    
    TLorentzVector FE_fourvec=FE.p4();
    // Muon block
    if(acc_muon_FE_Link.isAvailable(FE)){
      std::vector< ElementLink < xAOD::MuonContainer > > MuonLinks=acc_muon_FE_Link(FE);
      int nMuons_per_FE=MuonLinks.size();
      if(nMuons_per_FE>0){ // skip cases w/o match
	    if(m_doNeutralFE)
	      m_NFE_muon_NMatchedMuon->Fill(nMuons_per_FE,eventInfo.beamSpotWeight());
	    else
	      m_CFE_muon_NMatchedMuon->Fill(nMuons_per_FE,eventInfo.beamSpotWeight());	
      }
            
      for (ElementLink<xAOD::MuonContainer> MuonLink: MuonLinks){

	      //get Muon from link by de-referencing it
        if (!MuonLink.isValid()){
          std::cerr << "FlowElement_LinkerPlots encountered an invalid muon element link. Skipping. "<<std::endl;
          continue; 
        }
	      const xAOD::Muon* muon = *MuonLink;
	      TLorentzVector muon_fourvec=muon->p4();
	      double deltaR=muon_fourvec.DeltaR(FE_fourvec);
	      if(muon->muonType()==4) // skip forward muons, as have tracklets which mis-link to FE.
	      continue;
	      if(m_doNeutralFE)
	        m_NFE_muon_dR->Fill(deltaR,eventInfo.beamSpotWeight());
	      else{
	        m_CFE_muon_dR->Fill(deltaR,eventInfo.beamSpotWeight());
  	    if(deltaR>1){// should never happen, but catch for extreme cases
		      int auth=muon->author();
		      int type=muon->muonType();
		      m_CFE_muon_largeDR_debug_author->Fill(auth,eventInfo.beamSpotWeight());
		      m_CFE_muon_largeDR_debug_type->Fill(type,eventInfo.beamSpotWeight());
	      } 
	    }// end of CFE fill block for muon_dR code	
      }
    }// end of muon acc block
     
    // Electron block
    if(acc_electron_FE_Link.isAvailable(FE)){
      std::vector< ElementLink < xAOD::ElectronContainer > > ElectronLinks=acc_electron_FE_Link(FE);
      int nElectrons_per_FE=ElectronLinks.size();
      if(nElectrons_per_FE>0){ // skip cases w/o match
	      if(m_doNeutralFE)
	        m_NFE_electron_NMatchedElectron->Fill(nElectrons_per_FE,eventInfo.beamSpotWeight());
	      else
	        m_CFE_electron_NMatchedElectron->Fill(nElectrons_per_FE,eventInfo.beamSpotWeight());	
      }

      for (ElementLink<xAOD::ElectronContainer> ElectronLink: ElectronLinks){
	      //get Electron from link by de-referencing it
        if (!ElectronLink.isValid()){
          std::cerr << "FlowElement_LinkerPlots encountered an invalid electron element link. Skipping. "<<std::endl;
          continue; 
        }
	      const xAOD::Electron* electron = *ElectronLink;
	      TLorentzVector electron_fourvec=electron->p4();
	      double deltaR=electron_fourvec.DeltaR(FE_fourvec);
	      if(m_doNeutralFE)
	        m_NFE_electron_dR->Fill(deltaR,eventInfo.beamSpotWeight());
	      else
	        m_CFE_electron_dR->Fill(deltaR,eventInfo.beamSpotWeight());	
      }
    }// end of electron acc block
    
    // Photon block
    if(acc_photon_FE_Link.isAvailable(FE)){
      std::vector< ElementLink < xAOD::PhotonContainer > > PhotonLinks=acc_photon_FE_Link(FE);
      int nPhotons_per_FE=PhotonLinks.size();
      if(nPhotons_per_FE>0){ // skip cases w/o match
  	    if(m_doNeutralFE)
	        m_NFE_photon_NMatchedPhoton->Fill(nPhotons_per_FE,eventInfo.beamSpotWeight());
	      else
	        m_CFE_photon_NMatchedPhoton->Fill(nPhotons_per_FE,eventInfo.beamSpotWeight());	
      }

      for (ElementLink<xAOD::PhotonContainer> PhotonLink: PhotonLinks){
  	  //get Photon from link by de-referencing it
      if (!PhotonLink.isValid()){
        std::cerr << "FlowElement_LinkerPlots encountered an invalid photon element link. Skipping. "<<std::endl;
        continue; 
      }
	    const xAOD::Photon* photon = *PhotonLink;
	    TLorentzVector photon_fourvec=photon->p4();
	    double deltaR=photon_fourvec.DeltaR(FE_fourvec);
	    if(m_doNeutralFE)
	      m_NFE_photon_dR->Fill(deltaR,eventInfo.beamSpotWeight());
	    else
	      m_CFE_photon_dR->Fill(deltaR,eventInfo.beamSpotWeight());	
      }
    }// end of photon acc block

    // Taujet block
    if(acc_tau_FE_Link.isAvailable(FE)){
      std::vector< ElementLink < xAOD::TauJetContainer > > TaujetLinks=acc_tau_FE_Link(FE);
      int nTaujets_per_FE=TaujetLinks.size();
      if(nTaujets_per_FE>0){ // skip cases w/o match
  	    if(m_doNeutralFE)
	        m_NFE_tau_NMatchedTau->Fill(nTaujets_per_FE,eventInfo.beamSpotWeight());
	      else
	        m_CFE_tau_NMatchedTau->Fill(nTaujets_per_FE,eventInfo.beamSpotWeight());	
      }
      
      TLorentzVector FE_fourvec=FE.p4();
      for (ElementLink<xAOD::TauJetContainer> TaujetLink: TaujetLinks){
	      //get Taujet from link by de-referencing it
        if (!TaujetLink.isValid()){
          std::cerr << "FlowElement_LinkerPlots encountered an invalid tau jet element link. Skipping. "<<std::endl;
          continue; 
        }
	      const xAOD::TauJet* taujet = *TaujetLink;
	      TLorentzVector taujet_fourvec=taujet->p4();
	      double deltaR=taujet_fourvec.DeltaR(FE_fourvec);
	      if(m_doNeutralFE)
	        m_NFE_tau_dR->Fill(deltaR,eventInfo.beamSpotWeight());
	      else
	        m_CFE_tau_dR->Fill(deltaR,eventInfo.beamSpotWeight());	
      }
    }// end of taujet acc block    
  } // end of fill statement
} // end of PFO namespace
