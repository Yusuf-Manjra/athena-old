#====================================================================
# BPHY7.py
# 
# https://twiki.cern.ch/twiki/bin/view/AtlasProtected/LfvBphy7 
#====================================================================


#====================================================================
# FLAGS TO PERSONALIZE THE DERIVATION
#====================================================================

onlyAugmentations = False  # Set to True to deactivate thinning and skimming, and only keep augmentations (to generate a sample with full xAOD plus all the extra)
thinTruth = True
addMuExtrapolationForTrigger = True

from InDetRecExample import TrackingCommon
from DerivationFrameworkCore.DerivationFrameworkMaster import *
isSimulation = False
if globalflags.DataSource()=='geant4':
    isSimulation = True

from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.METCommon import *


#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_BPHY7Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_BPHY7Stream )

BPHY7Stream = MSMgr.NewPoolRootStream( streamName, fileName )
BPHY7Stream.AcceptAlgs(["BPHY7Kernel2"])

## 0/ setup vertexing tools and services
#include( "JpsiUpsilonTools/configureServices.py" )

include("DerivationFrameworkBPhys/configureVertexing.py")
BPHY7_VertexTools = BPHYVertexTools("BPHY7")


#====================================================================
# TriggerCounting for Kernel1 #Added by Matteo
#====================================================================
#List of trigggers to be counted (high Sig-eff*Lumi ones are in)
triggersToMetadata= ["HLT_2mu10",
                     "HLT_2mu10_bJpsimumu",
                     "HLT_2mu10_bJpsimumu_delayed",
                     "HLT_2mu10_l2msonly",
                     "HLT_2mu10_nomucomb",
                     "HLT_2mu14",
                     "HLT_2mu14_nomucomb",
                     "HLT_2mu4",
                     "HLT_2mu4_bBmumuxv2",
                     "HLT_2mu4_bDimu_noinvm_novtx_ss",
                     "HLT_2mu6",
                     "HLT_2mu6_10invm30_pt2_z10",
                     "HLT_2mu6_bBmumu",
                     "HLT_2mu6_bBmumux_Taumumux",
                     "HLT_2mu6_bBmumuxv2",
                     "HLT_2mu6_bBmumuxv2_delayed",
                     "HLT_2mu6_bDimu_noinvm_novtx_ss",
                     "HLT_2mu6_bJpsimumu",
                     "HLT_2mu6_bJpsimumu_delayed",
                     "HLT_2mu6_bJpsimumu_Lxy0_delayed",
                     "HLT_2mu6_nomucomb_bPhi",
                     "HLT_2mu6_nomucomb_mu4_nomucomb_bTau_L12MU6_3MU4",
                     "HLT_3mu4",
                     "HLT_3mu4_bDimu",
                     "HLT_3mu4_bDimu2700",
                     "HLT_3mu4_bTau",
                     "HLT_3mu4_l2msonly",
                     "HLT_3mu4_nomucomb_bTau",
                     "HLT_3mu4_nomucomb_delayed",
                     "HLT_3mu6",
                     "HLT_3mu6_bTau",
                     "HLT_3mu6_msonly",
                     "HLT_mu10_mu6_bBmumux_BcmumuDsloose_delayed",
                     "HLT_mu10_mu6_bBmumux_Taumumux",
                     "HLT_mu10_mu6_bBmumux_Taumumux_noL2",
                     "HLT_mu10_mu6_bBmumuxv2",
                     "HLT_mu10_mu6_bBmumuxv2_delayed",
                     "HLT_mu10_mu6_bJpsimumu",
                     "HLT_mu10_mu6_bJpsimumu_Lxy0",
                     "HLT_mu10_mu6_bJpsimumu_Lxy0_delayed",
                     "HLT_mu10_mu6_bUpsimumu",
                     "HLT_mu11_mu6_bBmumu",
                     "HLT_mu11_mu6_bBmumux_BpmumuKp",
                     "HLT_mu11_mu6_bBmumuxv2",
                     "HLT_mu11_mu6_bDimu",
                     "HLT_mu11_mu6_bDimu2700",
                     "HLT_mu11_mu6_bDimu2700_Lxy0",
                     "HLT_mu11_mu6_bDimu_Lxy0",
                     "HLT_mu11_mu6_bJpsimumu",
                     "HLT_mu11_mu6_bJpsimumu_Lxy0",
                     "HLT_mu11_mu6_bPhi",
                     "HLT_mu11_mu6_bTau",
                     "HLT_mu11_mu6_bUpsimumu",
                     "HLT_mu11_mu6noL1_bPhi_L1MU11_2MU6",
                     "HLT_mu10_mu6_bDimu",
                     "HLT_2mu6_bBmumu_Lxy0_L1BPH-2M9-2MU6_BPH-2DR15-2MU6",
                     "HLT_2mu6_bJpsimumu_Lxy0_L1BPH-2M9-2MU6_BPH-2DR15-2MU6",
                     "HLT_2mu10_bDimu",
                     "HLT_mu11_2mu4noL1_nscan03_L1MU11_2MU6",
                     "HLT_mu11_L1MU10_2mu4noL1_nscan03_L1MU10_2MU6",
                     "HLT_mu11_nomucomb_2mu4noL1_nscan03_L1MU11_2MU6",
                     "HLT_mu11_nomucomb_2mu4noL1_nscan03_L1MU11_2MU6_bTau",
                     "HLT_mu11_nomucomb_mu6noL1_nscan03_L1MU11_2MU6",
                     "HLT_mu11_nomucomb_mu6noL1_nscan03_L1MU11_2MU6_bTau",
                     "HLT_mu11_nomucomb_mu6noL1_nscan03_L1MU11_2MU6_bTau_delayed",
                     "HLT_mu18_2mu4noL1",
                     "HLT_mu18_mu8noL1",
                     "HLT_mu20_2mu4noL1",
                     "HLT_mu20_l2idonly_mu6noL1_nscan03",
                     "HLT_mu20_l2idonly_mu6noL1_nscan03_bTau",
                     "HLT_mu20_msonly_mu6noL1_msonly_nscan05",
                     "HLT_mu20_mu8noL1",
                     "HLT_mu20_nomucomb_mu6noL1_nscan03",
                     "HLT_mu20_nomucomb_mu6noL1_nscan03_bTau",
                     "HLT_mu22_2mu4noL1",
                     "HLT_mu22_mu8noL1",
                     "HLT_mu24_2mu4noL1",
                     "HLT_mu24_imedium",
                     "HLT_mu24_mu8noL1",
                     "HLT_mu26_ivarmedium",
                     "HLT_mu26i",
                     "HLT_mu50",
                     "HLT_mu6_2mu4",
                     "HLT_mu6_2mu4_bJpsi_delayed",
                     "HLT_mu6_2mu4_bTau_noL2",
                     "HLT_mu6_l2msonly_2mu4_l2msonly_L1MU6_3MU4",
                     "HLT_mu6_mu4_bBmumuxv2",
                     "HLT_mu6_mu4_bBmumuxv2_delayed",
                     "HLT_mu6_mu4_bDimu_noinvm_novtx_ss",
                     "HLT_mu6_nomucomb_2mu4_nomucomb_bTau_L1MU6_3MU4",
                     "HLT_mu6_nomucomb_2mu4_nomucomb_delayed_L1MU6_3MU4",
                     "HLT_mu20_mu6noL1_bTau",
                     "HLT_2mu6_mu4_bTau_L12MU6_3MU4",
                     "HLT_mu6_2mu4_bTau_L1MU6_3MU4",
                     "HLT_mu11_2mu4noL1_bTau_L1MU11_2MU6",
                     "HLT_mu11_mu6noL1_bTau_L1MU11_2MU6",
                     "HLT_3mu4_bPhi",
                     "HLT_mu11_mu6_bPhi",
                     "HLT_mu11_nomucomb_mu6_nomucomb_bPhi",
                     "HLT_mu11_nomucomb_mu6noL1_nscan03_L1MU11_2MU6_bPhi",
                     "HLT_mu6_2mu4_bTau_L1MU6_3MU4",
                     "HLT_mu20_mu6btrk_bTauTight",
                     "HLT_mu20_2mu2btrk_bTauTight",
                     "HLT_mu11_2mu2btrk_bTauTight_L1MU11_2MU6",
                     "HLT_3mu4_bPhi",
                     "HLT_mu11_mu6_bPhi",
                     "HLT_mu11_mu6noL1_bPhi_L1MU11_2MU6",
                     "HLT_mu11_mu6_bPhi_L1LFV-MU11",
                     "HLT_2mu6_bPhi_L1LFV-MU6" ]




