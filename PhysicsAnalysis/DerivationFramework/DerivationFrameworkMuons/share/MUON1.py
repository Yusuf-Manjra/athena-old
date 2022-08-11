#====================================================================
# MUON1.py for Z->mumu events
# This requires the reductionConf flag MUON1 in Reco_tf.py   
#====================================================================

# Set up common services and job object.
# This should appear in ALL derivation job options
from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkMuons.MuonsCommon import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
# from DerivationFrameworkJetEtMiss.METCommon import *
import AthenaCommon.SystemOfUnits as Units

#from MuonPerformanceAlgs.CommonMuonTPConfig import GetIDTrackCaloDepositsDecorator
if not hasattr(ToolSvc,"IDTrackCaloDepositsDecoratorTool"):
  from DerivationFrameworkMuons.DerivationFrameworkMuonsConf import IDTrackCaloDepositsDecoratorTool
  DecoTool = IDTrackCaloDepositsDecoratorTool("IDTrackCaloDepositsDecoratorTool")
  if hasattr(DecoTool, "TrackDepositInCaloTool"):
    if not hasattr(ToolSvc,"TrkDepositInCaloTool"):
        from CaloTrkMuIdTools.CaloTrkMuIdToolsConf import TrackDepositInCaloTool
        TrkDepositInCaloTool = TrackDepositInCaloTool("TrkDepositInCaloTool")
        TrkDepositInCaloTool.CaloCellContainerName = "AODCellContainer"
        ToolSvc += TrkDepositInCaloTool
    DecoTool.TrackDepositInCaloTool = ToolSvc.TrkDepositInCaloTool

    ToolSvc += DecoTool


if DerivationFrameworkHasTruth:
  from DerivationFrameworkMCTruth.MCTruthCommon import addStandardTruthContents
  addStandardTruthContents()

#====================================================================
# SET UP STREAM   
#====================================================================
MUON1Stream = MSMgr.NewPoolRootStream(derivationFlags.WriteDAOD_MUON1Stream.StreamName, buildFileName( derivationFlags.WriteDAOD_MUON1Stream))
MUON1Stream.AcceptAlgs(["MUON1Kernel"])

### trigger thinning
triggerList1 = ['HLT_.*mu\d+.*']

from DerivationFrameworkCore.ThinningHelper import ThinningHelper
MUON1ThinningHelper = ThinningHelper( "MUON1ThinningHelper" )
MUON1ThinningHelper.TriggerChains = '|'.join(triggerList1)
printfunc (MUON1ThinningHelper.TriggerChains)
MUON1ThinningHelper.AppendToStream( MUON1Stream )

#====================================================================
# AUGMENTATION TOOLS
#====================================================================
MUON1AugmentTools = []

skimmingORs = []
thinningORs = []

### Z->mumu events
brPrefix1a = 'MUON1a'
from DerivationFrameworkMuons.DerivationFrameworkMuonsConf import DerivationFramework__dimuonTaggingTool
MUON1AugmentTool1a = DerivationFramework__dimuonTaggingTool(name = "MUON1AugmentTool1a",
                                                           IDTrackCaloDepoDecoTool = ToolSvc.IDTrackCaloDepositsDecoratorTool,
                                                           Mu1PtMin = 24*Units.GeV,
                                                           Mu1AbsEtaMax = 2.5,
                                                           Mu1Types = [0],
                                                           Mu1Trigs = [],
                                                           Mu1IsoCuts = {},
                                                           Mu2PtMin = 2.5*Units.GeV,
                                                           Mu2AbsEtaMax = 9999.,
                                                           Mu2Types = [],
                                                           Mu2Trigs = [],
                                                           Mu2IsoCuts = {},
                                                           UseTrackProbe = True, # bool
                                                           TrackContainerKey = 'InDetTrackParticles', # str
                                                           OppositeCharge = False,
                                                           InvariantMassLow = 60*Units.GeV,
                                                           InvariantMassHigh = -1,
                                                           IDTrackThinningConeSize = 0.4,
                                                           BranchPrefix = brPrefix1a
                                                           )

ToolSvc += MUON1AugmentTool1a
MUON1AugmentTools.append(MUON1AugmentTool1a)
skimmingORs.append(brPrefix1a+'DIMU_pass>0')
thinningORs.append(brPrefix1a+'DIMU_trkStatus>0')
printfunc (MUON1AugmentTool1a)


