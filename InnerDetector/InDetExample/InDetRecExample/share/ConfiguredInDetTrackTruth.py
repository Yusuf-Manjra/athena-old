# Blocking the include for after first inclusion of the TrackTruth module
include.block ('InDetRecExample/ConfiguredInDetTrackTruth.py')

# -------------------------------------------------------------------------
#
# ------- fragment to handle track truth association
#
# -------------------------------------------------------------------------
from InDetRecExample.InDetKeys          import InDetKeys
class  ConfiguredInDetTrackTruth:
    

    def __init__(self, Tracks = None, DetailedTruth = None, TracksTruth = None, PixelClustersTruth = InDetKeys.PixelClustersTruth(), SCT_ClustersTruth = InDetKeys.SCT_ClustersTruth(), TRT_DriftCirclesTruth = InDetKeys.TRT_DriftCirclesTruth()):

        from InDetRecExample.InDetJobProperties import InDetFlags
        from AthenaCommon.DetFlags              import DetFlags
        from InDetRecExample.InDetKeys          import InDetKeys
        #
        # get ToolSvc and topSequence
        #
        from AthenaCommon.AppMgr                import ToolSvc
        from AthenaCommon.AlgSequence           import AlgSequence
        from RecExConfig.hideInput              import hideInput
        topSequence = AlgSequence()
        #
        # --- Enable the detailed track truth
        #
        from InDetTruthAlgs.InDetTruthAlgsConf import InDet__InDetDetailedTrackTruthMaker
        hideInput ('DetailedTrackTruthCollection', DetailedTruth)
        DetailedTruthMaker = InDet__InDetDetailedTrackTruthMaker(name                   = DetailedTruth+"Maker",
                                                                 TrackCollectionName    = Tracks,
                                                                 DetailedTrackTruthName = DetailedTruth,
                                                                 TruthNamePixel         = PixelClustersTruth,
                                                                 TruthNameSCT           = SCT_ClustersTruth,
                                                                 TruthNameTRT           = TRT_DriftCirclesTruth)
            
        # this is how the truth maker gets to know which detector is on ...
        if (not DetFlags.haveRIO.pixel_on()):
            DetailedTruthMaker.TruthNamePixel = ""
        if (not DetFlags.haveRIO.SCT_on()):
            DetailedTruthMaker.TruthNameSCT = ""
        # for cosmics, at the stage of SiPatternRecognition, the TRT truth information is not yet available
        if ((not DetFlags.haveRIO.TRT_on()) or
            (InDetFlags.doCosmics() and (DetailedTruth == "SiSPSeededTracksDetailedTruth" or DetailedTruth == "ResolvedTracksDetailedTruth"))):
            DetailedTruthMaker.TruthNameTRT = ""

        #if Tracks == "Tracks":
        #    DetailedTruthMaker.OutputLevel = VERBOSE
        topSequence += DetailedTruthMaker
        if (InDetFlags.doPrintConfigurables()):
            printfunc (DetailedTruthMaker        )
        #
        # --- Detailed to old TrackTruth
        #
        if InDetFlags.truthMatchStrategy() == 'TruthMatchRatio':
            from TrkTruthCreatorTools.TrkTruthCreatorToolsConf import Trk__TruthMatchRatio as InDetTruthMatchTool
        elif InDetFlags.truthMatchStrategy() == 'TruthMatchTanimoto':
            from TrkTruthCreatorTools.TrkTruthCreatorToolsConf import Trk__TruthMatchTanimoto as InDetTruthMatchTool
        else:
            printfunc ("ConfiguredInDetTrackTruth: error! InDetFlags.truthMatchStrategy must be TruthMatchRatio or TruthMatchTanimoto but is: "+InDetFlags.truthMatchStrategy())

        InDetTruthMatchSimilarityTool = InDetTruthMatchTool(name        = "InDetTruthMatchTool",
                                                            WeightPixel =  10.,
                                                            WeightSCT   =   5.,
                                                            WeightTRT   =   1.)
        
        # --- only add this once !!!
        if not hasattr(ToolSvc, "InDetTruthMatchTool"):
            ToolSvc += InDetTruthMatchSimilarityTool
            if (InDetFlags.doPrintConfigurables()):
                printfunc (InDetTruthMatchSimilarityTool)

        from TrkTruthAlgs.TrkTruthAlgsConf import TrackTruthSimilaritySelector
        InDetTruthSimilaritySelector = TrackTruthSimilaritySelector(name                     = TracksTruth+"Selector",
                                                                    DetailedTrackTruthName   = DetailedTruth,
                                                                    OutputName               = TracksTruth,
                                                                    TrackTruthSimilarityTool = InDetTruthMatchSimilarityTool)
        #if Tracks == "Tracks":
        #    InDetTruthSimilaritySelector.OutputLevel = VERBOSE
        topSequence += InDetTruthSimilaritySelector
        if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetTruthSimilaritySelector)

        # --- remember imput
        self.__Tracks        = Tracks
        self.__TracksTruth   = TracksTruth
        self.__DetailedTruth = DetailedTruth
        
            
    def Tracks (self):
        return self.__Tracks

    def TracksTruth (self):
        return self.__TracksTruth

    def DetailedTruth (self):
        return self.__DetailedTruth