triggersToMetadata_filter = list( set(triggersToMetadata) )

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__TriggerCountToMetadata
BPHY7TriggerCountToMetadata = DerivationFramework__TriggerCountToMetadata(name = "BPHY7TriggerCount",
                                                                          TriggerList = triggersToMetadata_filter,
                                                                          FolderName = "BPHY7")

ToolSvc += BPHY7TriggerCountToMetadata

#====================================================================
# PRESELECTION for Kernel1 #Added by Matteo
#====================================================================
## 1/ Setup the skimming based on triggers
##     

triggerList = [ "HLT_2mu10",
                "HLT_2mu10_l2msonly",
                "HLT_2mu10_nomucomb",
                "HLT_2mu14",
                "HLT_mu50",
                "HLT_2mu14_l2msonly",
                "HLT_2mu14_nomucomb",
                "HLT_2mu6_l2msonly_mu4_l2msonly_L12MU6_3MU4",
                "HLT_2mu6_nomucomb_mu4_nomucomb_L12MU6_3MU4",
                "HLT_mu6_2mu4",
                "HLT_mu6_l2msonly_2mu4_l2msonly_L1MU6_3MU4",
                "HLT_mu6_nomucomb_2mu4_nomucomb_L1MU6_3MU4",
                "HLT_3mu6",
                "HLT_3mu6_msonly",
                "HLT_3mu6_nomucomb",
                "HLT_mu4","HLT_mu6","HLT_mu10","HLT_mu18",
                "HLT_mu14",
                "HLT_mu24",
                "HLT_mu24_L1MU15",
                "HLT_2mu4",
                "HLT_2mu6",
                "HLT_mu20_L1MU15",
                "HLT_mu18_2mu4noL1",
                "HLT_mu18_nomucomb_2mu4noL1",
                "HLT_mu20_2mu4noL1",
                "HLT_mu20_l2idonly_2mu4noL1",
                "HLT_mu20_nomucomb_2mu4noL1",
                "HLT_mu18_mu8noL1",
                "HLT_mu18_nomucomb_mu8noL1",
                "HLT_mu20_mu8noL1",
                "HLT_mu20_l2idonly_2mu4noL1",
                "HLT_mu20_nomucomb_mu8noL1",
                "HLT_mu22_mu8noL1",
                "HLT_mu22_l2idonly_2mu4noL1",
                "HLT_mu22_nomucomb_mu8noL1",
                "HLT_mu22_2mu4noL1",
                "HLT_mu22_nomucomb_2mu4noL1",
                "HLT_mu20_2mu4noL1", "HLT_mu20_mu8noL1",
                "HLT_mu14_tau25_medium1_tracktwo",
                "HLT_mu14_tau35_medium1_tracktwo",
                "HLT_mu14_tau25_medium1_tracktwo_xe50",
                "HLT_mu14_tau35_medium1_tracktwo_L1TAU20",
                "HLT_mu24_mu8noL1",
                "HLT_mu6_nomucomb_2mu4_nomucomb_delayed_L1MU6_3MU4", 
                "HLT_2mu6_bBmumuxv2_delayed", 
                "HLT_2mu4_bDimu_noinvm_novtx_ss", 
                "HLT_2mu6_bDimu_noinvm_novtx_ss", 
                "HLT_mu24_2mu4noL1", 
                "HLT_mu10_mu6_bUpsimumu", 
                "HLT_mu10_mu6_bBmumuxv2", 
                "HLT_mu10_mu6_bJpsimumu", 
                "HLT_mu6_mu4_bBmumuxv2_delayed", 
                "HLT_2mu6_10invm30_pt2_z10", 
                "HLT_2mu6_nomucomb_bPhi",
                "HLT_mu6_mu4_bDimu_noinvm_novtx_ss",
                "HLT_mu11_mu6_bDimu2700",
		"HLT_2mu6_bBmumux_Taumumux",
                "HLT_mu10_mu6_bBmumux_Taumumux",
                "HLT_mu10_mu6_bBmumux_Taumumux_noL2",
                "HLT_.*mu11_mu6.*",     # Recent triggers
                "HLT_.*3mu4.*",
                "HLT_.*mu.*imedium.*",	# Trigger with looser isolation selection 
                "HLT_.*mu.*iloose.*",
                "HLT_.*mu6.*2mu4.*",
                "HLT_.*mu11.*2mu4noL1.*",
                "HLT_.*2mu14_nomucomb.*",
                "HLT_.*bTau.*",		# Our tau triggers
                "HLT_.*bDimu2700.*",
                "HLT_.*bPhi.*",
                "HLT_.*bBmumuxv2.*",
                "HLT_.*nscan.*"  ]	# Narrow scan triggers

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
BPHY7TriggerSkim = DerivationFramework__TriggerSkimmingTool(name = "BPHY7TriggerSkim",
                                                            TriggerListOR = triggerList,
                                                            TriggerListAND = [] )

