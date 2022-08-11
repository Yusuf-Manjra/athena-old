from __future__ import print_function
###############################################################
#
# Skeleton top job options for ESD->AOD
# Put here outputs that require rec.doAOD=True
#
# New version for revamped job transforms
#
#
#==============================================================

# Common job options disable most RecExCommon by default. Re-enable below on demand.
include("RecJobTransforms/CommonRecoSkeletonJobOptions.py")
rec.doAOD=True

#from AthenaCommon.Logging import logging
import logging
recoLog = logging.getLogger('esd_to_aod')
recoLog.info( '****************** STARTING ESD->AOD MAKING *****************' )

from AthenaCommon.AppMgr import ServiceMgr; import AthenaPoolCnvSvc.AthenaPool
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AthenaConfiguration.AllConfigFlags import ConfigFlags

## Input
if hasattr(runArgs,"inputFile"):
    athenaCommonFlags.FilesInput.set_Value_and_Lock( runArgs.inputFile )
    ConfigFlags.Input.Files = athenaCommonFlags.FilesInput()
if hasattr(runArgs,"inputESDFile"):
    globalflags.InputFormat.set_Value_and_Lock('pool')
    rec.readESD.set_Value_and_Lock( True )
    rec.readRDO.set_Value_and_Lock( False )
    athenaCommonFlags.PoolESDInput.set_Value_and_Lock( runArgs.inputESDFile )
    ConfigFlags.Input.Files = athenaCommonFlags.PoolESDInput()

## Pre-exec
if hasattr(runArgs,"preExec"):
    recoLog.info("transform pre-exec")
    for cmd in runArgs.preExec:
        recoLog.info(cmd)
        exec(cmd)

## Pre-include
if hasattr(runArgs,"preInclude"): 
    for fragment in runArgs.preInclude:
        print("preInclude",fragment)
        include(fragment)

## Outputs
if hasattr(runArgs,"outputAODFile"):
    rec.doAOD.set_Value_and_Lock( True )
    rec.doWriteAOD.set_Value_and_Lock( True ) 
    athenaCommonFlags.PoolAODOutput.set_Value_and_Lock( runArgs.outputAODFile )
    # Lock DQ configuration to prevent downstream override
    # RB 15/12/2020: This logic was added in !36737, not sure if still needed
    from AthenaMonitoring.DQMonFlags import DQMonFlags
    print('DQMonFlags.useTrigger override')
    DQMonFlags.useTrigger.set_Value_and_Lock(rec.doTrigger() and DQMonFlags.useTrigger())

if hasattr(runArgs,"outputTAGFile"):
    # should be used as outputTAGFile_e2a=myTAG.root so that it does not trigger AODtoTAG
    # if writing TAG file, need AOD object in any case
    rec.doAOD.set_Value_and_Lock( True )
    rec.doWriteTAG.set_Value_and_Lock( True )
    athenaCommonFlags.PoolTAGOutput.set_Value_and_Lock( runArgs.outputTAGFile )

if hasattr(runArgs,"tmpAOD"):
    rec.doAOD.set_Value_and_Lock( True )
    rec.doWriteAOD.set_Value_and_Lock( True ) 
    athenaCommonFlags.PoolAODOutput.set_Value_and_Lock( runArgs.tmpAOD )

if hasattr(runArgs,"outputHIST_AOD_INTFile"):
    rec.doMonitoring.set_Value_and_Lock(True)
    DQMonFlags.histogramFile.set_Value_and_Lock( runArgs.outputHIST_AOD_INTFile )

if hasattr(runArgs,"outputNTUP_BTAGFile"):
    from BTagging.BTaggingFlags import BTaggingFlags
    BTaggingFlags.doJetTagNtuple = True
    BTaggingFlags.JetTagNtupleName = runArgs.outputNTUP_BTAGFile

if hasattr(runArgs,"outputHIST_PHYSVALMONFile"):
    rec.doPhysValMonHists=True
    
    ## Setup the output file(s):
    from GaudiSvc.GaudiSvcConf import THistSvc
    svcMgr += THistSvc()
    output=svcMgr.THistSvc.Output
    svcMgr.THistSvc.Output+= ["PhysValMon DATAFILE='"+runArgs.outputHIST_PHYSVALMONFile+"' OPT='RECREATE'"]
    # now done in RecExCommon_topOption to ensure the right ordering of algs.
    # include("PhysValMon/PhysValMon_RecoOpt.py")
    
if hasattr(runArgs, 'outputXML_JiveXMLFile'):
    jp.Rec.doJiveXML.set_Value_and_Lock(True)

rec.OutputFileNameForRecoStep="ESDtoAOD"

#========================================================
# Central topOptions (this is one is a string not a list)
#========================================================
if hasattr(runArgs,"topOptions"): include(runArgs.topOptions)
else: include( "RecExCommon/RecExCommon_topOptions.py" )

# Remove unwanted back navigation to ESD when ESD is temporary
if hasattr(runArgs,"outputAODFile"):
    if hasattr(runArgs,"ESDFileIO") and runArgs.ESDFileIO == "temporary":
        try:
            StreamAOD.ExtendProvenanceRecord = False
        except:
            recoLog.info("StreamAOD was not defined, cannot set ExtendProvenanceRecord = False. Check your flags.")

#D3PDMaker outputs
if hasattr(runArgs,"outputNTUP_MINBIASFile"):
    from D3PDMakerConfig.D3PDProdFlags import prodFlags
    prodFlags.WriteMinBiasD3PD.FileName = runArgs.outputNTUP_MINBIASFile
    prodFlags.WriteMinBiasD3PD.set_Value_and_Lock( True )
    include( prodFlags.WriteMinBiasD3PD.DPDMakerScript )
    pass

if hasattr(runArgs,"outputDESDM_BEAMSPOTFile"):
    #needs to be used with: preInclude=InDetBeamSpotFinder/BeamSpotRecoPreInclude_standard.py
    from InDetBeamSpotFinder import BeamSpotDPDFlags 
    primDPD.WriteDESDM_BEAMSPOTStream.FileName=runArgs.outputDESDM_BEAMSPOTFile
    primDPD.WriteDESDM_BEAMSPOTStream.set_Value_and_Lock( True )
    include("InDetBeamSpotFinder/DESDM_BEAMSPOTFragment.py")

#==========================================================
# Use ZLIB for compression of all temporary outputs
#==========================================================
from AthenaPoolCnvSvc import PoolAttributeHelper as pah
if hasattr(runArgs, "outputAODFile") and (runArgs.outputAODFile.endswith('_000') or runArgs.outputAODFile.startswith('tmp.')):
    ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ pah.setFileCompAlg( athenaCommonFlags.PoolAODOutput(), 1 ) ]
    ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ pah.setFileCompLvl( athenaCommonFlags.PoolAODOutput(), 1 ) ]

## Post-include
if hasattr(runArgs,"postInclude"): 
    for fragment in runArgs.postInclude:
        include(fragment)

## Post-exec
if hasattr(runArgs,"postExec"):
    recoLog.info("transform post-exec")
    for cmd in runArgs.postExec:
        recoLog.info(cmd)
        exec(cmd)
