/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONEF_TRIGMUONEFTRACKISOLATIONALG_H
#define TRIGMUONEF_TRIGMUONEFTRACKISOLATIONALG_H 1

// Base class
#include "AthenaBaseComps/AthReentrantAlgorithm.h" 
#include "StoreGate/ReadHandleKey.h" 
#include "StoreGate/WriteHandleKey.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "AthenaMonitoringKernel/Monitored.h"

// Retrieve containers
#include "Particle/TrackParticleContainer.h" 
#include "xAODMuon/MuonContainer.h"

#include "TrigMuonToolInterfaces/IMuonEFTrackIsolationTool.h"


//#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

class TrigMuonEFTrackIsolationAlg : public AthReentrantAlgorithm
{
  public :

    /** Constructor **/
    TrigMuonEFTrackIsolationAlg( const std::string& name, ISvcLocator* pSvcLocator );
    /** Destructor **/
    ~TrigMuonEFTrackIsolationAlg(){};
  
    /** initialize. Called by the Steering. */
    virtual StatusCode initialize() override;
  
    /** execute execute the combined muon FEX. */
    virtual StatusCode execute(const EventContext& ctx) const override;


  private :

    /// Tool handle for isolation tool
    ToolHandle<IMuonEFTrackIsolationTool> m_onlineEfIsoTool {
	this, "OnlineIsolationTool", "TrigMuonEFTrackIsolationTool/TrigMuonEFTrackIsolationTool", "Select online muon isolation tool you want to use"};

    // ID Tracks and EF Muons
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticleKey {
  	this, "IdTrackParticles", "InDetTrigTrackingxAODCnv_Muon_FTF", "Name of ID Track Particle container" };
  
    SG::ReadHandleKey<xAOD::MuonContainer> m_efMuonContainerKey {
  	this, "MuonEFContainer", "Muons", "Name of EF Muons container" };

    SG::WriteHandleKey<xAOD::MuonContainer> m_muonContainerKey {
	this, "MuonContName", "MuonEFInfo", "Name of output objects for EF" };

    SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonIso20Key {
	this, "ptcone02Name", "Muons.ptcone02", "Isolation for ptcone 0.2" };

    SG::WriteDecorHandleKey<xAOD::MuonContainer> m_muonIso30Key {
	this, "ptcone03Name", "Muons.ptcone03", "Isolation for ptcone 0.3" };


    // Require that EF muons are combined
    Gaudi::Property<bool> m_requireCombined { this, "requireCombinedMuon", true, "Require that EF Muons are combined"};
  
    // Use offline isolation variables
    Gaudi::Property<bool> m_useVarIso { this, "useVarIso", true, "Use offline isolation variables"};
  
    // cone sizes to calculate the isolation
    std::vector<double> m_coneSizes; 

    // Monitoring tool
    ToolHandle< GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };

};


#endif