ToolSvc += BPHY7TriggerSkim


#====================================================================
# 2mu vertex for Kernel2 #Added by Matteo
#====================================================================

from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiFinder
BPHY7DiMuon_Finder = Analysis__JpsiFinder(name                         = "BPHY7DiMuon_Finder",
                                          #    OutputLevel                 = DEBUG,
                                          muAndMu                     = True,
                                          muAndTrack                  = False,
                                          TrackAndTrack               = False,
                                          assumeDiMuons               = True, 
                                          invMassUpper                = 2900.0, # Cut just below the J/psi
                                          invMassLower                = 0.0,
                                          Chi2Cut                     = 110., #CHANGED! Was 200
                                          oppChargesOnly	            = False,
                                          allChargeCombinations	    = True,
                                          combOnly		    = False,
                                          atLeastOneComb		    = True,
                                          useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
                                          muonCollectionKey           = "Muons",
                                          TrackParticleCollection     = "InDetTrackParticles",
                                          V0VertexFitterTool          = BPHY7_VertexTools.TrkV0Fitter,             # V0 vertex fitter
                                          useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
                                          TrkVertexFitterTool         = BPHY7_VertexTools.TrkVKalVrtFitter,        # VKalVrt vertex fitter
                                          TrackSelectorTool           = BPHY7_VertexTools.InDetTrackSelectorTool,
                                          VertexPointEstimator        = BPHY7_VertexTools.VtxPointEstimator,
                                          useMCPCuts                  = False)
