# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from JetMonitoring.JetHistoTools import jhm, selectionAndHistos
from JetMonitoring.JetMonitoringConf import JetMonitoringTool, JetKinematicHistos, JetContainerHistoFiller, JetSubStructureHistos
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
from PyUtils.MetaReader import read_metadata

input_file = svcMgr.EventSelector.InputCollections[0]
metadata = read_metadata(input_file) # opens the first file from the InputCollections list
metadata = metadata[input_file]  # promote keys stored under input_file key one level up to access them directly
# this is a dict of dicts, take a look at what's available! Below are some examples:
isMC = 'IS_SIMULATION' in metadata['eventTypes']
beam_energy = metadata['beam_energy']
conditions_tag = metadata['IOVDbGlobalTag'] #useful for figuring out which mc production this is
print ("PhysicsValidationHistos: isMC=",isMC, " beam=",beam_energy," conditions_tag=",conditions_tag)

def commonPhysValTool(container, refcontainer="", onlyKinematics = False, globalSelection= ""):
    containerfiller = JetContainerHistoFiller(container+"HistoFiller",JetContainer = container)
    filler = containerfiller
    
    if globalSelection !="":
        # here we create an intermediate selection tool, we'll pass it the histo builders
        # instead of passing them to containerfiller
        # Create the intermediate tool and attach it to containerfiller:
        containerfiller.HistoTools = [ selectionAndHistos( globalSelection,
                                                           [] , # no histo builders, will be set below
                                                           histoNameSuffix= "none", # same histo names as without global selection
                                                           )
                                       ]
        # attach the selection :
        filler = containerfiller.HistoTools[0]
        containerfiller.OutputLevel = 2        
        filler.OutputLevel = 2
    
    # Give a list of predefined tools from jhm or a combination of such tools
    filler.HistoTools = [
        # build a special tool without 2D hists :
        JetKinematicHistos("kinematics",PlotOccupancy=False, PlotAveragePt=False, PlotNJet=True) ,
    ]

    if onlyKinematics:
        # return now
        return containerfiller

    filler.HistoTools += [

        # Draw a set of histo for a particular jet selection :
        selectionAndHistos( "leadingjet" , [ "basickinematics", ] ),
        selectionAndHistos( "subleadingjet" , [ "basickinematics"] ),
        selectionAndHistos("40000<pt<50000",["pt"]),

        # distances between 2 leading jets.
        jhm.leadingjetrel,
        ]

    if ( ("Topo" in container or "PFlow" in container) and "Trimmed" not in container) and "SoftDrop" not in container:
        filler.HistoTools += [

            jhm.Width,

            # jet states
            jhm.basickinematics_emscale,
            #track variables
            jhm.tool("JVF[0]"),
            jhm.JVF,
            jhm.JVFCorr,
            jhm.Jvt,
            jhm.JvtRpt,

            # calo variables
            jhm.NegativeE,
            jhm.Timing,
            jhm.LArQuality,

        ]

    if "Topo" in container or "PFlow" in container or "UFO" in container:
        filler.HistoTools += [
            
            # jet states
            jhm.basickinematics_constscale,

            # track variables
            jhm.SumPtTrkPt500,
            jhm.tool("SumPtTrkPt500[0]"),
            jhm.tool("NumTrkPt500[0]"),
            jhm.tool("NumTrkPt1000[0]"),
            jhm.tool("TrackWidthPt1000[0]"),
            jhm.GhostTrackCount,
            jhm.GhostTruthAssociationFraction,
            jhm.GhostMuonSegmentCount,

            # calo variables
            jhm.EMFrac,
            jhm.HECFrac,

            jhm.EMB2,
            jhm.EMB3,            
            jhm.EME2,
            jhm.EME3,
            jhm.HEC2,
            jhm.HEC3,
            jhm.FCAL0,
            jhm.FCAL1,
            jhm.TileBar0,
            jhm.TileBar1,
            jhm.TileExt0,
            jhm.TileExt1
        ]
        
        if "PFlow" in container:
            filler.HistoTools += [
                jhm.tool("SumPtChargedPFOPt500[0]"),
                jhm.tool("NumChargedPFOPt500[0]"),
                jhm.tool("NumChargedPFOPt1000[0]"),
                jhm.tool("ChargedPFOWidthPt1000[0]"),
                jhm.DFCommonJets_QGTagger_NTracks,
                jhm.DFCommonJets_QGTagger_TracksWidth,
                jhm.DFCommonJets_QGTagger_TracksC1,
                jhm.DFCommonJets_fJvt,
            ]


        if refcontainer:
            # efficiency
            filler.HistoTools += [jhm.effresponse,]
            filler.HistoTools['effresponse'].RefContainer = refcontainer

        if "SoftDrop" in container:
            filler.HistoTools += [
                jhm.rg,
                jhm.zg,
            ]


        if "Trimmed" in container or "SoftDrop" in container:
            filler.HistoTools += [
               
              #main list of substrcture variables
              JetSubStructureHistos("substructure"), 

              jhm.ECF1,
              jhm.ECF2,
              jhm.ECF3,
              jhm.Tau1_wta,
              jhm.Tau2_wta,
              jhm.Tau3_wta,
              jhm.Split12,
              jhm.Split23,
              jhm.NTrimSubjets,
              jhm.DetectorEta,
              jhm.TrackSumPt,
              jhm.TrackSumMass,      
              jhm.Qw,
              jhm.PlanarFlow,
              jhm.FoxWolfram2,
              jhm.FoxWolfram0,
              jhm.Angularity,
              jhm.Aplanarity,
              jhm.KtDR,
              jhm.ZCut12,
              jhm.Dip12,
              jhm.Dip13,
              jhm.Dip23,
              jhm.DipExcl12,
              jhm.Split34,
              jhm.ZCut23,
              jhm.ZCut34,
              jhm.Mu12,
              jhm.ThrustMin,
              jhm.ThrustMaj,
              jhm.FoxWolfram1,
              jhm.FoxWolfram3,
              jhm.FoxWolfram4,
              jhm.Sphericity,
              jhm.PullMag,
              jhm.PullPhi,
              jhm.Pull_C00,
              jhm.Pull_C01,
              jhm.Pull_C10,
              jhm.Pull_C11,
              jhm.Charge,
                ]

    if 'PV0Track' in container:
        filler.HistoTools += [
            jhm.HadronConeExclTruthLabelID,
            jhm.HadronConeExclExtendedTruthLabelID,
            jhm.HadronGhostTruthLabelID,
            jhm.HadronGhostExtendedTruthLabelID,
        ]

    #filler.OutputLevel =2 
    return containerfiller