### Jpsi for tag-probe
andTriggers1b = ['HLT_mu20_iloose_L1MU15', 'HLT_mu24', 'HLT_mu26', 'HLT_mu24_imedium', 'HLT_mu26_imedium']
orTriggers1b = ['HLT_mu4','HLT_mu6','HLT_mu14','HLT_mu6_idperf',"HLT_mu6_bJpsi_lowpt_TrkPEB", 'HLT_mu4_bJpsi_Trkloose','HLT_mu6_bJpsi_Trkloose ','HLT_mu10_bJpsi_Trkloose','HLT_mu18_bJpsi_Trkloose','HLT_mu20_2mu0noL1_JpsimumuFS','HLT_mu18_2mu0noL1_JpsimumuFS','HLT_mu20_2mu4_JpsimumuL2','HLT_mu18_2mu4_JpsimumuL2','HLT_mu4_mu4_idperf_bJpsimumu_noid','HLT_mu4_bJpsi_TrkPEB','HLT_mu6_bJpsi_TrkPEB','HLT_mu10_bJpsi_TrkPEB','HLT_mu14_bJpsi_TrkPEB','HLT_mu20_bJpsi_TrkPEB','HLT_mu6_mu2noL1_msonly_bJpsimumu_noid','HLT_mu6_mu2noL1_msonly_bJpsimumu_noid_PEB']

brPrefix1b = 'MUON1b'
MUON1AugmentTool1b = DerivationFramework__dimuonTaggingTool(name = "MUON1AugmentTool1b",
                                                           IDTrackCaloDepoDecoTool = ToolSvc.IDTrackCaloDepositsDecoratorTool,
                                                           OrTrigs = orTriggers1b,
                                                           AndTrigs = andTriggers1b,
                                                           Mu1PtMin = 4*Units.GeV,
                                                           Mu1AbsEtaMax = 2.5,
                                                           Mu1Types = [0],
                                                           Mu1Trigs = [],
                                                           Mu1IsoCuts = {},
                                                           Mu2PtMin = 2.5*Units.GeV,
                                                           Mu2AbsEtaMax = 9999.,
                                                           Mu2Types = [],
                                                           Mu2Trigs = [],
                                                           Mu2IsoCuts = {},
                                                           UseTrackProbe = True, # bool
                                                           TrackContainerKey = 'InDetTrackParticles', # str
                                                           OppositeCharge = False,
                                                           InvariantMassLow = 2.0*Units.GeV,
                                                           InvariantMassHigh = 4.8*Units.GeV,
                                                           IDTrackThinningConeSize = 0.4,
                                                           BranchPrefix = brPrefix1b
                                                          )

ToolSvc += MUON1AugmentTool1b
MUON1AugmentTools.append(MUON1AugmentTool1b)
skimmingORs.append(brPrefix1b+'DIMU_pass>0')
thinningORs.append(brPrefix1b+'DIMU_trkStatus>0')
printfunc (MUON1AugmentTool1b)


### Jpsi for calibration
brPrefix1c = 'MUON1c'
orTriggers1c = []
andTriggers1c = []

MUON1AugmentTool1c = DerivationFramework__dimuonTaggingTool(name = "MUON1AugmentTool1c",
                                                           IDTrackCaloDepoDecoTool = ToolSvc.IDTrackCaloDepositsDecoratorTool,
                                                           OrTrigs = orTriggers1c,
                                                           AndTrigs = andTriggers1c,
                                                           Mu1PtMin = 5.*Units.GeV,
                                                           Mu1AbsEtaMax = 999.,
                                                           Mu1Types = [0],
                                                           Mu1Trigs = [],
                                                           Mu1IsoCuts = {}, #ptcone20<10 GeV, etcone40<20 GeV
                                                           Mu2PtMin = 5.*Units.GeV,
                                                           Mu2AbsEtaMax = 999.,
                                                           Mu2Types = [0],
                                                           Mu2Trigs = [],
                                                           Mu2IsoCuts = {},
                                                           UseTrackProbe = False, # bool
                                                           TrackContainerKey = 'InDetTrackParticles', # str
                                                           OppositeCharge = True,
                                                           InvariantMassLow = 2.0*Units.GeV,
                                                           InvariantMassHigh = 4.8*Units.GeV,
                                                           IDTrackThinningConeSize = 0.4,
                                                           BranchPrefix = brPrefix1c
                                                           )

