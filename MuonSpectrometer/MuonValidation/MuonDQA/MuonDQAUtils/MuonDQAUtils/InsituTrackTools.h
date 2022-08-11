/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Muon_InsituTrackTools_H
#define Muon_InsituTrackTools_H

/// General Classes
#include <stdint.h>
#include <algorithm>
#include <functional>
#include <string>
/// Gaudi Tools
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
/// ROOT Classes
#include "TH1.h"
#include "TH2.h" 
#include "TLorentzVector.h"
/// Muon Trigger
#include "AnalysisTriggerEvent/LVL1_ROI.h"
#include "AnalysisTriggerEvent/Muon_ROI.h"
#include "TrigMuonEvent/CombinedMuonFeature.h"
#include "TrigMuonEvent/TrigMuonEFContainer.h"
/// Track Particles
#include "Particle/TrackParticle.h"
#include "Particle/TrackParticleContainer.h"
/// Electron
#include "egammaEvent/Electron.h"
/// Muon
#include "muonEvent/MuonContainer.h"
#include "muonEvent/Muon.h"
/// common implementation of all particles
#include "ParticleEvent/ParticleBaseContainer.h"
/// particle jets
#include "JetTagEvent/JetConstituent.h"
#include "JetEvent/JetCollection.h"
#include "JetTagEvent/TrackConstituents.h" 

#include "EventKernel/INavigable4Momentum.h"
#include "MuonDQAUtils/IInsituTrackTools.h"

class AtlasDetectorID;
class Identifier;


/** @class InsituTrackTools 

This is for the Doxygen-Documentation.  
Please delete these lines and fill in information about the Algorithm!
Please precede every member function declaration with a
short Doxygen comment stating the purpose of this function.

@author  Nektarios Benekos <nbenekos@illinois.edu>
*/  
namespace Muon 
{
  class InsituTrackTools : virtual public IInsituTrackTools, public AthAlgTool
    {
    public:
      InsituTrackTools(const std::string&,const std::string&,const IInterface*);
	
      /** default destructor */
      virtual ~InsituTrackTools () {};
		
      virtual bool	isZBosonCandidate(const INavigable4Momentum *track1, const INavigable4Momentum *track2) override;
      virtual bool	isTriggeredMuon(const INavigable4Momentum *track1) override;
      virtual bool	isTriggeredElectron(const INavigable4Momentum *track1) override;
      virtual bool	isCloseTrack(const INavigable4Momentum *track1, const INavigable4Momentum *track2) override;
      virtual bool	isIsolatedTrack(const INavigable4Momentum *track1) override;
      virtual bool	isIsolatedMuon(const INavigable4Momentum *track1) override;
      virtual bool	isElectronCandidate(const INavigable4Momentum *track1) override;
		
      virtual double	getInvariantMass(const INavigable4Momentum *track1, const INavigable4Momentum *track2) override;
      virtual double	getDistance(const INavigable4Momentum *track1, const INavigable4Momentum *track2) override;
      virtual bool	getTrackIsolation(const INavigable4Momentum *trackParticle, float &PtIsolation, int &NIsolation) override;
      virtual float	getTrackPtIsolation(const INavigable4Momentum *trackParticle) override;
      virtual int	getNTrackIsolation(const INavigable4Momentum *trackParticle) override;
      virtual float	getJetIsolation(const INavigable4Momentum *trackParticle) override;
       	
    private:
		
      /** member variables for algorithm properties: */
      std::string	m_InnerTrackContainerName;
      std::string	m_ConeJetContainerName;
		
      /// Cut variables
      double		m_MaximalMassDifferenceToZBoson;
      double		m_MaximalPTofTracksInCone;
      double		m_MaximalNumberofTracksInCone;
      double		m_MaximalJetEnergyInCone;
    }; 
}

#endif 
