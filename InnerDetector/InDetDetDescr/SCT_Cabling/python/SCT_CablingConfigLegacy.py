# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/ConfiguredFactory

from AthenaCommon import CfgMgr

def setSCT_CablingDataBase():
    from IOVDbSvc.CondDB import conddb
    from InDetRecExample.InDetJobProperties import InDetFlags

    SCTConfigurationFolderPath='/SCT/DAQ/Config/'

    #if its COMP200, use old folders...
    if (conddb.dbdata == "COMP200"):
        SCTConfigurationFolderPath='/SCT/DAQ/Configuration/'

    #...but now check if we want to override that decision with explicit flag (if there is one)
    try:
        if InDetFlags.ForceCoraCool():
            SCTConfigurationFolderPath='/SCT/DAQ/Configuration/'
    except Exception:
        pass

    try:
        if InDetFlags.ForceCoolVectorPayload():
            SCTConfigurationFolderPath='/SCT/DAQ/Config/'
    except Exception:
        pass

    try:
        if (InDetFlags.ForceCoolVectorPayload() and InDetFlags.ForceCoraCool()):
            # Setup logger
            from AthenaCommon.Logging import logging
            msg = logging.getLogger("SCT_CablingConfig")
            msg.setLevel(logging.INFO)
            msg.warning("*** SCT DB CONFIGURATION FLAG CONFLICT: Both CVP and CoraCool selected****")
            SCTConfigurationFolderPath=''
    except Exception:
        pass
        
    SCTRodConfigPath=SCTConfigurationFolderPath+'ROD'
    SCTMurConfigPath=SCTConfigurationFolderPath+'MUR'
    SCTRodMurConfigPath=SCTConfigurationFolderPath+'RODMUR'
    SCTGeogConfigPath=SCTConfigurationFolderPath+'Geog'

    if not conddb.folderRequested(SCTRodConfigPath):
        conddb.addFolderSplitMC("SCT",SCTRodConfigPath,SCTRodConfigPath, className="CondAttrListVec")
    if not conddb.folderRequested(SCTMurConfigPath):
        conddb.addFolderSplitMC("SCT", SCTMurConfigPath, SCTMurConfigPath, className="CondAttrListVec")
    if not conddb.folderRequested(SCTRodMurConfigPath):
        conddb.addFolderSplitMC("SCT",SCTRodMurConfigPath,SCTRodMurConfigPath, className="CondAttrListVec")
    if not conddb.folderRequested(SCTGeogConfigPath):
        conddb.addFolderSplitMC("SCT",SCTGeogConfigPath,SCTGeogConfigPath, className="CondAttrListVec")

    return SCTConfigurationFolderPath

def getSCT_CablingTool(name="SCT_CablingTool", **kwargs):
    return CfgMgr.SCT_CablingTool(name, **kwargs)

def getSCT_CablingCondAlgFromCoraCool(name="SCT_CablingCondAlgFromCoraCool", **kwargs):
    folderPath=setSCT_CablingDataBase()
    kwargs.setdefault("ReadKeyRod", folderPath+"ROD")
    kwargs.setdefault("ReadKeyRodMur", folderPath+"RODMUR")
    kwargs.setdefault("ReadKeyMur", folderPath+"MUR")
    kwargs.setdefault("ReadKeyGeo", folderPath+"Geog")
    return CfgMgr.SCT_CablingCondAlgFromCoraCool(name, **kwargs)