ToolSvc += BPHY7DiMuon_Finder

#--------------------------------------------------------------------
##Comment from BPHY2...
## 3/ setup the vertex reconstruction "call" tool(s). They are part of the derivation framework.
##    These Augmentation tools add output vertex collection(s) into the StoreGate and add basic 
##    decorations which do not depend on the vertex mass hypothesis (e.g. lxy, ptError, etc).
##    There should be one tool per topology, i.e. Jpsi and Psi(2S) do not need two instance of the
##    Reco tool is the JpsiFinder mass window is wide enough.
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_Vertex
BPHY7DiMuon_SelectAndWrite = DerivationFramework__Reco_Vertex(name              = "BPHY7DiMuon_SelectAndWrite",
                                                            VertexSearchTool    	      = BPHY7DiMuon_Finder,
                                                            OutputVtxContainerName = "BPHY7TwoMuCandidates",
                                                            V0Tools                = TrackingCommon.getV0Tools(),
                                                            PVRefitter             = BPHY7_VertexTools.PrimaryVertexRefitter,
                                                            PVContainerName        = "PrimaryVertices",
                                                            RefPVContainerName     = "SHOULDNOTBEUSED_DiMuonRefittedPV")
ToolSvc += BPHY7DiMuon_SelectAndWrite

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Select_onia2mumu

## a/ augment and select Jpsi->mumu candidates
BPHY7DiMuon_Decorator = DerivationFramework__Select_onia2mumu(name                  = "BPHY7DiMuon_Decorator",
                                                              HypothesisName        = "Jpsi",
                                                              InputVtxContainerName = "BPHY7TwoMuCandidates",
                                                              V0Tools               = TrackingCommon.getV0Tools(),
                                                              VtxMassHypo           = 1230,   # used to determine time-of-flight and thus lifetime (deviations and sigmas are also added to the vertex)
                                                              MassMin               = 0.0,
                                                              MassMax               = 2900.0,
                                                              Chi2Max               = 200,
                                                              DoVertexType =1)              #	1 = Pt, 2 = A0, 4 = Z0
  
ToolSvc += BPHY7DiMuon_Decorator
#====================================================================
# 3mu/2mu+trk vertex for Kernel2 #Added by Matteo
#====================================================================
## 4/ setup a new vertexing tool (necessary due to use of mass constraint) 
from TrkVKalVrtFitter.TrkVKalVrtFitterConf import Trk__TrkVKalVrtFitter
BpmVertexFit = Trk__TrkVKalVrtFitter(name                = "BpmVertexFit",
                                     Extrapolator        = BPHY7_VertexTools.InDetExtrapolator,
                                     FirstMeasuredPoint  = True,
                                     MakeExtendedVertex  = True)
ToolSvc += BpmVertexFit