from RecExConfig.RecFlags import rec 
akt4refContainer = "AntiKt4TruthJets" if rec.doTruth() else ""

#globalSelection = "0.7<JVF[0]"
globalSelection = ""

##
# necessary updates for validation directories
###

athenaMonTool = JetMonitoringTool(HistoTools = [
    commonPhysValTool( "AntiKt4LCTopoJets", akt4refContainer ,globalSelection = globalSelection),
    commonPhysValTool( "AntiKt4EMTopoJets", akt4refContainer ,globalSelection = globalSelection),
    commonPhysValTool( "AntiKt4EMPFlowJets", akt4refContainer ,globalSelection = globalSelection ),
    commonPhysValTool( "AntiKt4TruthJets" ),
    commonPhysValTool( "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets" ),
    commonPhysValTool( "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets" ),
    commonPhysValTool( "AntiKt10TruthTrimmedPtFrac5SmallR20Jets" ),
    commonPhysValTool( "AntiKt10TruthSoftDropBeta100Zcut10Jets" ),
    commonPhysValTool( "AntiKtVR30Rmax4Rmin02PV0TrackJets"),
], IntervalType=8) # 8 == HistoGroupBase::all


if not isMC:
    athenaMonTool = JetMonitoringTool(HistoTools = [
        commonPhysValTool( "AntiKt4LCTopoJets", akt4refContainer ,globalSelection = globalSelection),
        commonPhysValTool( "AntiKt4EMTopoJets", akt4refContainer ,globalSelection = globalSelection),
        commonPhysValTool( "AntiKt4EMPFlowJets" ),
        commonPhysValTool( "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets" ),
        commonPhysValTool( "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets" ),
        commonPhysValTool( "AntiKtVR30Rmax4Rmin02PV0TrackJets"),
    ], IntervalType=8) # 8 == HistoGroupBase::all
    
#ToolSvc += athenaMonTool

def athenaPhysValTool():
    return athenaMonTool
