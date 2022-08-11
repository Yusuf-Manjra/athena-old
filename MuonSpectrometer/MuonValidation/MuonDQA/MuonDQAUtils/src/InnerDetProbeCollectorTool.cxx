/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonDQAUtils/InnerDetProbeCollectorTool.h"

//================ Constructor =================================================
namespace Muon {
  InnerDetProbeCollectorTool::InnerDetProbeCollectorTool(const std::string& t,
							 const std::string& n,
							 const IInterface*  p )
    :
    AthAlgTool(t,n,p),
    m_InsituPerformanceTools("InsituTrackPerformanceTools/TestTool")
  {
    declareInterface<IProbeCollectorTool>(this);

    declareProperty("InnerTrackContainerName",		m_InnerTrackContainerName = "TrackParticleCandidate");
    declareProperty("MSTrackContainerName",			m_MSTrackContainerName = "MuonboyTrackParticles" );
    declareProperty("CombinedMuonTracksContainerName",	m_CombinedMuonTracksContainerName = "StacoMuonCollection");
    declareProperty("RequireTrigger",			m_RequireTrigger = false);
    declareProperty("InsituPerformanceTools",		m_InsituPerformanceTools);
    declareProperty("MuonPtCut", m_muonPtCut=20000.);
  }

  //================ Initialisation =================================================

  StatusCode InnerDetProbeCollectorTool::initialize()
  {
    ATH_CHECK(AlgTool::initialize());

    /// Getting InsituPerformanceTools
    ATH_CHECK(m_InsituPerformanceTools->initialize());

    ATH_MSG_INFO("initialize() successful");
    return StatusCode::SUCCESS;
  }

  //============================================================================================

  StatusCode InnerDetProbeCollectorTool::createProbeCollection()
  {
    ATH_MSG_DEBUG("createProbeCollection() for Inner Detector");
	  /// Record the container of Probe Muons in evtStore
    m_IDProbeTrackContainer = new Rec::TrackParticleContainer();
    ATH_CHECK(evtStore()->record(m_IDProbeTrackContainer,"InnerDetectorProbeTracks"));
    ATH_MSG_DEBUG("InnerDetectorProbeTracks Container recorded in evtStore.");
	
    /// Retrieve Muon Tracks
    const Rec::TrackParticleContainer* trackTES=nullptr;
    ATH_CHECK(evtStore()->retrieve( trackTES, m_MSTrackContainerName));
    if (!trackTES) {
	ATH_MSG_WARNING("No " << m_MSTrackContainerName << " container found in TDS"); 
	return StatusCode::FAILURE;
      }  
    ATH_MSG_DEBUG("MuonTrack Container successfully retrieved");

    /// Retrieve Combined Tracks
    const Analysis::MuonContainer* muonTDS=nullptr;
    ATH_CHECK(evtStore()->retrieve( muonTDS, m_CombinedMuonTracksContainerName));
    if (!muonTDS ) {
	ATH_MSG_WARNING("No AOD "<<m_CombinedMuonTracksContainerName<<" container of muons found in TDS"); 
	return StatusCode::FAILURE;
      }
    ATH_MSG_DEBUG("MuonContainer successfully retrieved");
	
    /// Loop over Combined Muon Tracks
    Analysis::MuonContainer::const_iterator muonItr  = muonTDS->begin();
    Analysis::MuonContainer::const_iterator muonItrE = muonTDS->end();
    for (; muonItr != muonItrE; ++muonItr)
      {
	if (((*muonItr)->pt()>m_muonPtCut) && ((*muonItr)->isCombinedMuon()))
	  {
	    const Rec::TrackParticle *muonTrack	= (*muonItr)->track();
	    if ((m_RequireTrigger==false) || (m_InsituPerformanceTools->isTriggeredMuon(muonTrack)==true))
	      {
		Rec::TrackParticleContainer::const_iterator MSTrackItr  = trackTES->begin();
		Rec::TrackParticleContainer::const_iterator MSTrackItrE = trackTES->end();
		for (; MSTrackItr != MSTrackItrE; ++MSTrackItr) 
		  {
		    const Rec::TrackParticle *MSTrack	= (*MSTrackItr);
		    if (MSTrack->pt()>m_muonPtCut)
		      {
						
			if (m_InsituPerformanceTools->isZBosonCandidate(MSTrack, muonTrack)==true)
			  {
			    m_IDProbeTrackContainer->push_back( new Rec::TrackParticle(**MSTrackItr) );
			  }
		      }
		  }
	      }
	  }
      }
    ATH_CHECK(evtStore()->setConst(m_IDProbeTrackContainer));
    return StatusCode::SUCCESS;
  }
}//namespace
