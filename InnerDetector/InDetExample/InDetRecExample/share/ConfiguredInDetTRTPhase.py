# Blocking the include for after first inclusion
include.block ('InDetRecExample/ConfiguredInDetTRTPhase.py')

# --------------------------------------------------------------------------------
#
# --- TRT phase calculation
#
# --------------------------------------------------------------------------------

class ConfiguredInDetTRTPhase:

    def __init__(self, InputTrackCollections = [], TRT_Segments_EC = None):
        
        from InDetRecExample.InDetJobProperties import InDetFlags
        from AthenaCommon.DetFlags import DetFlags
        from InDetRecExample.InDetKeys import InDetKeys
        #
        # get ToolSvc and topSequence
        #
        from AthenaCommon.AppMgr import ToolSvc
        from AthenaCommon.AlgSequence import AlgSequence
        topSequence = AlgSequence()
        
        if InDetFlags.doPRDFormation() and DetFlags.makeRIO.TRT_on():

            #    
            # --- calculation of the event phase from all 3 input collections
            #

            useNewEP = True
            if globalflags.DataSource == 'data':
                if InDetFlags.doCosmics():
                    globalOffset = 8
                else:
                    globalOffset = 0
            else:
                globalOffset = -3.125
            cutWindowCenter  = -8.5
            numberIterations = 5
            cutWindowSize    = 7
            # CalDb tool
            from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbTool
            InDetTRTCalDbTool = TRT_CalDbTool(name = "TRT_CalDbTool")

                                                    
            #    
            # --- load tool
            #
            from InDetCosmicsEventPhase.InDetCosmicsEventPhaseConf import InDet__InDetCosmicsEventPhaseTool
            InDetCosmicsEventPhaseTool = InDet__InDetCosmicsEventPhaseTool(name              = "InDetCosmicsEventPhaseTool",
                                                                           UseNewEP          = useNewEP,
                                                                           GlobalOffset      = globalOffset,
                                                                           TRTCalDbTool      = InDetTRTCalDbTool
                                                                           )
                                                                           
            ToolSvc += InDetCosmicsEventPhaseTool
            if(InDetFlags.doPrintConfigurables()):
                printfunc (InDetCosmicsEventPhaseTool)
            
            from InDetCosmicsEventPhase.InDetCosmicsEventPhaseConf import InDet__InDetFixedWindowTrackTimeTool
            InDetFixedWindowTrackTimeTool = InDet__InDetFixedWindowTrackTimeTool(name              = "InDetFixedWindowTrackTimeTool",
                                                                                 UseNewEP          = useNewEP,
                                                                                 GlobalOffset      = globalOffset,
                                                                                 WindowCenter      = cutWindowCenter,
                                                                                 WindowSize        = cutWindowSize,
                                                                                 TRTCalDbTool      = InDetTRTCalDbTool
                                                                                 )
                                                                           
            ToolSvc += InDetFixedWindowTrackTimeTool
            if(InDetFlags.doPrintConfigurables()):
                printfunc (InDetFixedWindowTrackTimeTool)
            
            
            from InDetCosmicsEventPhase.InDetCosmicsEventPhaseConf import InDet__InDetSlidingWindowTrackTimeTool
            InDetSlidingWindowTrackTimeTool = InDet__InDetSlidingWindowTrackTimeTool(name              = "InDetSlidingWindowTrackTimeTool",
                                                                                     UseNewEP          = useNewEP,
                                                                                     GlobalOffset      = globalOffset,
                                                                                     NumberIterations  = numberIterations,
                                                                                     WindowSize        = cutWindowSize,
                                                                                     TRTCalDbTool      = InDetTRTCalDbTool
                                                                                     )
                                                                           
            ToolSvc += InDetSlidingWindowTrackTimeTool
            if(InDetFlags.doPrintConfigurables()):
                printfunc (InDetSlidingWindowTrackTimeTool)

            #
            # --- load algorithm
            #
            from InDetCosmicsEventPhase.InDetCosmicsEventPhaseConf import InDet__InDetCosmicsEventPhase
            InDetCosmicsEventPhase = InDet__InDetCosmicsEventPhase(name              = "InDetCosmicsEventPhase",
                                                                   InputTracksNames  = InputTrackCollections,
                                                                   TrackSummaryTool  = InDetTrackSummaryTool,
                                                                   TRTCalDbTool      = InDetTRTCalDbTool,
                                                                   #EventPhaseTool    = InDetCosmicsEventPhaseTool)
                                                                   #EventPhaseTool    = InDetFixedWindowTrackTimeTool)
                                                                   EventPhaseTool    = InDetSlidingWindowTrackTimeTool)
            if InDetFlags.doCosmics():
                InDetCosmicsEventPhase.EventPhaseTool=InDetCosmicsEventPhaseTool
                topSequence += InDetCosmicsEventPhase  # indented here: do not calculate the phase in collisions 
            if (InDetFlags.doPrintConfigurables()):
                printfunc (InDetCosmicsEventPhase)
    