## 5/ setup the Jpsi+1 track finder
from JpsiUpsilonTools.JpsiUpsilonToolsConf import Analysis__JpsiPlus1Track
BPHY7ThreeMuon_Finder = Analysis__JpsiPlus1Track(name 				= "BPHY7ThreeMuon_Finder",
                                                 OutputLevel 			= INFO,
                                                 pionHypothesis			= True,
                                                 kaonHypothesis			= False,
                                                 trkThresholdPt			= 1000.0,
                                                 #trkMaxEta			= 2.5, # is this value fine?? default would be 102.5
                                                 BThresholdPt			= 1000.0,
                                                 BMassUpper			= 5000.0, # What is this??
                                                 BMassLower			= 0.0,
                                                 JpsiContainerKey		= "BPHY7TwoMuCandidates",
                                                 TrackParticleCollection		= "InDetTrackParticles",
                                                 MuonsUsedInJpsi			= "NONE", #cannnot allow, would kill 3muons
                                                 ExcludeCrossJpsiTracks		= False,
                                                 TrkVertexFitterTool		= BpmVertexFit,
                                                 TrackSelectorTool		= BPHY7_VertexTools.InDetTrackSelectorTool,
                                                 UseMassConstraint		= False, 
                                                 Chi2Cut 			= 150) #Cut on chi2/Ndeg_of_freedom, so is very loose
												
        
ToolSvc += BPHY7ThreeMuon_Finder

## 6/ setup the combined augmentation/skimming tool for the Bpm
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Reco_Vertex
BPHY7ThreeMuon_SelectAndWrite = DerivationFramework__Reco_Vertex(name                     = "BPHY7ThreeMuon_SelectAndWrite",
                                                                  OutputLevel              = INFO,
                                                                  VertexSearchTool       = BPHY7ThreeMuon_Finder,
                                                                  OutputVtxContainerName   = "BPHY7Tau3MuCandidates",
                                                                  PVContainerName          = "PrimaryVertices",
                                                                  V0Tools                  = TrackingCommon.getV0Tools(),
                                                                  PVRefitter               = BPHY7_VertexTools.PrimaryVertexRefitter,
                                                                  RefPVContainerName       = "BPHY7RefittedPrimaryVertices",
                                                                  RefitPV                  = True,
                                                                  MaxPVrefit = 1000)
ToolSvc += BPHY7ThreeMuon_SelectAndWrite 

## b/ augment and select Bplus->JpsiKplus candidates
BPHY7ThreeMuon_Decorator = DerivationFramework__Select_onia2mumu(
  name                       = "BPHY7ThreeMuon_Decorator",
  OutputLevel                = INFO,
  HypothesisName             = "Tau3MuLoose",
  InputVtxContainerName      = "BPHY7Tau3MuCandidates",
  V0Tools                    = TrackingCommon.getV0Tools(),
  TrkMasses                  = [105.658, 105.658, 105.658],
  VtxMassHypo                = 1777.,
  MassMin                    = 0.0,
  MassMax                    = 5000.,  # If the two selections start differing one might have to check that the tools below still run on the right vertices
  Chi2Max                    = 100.)

ToolSvc += BPHY7ThreeMuon_Decorator

## b/ augment and select Bplus->JpsiKplus candidates
BPHY7ThreeMuon_Decorator2 = DerivationFramework__Select_onia2mumu(
  name                       = "BPHY7ThreeMuon_Decorator2",
  OutputLevel                = INFO,
  HypothesisName             = "Ds2MuPi",
  InputVtxContainerName      = "BPHY7Tau3MuCandidates",
  V0Tools                    = TrackingCommon.getV0Tools(),
  TrkMasses                  = [105.658, 105.658, 139.57],
  VtxMassHypo                = 1968.3,
  MassMin                    = 0.0,
  MassMax                    = 5000.,  # If the two selections start differing one might have to check that the tools below still run on the right vertices
  Chi2Max                    = 100.)

ToolSvc += BPHY7ThreeMuon_Decorator2

#Track isolation for candidates
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__VertexTrackIsolation
BPHY7TrackIsolationDecorator = DerivationFramework__VertexTrackIsolation(
  name                            = "BPHY7TrackIsolationDecorator",
  OutputLevel                     = INFO,
  TrackIsoTool 	                  = "xAOD::TrackIsolationTool",
  TrackContainer                  = "InDetTrackParticles",
  InputVertexContainer            = "BPHY7Tau3MuCandidates",
  PassFlags                       = ["passed_Tau3MuLoose", "passed_Ds2MuPi"] )

ToolSvc += BPHY7TrackIsolationDecorator

#CaloIsolationTool explicitly declared to avoid pointless warnings (it works!!!)
from IsolationTool.IsolationToolConf import xAOD__CaloIsolationTool
BPHY7CaloIsolationTool = xAOD__CaloIsolationTool(
  name                            = "BPHY7CaloIsolationTool",
  OutputLevel                     = WARNING,                  
  saveOnlyRequestedCorrections    = True,
  IsoLeakCorrectionTool           = "" ) #Workaround for a bug in older versions