ToolSvc += MUON1AugmentTool1c
MUON1AugmentTools.append(MUON1AugmentTool1c)
skimmingORs.append(brPrefix1c+'DIMU_pass>0')
thinningORs.append(brPrefix1c+'DIMU_trkStatus>0')
printfunc (MUON1AugmentTool1c)

### Upsilon tagging
brPrefix1d = 'MUON1d'
orTriggers1d = []
andTriggers1d = [] # No trigger in 8TeV data

MUON1AugmentTool1d = DerivationFramework__dimuonTaggingTool(name = "MUON1AugmentTool1d",
                                                           OrTrigs = orTriggers1d,
                                                           AndTrigs = andTriggers1d,
                                                           Mu1PtMin = 5.*Units.GeV,
                                                           Mu1AbsEtaMax = 999.,
                                                           Mu1Types = [0],
                                                           Mu1Trigs = [],
                                                           Mu1IsoCuts = {}, #ptcone20<10 GeV, etcone40<20 GeV
                                                           Mu2PtMin = 2.*Units.GeV,
                                                           Mu2AbsEtaMax = 999.,
                                                           Mu2Types = [0],
                                                           Mu2Trigs = [],
                                                           Mu2IsoCuts = {},
                                                           UseTrackProbe = False, # bool
                                                           TrackContainerKey = 'InDetTrackParticles', # str
                                                           OppositeCharge = True,
                                                           InvariantMassLow = 7.0*Units.GeV,
                                                           InvariantMassHigh = 13.*Units.GeV,
                                                           IDTrackThinningConeSize = 0.4,
                                                           BranchPrefix = brPrefix1d
                                                           )

ToolSvc += MUON1AugmentTool1d
MUON1AugmentTools.append(MUON1AugmentTool1d)
skimmingORs.append(brPrefix1d+'DIMU_pass>0')
thinningORs.append(brPrefix1d+'DIMU_trkStatus>0')
printfunc (MUON1AugmentTool1d)

### isolation decorations
from DerivationFrameworkMuons.TrackIsolationDecorator import MUON1IDTrackDecorator as MUON1AugmentTool2a
MUON1AugmentTool2a.SelectionString = "(InDetTrackParticles.MUON1aDIMU_Status==1000||InDetTrackParticles.MUON1bDIMU_Status==1000)"
MUON1AugmentTool2a.SelectionFlag = ""
MUON1AugmentTool2a.SelectionFlagValue = 0

ToolSvc += MUON1AugmentTool2a
MUON1AugmentTools.append(MUON1AugmentTool2a)
printfunc (MUON1AugmentTool2a)

from DerivationFrameworkMuons.TrackIsolationDecorator import MUON1MSTrackDecorator as MUON1AugmentTool3
ToolSvc += MUON1AugmentTool3
MUON1AugmentTools.append(MUON1AugmentTool3)
printfunc (MUON1AugmentTool3)

#====================================================================
# SKIMMING
#====================================================================
skimming_expression = '||'.join(skimmingORs)
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
MUON1SkimmingTool1 = DerivationFramework__xAODStringSkimmingTool( name = "MUON1SkimmingTool1",
                                                                         expression = skimming_expression)
ToolSvc += MUON1SkimmingTool1

#====================================================================
# THINNING
#====================================================================
MUON1ThinningTools = []