ToolSvc += BPHY7CaloIsolationTool

#Calo isolation for candidates
from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__VertexCaloIsolation
BPHY7CaloIsolationDecorator = DerivationFramework__VertexCaloIsolation(
  name                            = "BPHY7CaloIsolationDecorator",
  OutputLevel                     = INFO,                  
  CaloIsoTool                     = BPHY7CaloIsolationTool,  #"xAOD::CaloIsolationTool",
  TrackContainer                  = "InDetTrackParticles",
  InputVertexContainer            = "BPHY7Tau3MuCandidates",
  CaloClusterContainer            = "CaloCalTopoClusters",
  ParticleCaloExtensionTool       = "Trk::ParticleCaloExtensionTool/ParticleCaloExtensionTool",
  PassFlags                       = ["passed_Tau3MuLoose", "passed_Ds2MuPi"] )

ToolSvc += BPHY7CaloIsolationDecorator

#====================================================================
# Skimming tool to select only events with the correct vertices
#====================================================================

#--------------------------------------------------------------------
## 9/ select the event. We only want to keep events that contain certain three-mu vertices which passed certain selection.
##    Exactly like in the preselection, where only 2mu vertices are treated.

expression = "count(BPHY7Tau3MuCandidates.passed_Tau3MuLoose) > 0 || count(BPHY7Tau3MuCandidates.passed_Ds2MuPi) > 0"

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
BPHY7_SelectEvent = DerivationFramework__xAODStringSkimmingTool(name 		= "BPHY7_SelectEvent",
                                                                OutputLevel   	= INFO,
                                                                expression 	= expression)

ToolSvc += BPHY7_SelectEvent
print(BPHY7_SelectEvent)

#====================================================================
# Add Extrapolation of muons to trigger layers
#====================================================================

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__MuonExtrapolationTool 
BPHY7_Extrap_Tool = DerivationFramework__MuonExtrapolationTool(   name = "BPHY7_ExtrapolationTool",   OutputLevel = INFO ) 

ToolSvc += BPHY7_Extrap_Tool


#====================================================================
# Thinning Helper and various thinning tools
#====================================================================

#--------------------------------------------------------------------
## 10/ Setup the thinning helper, only tool able to perform thinning of trigger navigation information

from DerivationFrameworkCore.ThinningHelper import ThinningHelper
BPHY7ThinningHelper = ThinningHelper( "BPHY7ThinningHelper" )
BPHY7ThinningHelper.TriggerChains = 'HLT_.*mu.*' #triggerList	# . = any character; * = 0 or more times; + = 1 or more times; ? 0 or 1 times  "Regular_Expression"
BPHY7ThinningHelper.AppendToStream( BPHY7Stream )


#--------------------------------------------------------------------
## 11/ track and vertex thinning. We want to remove all reconstructed secondary vertices
##    which haven't passed any of the selections defined by (Select_*) tools.
##    We also want to keep only tracks which are associates with either muons or any of the
##    vertices that passed the selection. Multiple thinning tools can perform the 
##    selection. The final thinning decision is based OR of all the decisions (by default,
##    although it can be changed by the JO).

## 12/ Cleans up, removing duplicate vertices. An issue caused by the logic of Jpsi+1 track in the case of 3-muon candidates

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Thin_vtxDuplicates
BPHY7Thin_vtxDuplicates = DerivationFramework__Thin_vtxDuplicates(name                       = "BPHY7Thin_vtxDuplicates",
                                                                  OutputLevel                = INFO,
                                                                  VertexContainerName       = "BPHY7Tau3MuCandidates",
                                                                  PassFlags                  = ["passed_Tau3MuLoose", "passed_Ds2MuPi"])

ToolSvc += BPHY7Thin_vtxDuplicates

## a) thining out vertices that didn't pass any selection and idetifying tracks associated with 
##    selected vertices. The "VertexContainerNames" is a list of the vertex containers, and "PassFlags"
##    contains all pass flags for Select_* tools that must be satisfied. The vertex is kept is it 
##    satisfy any of the listed selections.

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Thin_vtxTrk
BPHY7Thin_vtxTrk = DerivationFramework__Thin_vtxTrk(
  name                       = "BPHY7Thin_vtxTrk",
  OutputLevel                = INFO,
  TrackParticleContainerName = "InDetTrackParticles",
  AcceptanceRadius	     = 1.,
  VertexContainerNames       = ["BPHY7Tau3MuCandidates"],
  PassFlags                  = ["passed_Tau3MuLoose", "passed_Ds2MuPi"],
  ApplyAnd                   = True )  # "and" requirement for Vertices