thinning_expression = "( abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 3 )"
# keep tracks around muons
thinning_expression2 = ""
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
MUON1ThinningTool2 = DerivationFramework__MuonTrackParticleThinning(name                    = "MUON1ThinningTool2",
                                                                    StreamName              = MUON1Stream.Name,
                                                                    MuonKey                 = "Muons",
                                                                    SelectionString         = thinning_expression,
                                                                    ConeSize                = 0.4,
                                                                    InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += MUON1ThinningTool2
MUON1ThinningTools.append(MUON1ThinningTool2)

# keep tracks marked in augmentation
thinning_expression1 = '||'.join(thinningORs)
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
MUON1ThinningTool1 = DerivationFramework__TrackParticleThinning(name                    = "MUON1ThinningTool1",
                                                                StreamName              = streamName,
                                                                SelectionString         = thinning_expression1,
                                                                InDetTrackParticlesKey  = "InDetTrackParticles")
ToolSvc += MUON1ThinningTool1
MUON1ThinningTools.append(MUON1ThinningTool1)

# keep topoclusters around muons
from DerivationFrameworkCalo.DerivationFrameworkCaloConf import DerivationFramework__CaloClusterThinning
MUON1ThinningTool4 = DerivationFramework__CaloClusterThinning(name                    = "MUON1ThinningTool4",
                                                              StreamName              = MUON1Stream.Name,
                                                              SGKey                   = "Muons",
                                                              SelectionString         = "Muons.pt>4*GeV",
                                                              TopoClCollectionSGKey   = "CaloCalTopoClusters",
                                                              ConeSize                = 0.5)
ToolSvc += MUON1ThinningTool4
printfunc (MUON1ThinningTool4)
MUON1ThinningTools.append(MUON1ThinningTool4)

### also for forward tracks
thinning_expression3 = "Muons.muonType==4"
MUON1ThinningTool2f = DerivationFramework__MuonTrackParticleThinning(name                   = "MUON1ThinningTool2f",
                                                                    StreamName              = MUON1Stream.Name,
                                                                    MuonKey                 = "Muons",
                                                                    SelectionString         = thinning_expression3,
                                                                    ConeSize                = 0.5,
                                                                    InDetTrackParticlesKey  = "InDetForwardTrackParticles")
ToolSvc += MUON1ThinningTool2f
MUON1ThinningTools.append(MUON1ThinningTool2f)

### cell thinning
from DerivationFrameworkCalo.CaloCellDFGetter import thinCaloCellsForDF
thinCaloCellsForDF (inputClusterKeys = ["MuonClusterCollection"],
                    streamName = MUON1Stream.Name,
                    outputCellKey = "DFMUONCellContainer")


from DerivationFrameworkMuons import  JPsiVertexFitSetup
MUON1AugmentTools += JPsiVertexFitSetup.AddJPsiVertexingAlgs('MUON1',False)
for t in MUON1AugmentTools:
  printfunc (t)

from DerivationFrameworkBPhys.DerivationFrameworkBPhysConf import DerivationFramework__Thin_vtxTrk
MUON1Thin_vtxTrk = DerivationFramework__Thin_vtxTrk(
  name                       = "MUON1Thin_vtxTrk",
  StreamName                 = streamName,
  TrackParticleContainerName = "InDetTrackParticles",
  VertexContainerNames       = ["MUON1JpsiCandidates"],
  PassFlags                  = ["passed_Jpsi"] )

ToolSvc += MUON1Thin_vtxTrk
MUON1ThinningTools.append(MUON1Thin_vtxTrk)

if DerivationFrameworkIsMonteCarlo:

  from DerivationFrameworkMCTruth.DerivationFrameworkMCTruthConf import DerivationFramework__MenuTruthThinning
  MUON1TruthThinningTool = DerivationFramework__MenuTruthThinning(name              = "MUON1TruthThinningTool",
                                                       StreamName                   = MUON1Stream.Name,
                                                       WritePartons                 = False,
                                                       WriteHadrons                 = False,
                                                       WriteCHadrons                = False,
                                                       WriteBHadrons                = True,
                                                       WriteGeant                   = False,
                                                       WriteTauHad                  = False,
                                                       PartonPtThresh               = -1.0,
                                                       WriteBSM                     = True,
                                                       WriteBosons                  = True,
                                                       WriteBosonProducts           = False,
                                                       WriteBSMProducts             = True,
                                                       WriteTopAndDecays            = False,
                                                       WriteEverything              = False,
                                                       WriteAllLeptons              = True,
                                                       WriteLeptonsNotFromHadrons   = False,
                                                       WriteStatus3                 = False,
                                                       WriteFirstN                  = -1,
                                                       PreserveAncestors            = False,
                                                       PreserveParentsSiblingsChildren = True,
                                                       PreserveGeneratorDescendants = False,
                                                       SimBarcodeOffset             = DerivationFrameworkSimBarcodeOffset)
  ToolSvc += MUON1TruthThinningTool
  MUON1ThinningTools.append(MUON1TruthThinningTool)

  from DerivationFrameworkMuons.DerivationFrameworkMuonsConf import DerivationFramework__MuonTruthClassifierFallback
  from MCTruthClassifier.MCTruthClassifierBase import MCTruthClassifier
  MUON1MuonTruthClassifierFallback = DerivationFramework__MuonTruthClassifierFallback( name = "MUON1MuonTruthClassifierFallback",MCTruthClassifierTool = MCTruthClassifier, ContainerKey="Muons")
  ToolSvc += MUON1MuonTruthClassifierFallback
  MUON1AugmentTools.append(MUON1MuonTruthClassifierFallback)

  from DerivationFrameworkMuons.DerivationFrameworkMuonsConf import DerivationFramework__MuonTruthIsolationTool
  MUON1MuonTruthIsolationTool = DerivationFramework__MuonTruthIsolationTool( name = "MUON1MuonTruthIsolationTool",ContainerKey="Muons")
  ToolSvc += MUON1MuonTruthIsolationTool
  MUON1AugmentTools.append(MUON1MuonTruthIsolationTool)

#====================================================================
# CREATE THE DERIVATION KERNEL ALGORITHM AND PASS THE ABOVE TOOLS 
#====================================================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
MUON1Seq = CfgMgr.AthSequencer("MUON1Sequence")
DerivationFrameworkJob += MUON1Seq
MUON1Seq += CfgMgr.DerivationFramework__DerivationKernel("MUON1Kernel",
                                                                       AugmentationTools = MUON1AugmentTools,
                                                                       SkimmingTools = [MUON1SkimmingTool1],
                                                                       ThinningTools = MUON1ThinningTools
                                                                       )

#====================================================================
#       Decorate the vertices with the sum pt of their tracks
#====================================================================
alg_name = "MUONVertexDecorationAlg"
if not hasattr(DerivationFrameworkJob, alg_name ): 
    from DerivationFrameworkMuons.DerivationFrameworkMuonsConf import VertexDecoratorAlg
    from InDetTrackSelectionTool.InDetTrackSelectionToolConf import InDet__InDetTrackSelectionTool
   
    the_alg = VertexDecoratorAlg(alg_name)
    InDetTool = InDet__InDetTrackSelectionTool("TrackSelectionTool_VertexDecoration")
    InDetTool.minPt = 400
    InDetTool.CutLevel = "Loose"
    ToolSvc+= InDetTool
    the_alg.TrackSelectionTool = InDetTool
    DerivationFrameworkJob += the_alg

#====================================================================
# JetTagNonPromptLepton decorations
#====================================================================
import JetTagNonPromptLepton.JetTagNonPromptLeptonConfig as JetTagConfig
import LeptonTaggers.LeptonTaggersConfig as LepTagConfig
if not hasattr(MUON1Seq,"Muons_decoratePromptLepton"):
    JetTagConfig.ConfigureAntiKt4PV0TrackJets(MUON1Seq,"MUON1")
    MUON1Seq += JetTagConfig.GetDecoratePromptLeptonAlgs()
    MUON1Seq += LepTagConfig.GetDecorateImprovedPromptLeptonAlgs()

from DerivationFrameworkMuons import ConstituentPileupIso
ConstituentPileupIso.ConstituentPileupIso(MUON1Seq)
from DerivationFrameworkFlavourTag.FlavourTagCommon import FlavorTagInit
FlavorTagInit(JetCollections=['AntiKt4EMPFlowJets'], Sequencer=MUON1Seq)



#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkMuons.StreamConfig import MuonsDxAODStreamConfigurer as conf
if hasattr(ToolSvc,"MUON1MuonTP_Reco_mumu"):
    conf.Items['MUON1']+=["xAOD::VertexContainer#%s"        % ToolSvc.MUON1MuonTP_Reco_mumu.OutputVtxContainerName]
    conf.Items['MUON1']+=["xAOD::VertexAuxContainer#%sAux." % ToolSvc.MUON1MuonTP_Reco_mumu.OutputVtxContainerName]
    conf.Items['MUON1']+=["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % ToolSvc.MUON1MuonTP_Reco_mumu.OutputVtxContainerName]

conf.Items['MUON1']+=["CaloCellContainer#DFMUONCellContainer"]
conf.Items['MUON1']+=["CaloClusterCellLinkContainer#MuonClusterCollection_links"]
conf.extraVariables['MUON1'] += JetTagConfig.GetExtraPromptVariablesForDxAOD()
conf.Config(MUON1Stream, 'MUON1')