ToolSvc += BPHY7Thin_vtxTrk


## 13/ thinning out tracks that are not attached to muons. The final thinning decision is based on the OR operation
##     between decision from this and the previous tools.
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
BPHY7MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name                    = "BPHY7MuonTPThinningTool",
                                                                         MuonKey                 = "Muons",
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += BPHY7MuonTPThinningTool

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__BPhysPVThinningTool
BPHY7_thinningTool_PV = DerivationFramework__BPhysPVThinningTool(name                       = "BPHY7_thinningTool_PV",
                                                                 CandidateCollections       = ["BPHY7Tau3MuCandidates"],
                                                                 KeepPVTracks  =True)

ToolSvc += BPHY7_thinningTool_PV

from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TauTrackParticleThinning
BPHY7TauTPThinningTool = DerivationFramework__TauTrackParticleThinning(name                    = "BPHY7TauTPThinningTool",
                                                                       TauKey                 = "TauJets",
                                                                       InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += BPHY7TauTPThinningTool

# Only save truth informtion directly associated with: mu Ds+ D+ D*+ Ds*+ D0 D*0 B+ B*+ B0 B*0 
from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning
BPHY7TruthThinTool = DerivationFramework__GenericTruthThinning(name                    = "BPHY7TruthThinTool",
                                                               ParticleSelectionString = "abs(TruthParticles.pdgId) == 13 || abs(TruthParticles.pdgId) == 431 || abs(TruthParticles.pdgId) == 411 || abs(TruthParticles.pdgId) == 413 || abs(TruthParticles.pdgId) == 433 || TruthParticles.pdgId == 421 || TruthParticles.pdgId == 423 || abs(TruthParticles.pdgId) == 521 || abs(TruthParticles.pdgId) == 523 || TruthParticles.pdgId == 511 || TruthParticles.pdgId == 513",
                                                               PreserveDescendants     = True,
                                                               PreserveAncestors      = True)
ToolSvc += BPHY7TruthThinTool

# Only save truth neutrino and b/c quarks information
from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__GenericTruthThinning
BPHY7TruthThinNoChainTool = DerivationFramework__GenericTruthThinning(name                    = "BPHY7TruthThinNoChainTool",
                                                              ParticleSelectionString = "abs(TruthParticles.pdgId) == 4 || abs(TruthParticles.pdgId) == 5 || abs(TruthParticles.pdgId) == 12 || abs(TruthParticles.pdgId) == 14 || abs(TruthParticles.pdgId) == 16",
                                                              PreserveDescendants     = False,
                                                              PreserveAncestors      = False)
ToolSvc += BPHY7TruthThinNoChainTool


#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS  
#====================================================================

BPHY7ThinningTools = [ BPHY7MuonTPThinningTool, BPHY7Thin_vtxDuplicates, BPHY7Thin_vtxTrk, BPHY7_thinningTool_PV,  BPHY7TauTPThinningTool]

BPHY7SkimmingTools = [BPHY7_SelectEvent]

BPHY7AugmentationTools = [BPHY7DiMuon_SelectAndWrite, BPHY7DiMuon_Decorator, BPHY7ThreeMuon_SelectAndWrite, BPHY7ThreeMuon_Decorator, BPHY7ThreeMuon_Decorator2, BPHY7TrackIsolationDecorator, BPHY7CaloIsolationDecorator]

if addMuExtrapolationForTrigger:
    BPHY7AugmentationTools.append(BPHY7_Extrap_Tool)

Kernel1Tools = [BPHY7TriggerSkim]

if isSimulation:
    #BPHY7AugmentationTools.append(DFCommonTauTruthMatchingWrapper)
    if thinTruth:
       BPHY7ThinningTools.append(BPHY7TruthThinTool)
       BPHY7ThinningTools.append(BPHY7TruthThinNoChainTool)

#The sequence object. Is in principle just a wrapper which allows to run two kernels in sequence
BPHY7_Sequence = CfgMgr.AthSequencer("BPHY7_Sequence")
from DerivationFrameworkFlavourTag.FlavourTagCommon import FlavorTagInit
FlavorTagInit(JetCollections=['AntiKt4EMPFlowJets'], Sequencer=BPHY7_Sequence)


#onlyAugmentations implementation
if onlyAugmentations:
    Kernel1Tools = []
    BPHY7SkimmingTools = []
    BPHY7ThinningTools = []

# Kernel n1 PRESELECTION
# The name of the kernel (BPHY7Kernel1 in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
BPHY7_Sequence += CfgMgr.DerivationFramework__DerivationKernel("BPHY7Kernel1",
                                                               AugmentationTools = [BPHY7TriggerCountToMetadata] ,
                                                               SkimmingTools     = Kernel1Tools)
# Kernel n2 deep Derivation
# The name of the kernel (BPHY7Kernel2 in this case) must be unique to this derivation
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
BPHY7_Sequence += CfgMgr.DerivationFramework__DerivationKernel("BPHY7Kernel2",
                                                               AugmentationTools = BPHY7AugmentationTools,
                                                               SkimmingTools     = BPHY7SkimmingTools, 
                                                               ThinningTools     = BPHY7ThinningTools)

#Vital, replaces the adding of kernels directly
DerivationFrameworkJob += BPHY7_Sequence

#====================================================================
# Slimming 
#====================================================================

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
BPHY7SlimmingHelper = SlimmingHelper("BPHY7SlimmingHelper")


SmartCollections = ["Electrons", "Photons", "TauJets", "AntiKt4EMTopoJets_BTagging201810", "BTagging_AntiKt4EMTopo_201810", "PrimaryVertices", "Muons", "InDetTrackParticles", "MET_Reference_AntiKt4EMTopo"]


AllVariables = ["METAssoc_AntiKt4EMTopo",
                 "MET_Core_AntiKt4EMTopo",
                 "MET_Truth",
                 "MET_Track",
                 "MET_LocHadTopo"]

AllVariables += ["Kt4EMTopoOriginEventShape",
                 "Kt4EMTopoEventShape"]

AllVariables += ["CombinedMuonTrackParticles",
                 "ExtrapolatedMuonTrackParticles",
                 "MuonSpectrometerTrackParticles"]


ExtraVariables = ["Photons.pt.eta.phi.m",
                  "Electrons.pt.eta.phi.m","TauJets.pt.eta.phi.m.IsTruthMatched.truthJetLink.truthParticleLink",
                  "AntiKt4EMTopoJets_BTagging201810.JetPileupScaleMomentum_pt.JetPileupScaleMomentum_eta.JetPileupScaleMomentum_phi.JetPileupScaleMomentum_m", 
                  "AntiKt4EMTopoJets_BTagging201810.JvtJvfcorr.HECFrac.LArQuality.HECQuality.NegativeE.AverageLArQF", 
                  "AntiKt4EMTopoJets_BTagging201810.JetEtaJESScaleMomentum_pt.JetEtaJESScaleMomentum_eta.JetEtaJESScaleMomentum_phi.JetEtaJESScaleMomentum_m"]

ExtraVariables += ["Muons.etaLayer1Hits.etaLayer2Hits.etaLayer3Hits.etaLayer4Hits.phiLayer1Hits.phiLayer2Hits.phiLayer3Hits.phiLayer4Hits",
                   "Muons.numberOfTriggerEtaLayers.numberOfPhiLayers",
                   "CombinedMuonTrackParticles.numberOfTRTHits.numberOfTRTHighThresholdHits", 
                   "InDetTrackParticles.numberOfTRTHits.numberOfTRTHighThresholdHits.vx.vy.vz",
                   "PrimaryVertices.chiSquared.covariance"]


StaticContent =  ["xAOD::VertexContainer#BPHY7RefittedPrimaryVertices",
                  "xAOD::VertexAuxContainer#BPHY7RefittedPrimaryVerticesAux."]

# ThreeBody candidates (vertices)
StaticContent += ["xAOD::VertexContainer#%s"        % BPHY7ThreeMuon_SelectAndWrite.OutputVtxContainerName]
StaticContent += ["xAOD::VertexAuxContainer#%sAux." % BPHY7ThreeMuon_SelectAndWrite.OutputVtxContainerName]
## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY7ThreeMuon_SelectAndWrite.OutputVtxContainerName]

# Truth information for MC only
if isSimulation:
    AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles", "METMap_Truth"]
    SmartCollections += ["AntiKt4TruthJets"] 

# Needed for trigger objects
BPHY7SlimmingHelper.IncludeMuonTriggerContent = True
BPHY7SlimmingHelper.IncludeBPhysTriggerContent = True

# Pass all lists to the SlimmingHelper
BPHY7SlimmingHelper.ExtraVariables = ExtraVariables
BPHY7SlimmingHelper.AllVariables = AllVariables
BPHY7SlimmingHelper.StaticContent = StaticContent
BPHY7SlimmingHelper.SmartCollections = SmartCollections
BPHY7SlimmingHelper.AppendContentToStream(BPHY7Stream)

