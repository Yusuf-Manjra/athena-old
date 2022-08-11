# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

class PixelSiPropertiesToolSetup:
  "Class to simplify setup of PixelSiPropertiesTool and required conditions algorithm"

  def __init__(self):
    self.algName = "PixelSiPropertiesCondAlg"
    self.alg = None
    self.toolName = "PixelSiPropertiesTool"
    self.tool = None

  def getAlgName(self):
    return self.algName

  def setAlgName(self, algName):
    self.algName = algName

  def getAlg(self):
    return self.alg

  def getToolName(self):
    return self.toolName

  def setToolName(self, toolName):
    self.toolName = toolName

  def getTool(self):
    return self.tool

  def setAlg(self):
    hvFolder = "/PIXEL/DCS/HV"
    tempFolder = "/PIXEL/DCS/TEMPERATURE"
    dbInstance = "DCS_OFL"

    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
    if athenaCommonFlags.isOnline():
      hvFolder = "/PIXEL/HLT/DCS/HV"
      tempFolder = "/PIXEL/HLT/DCS/TEMPERATURE"
      dbInstance = "PIXEL_ONL"

    from IOVDbSvc.CondDB import conddb
    if not conddb.folderRequested(hvFolder):
      conddb.addFolder(dbInstance, hvFolder, className="CondAttrListCollection")
    if not conddb.folderRequested(tempFolder):
      conddb.addFolder(dbInstance, tempFolder, className="CondAttrListCollection")

    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")
 
    if not hasattr(condSeq,"PixelDCSCondHVAlg"):
      from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelDCSCondHVAlg
      condSeq += PixelDCSCondHVAlg(name="PixelDCSCondHVAlg", ReadKey=hvFolder)

    if not hasattr(condSeq,"PixelDCSCondTempAlg"):
      from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelDCSCondTempAlg
      condSeq += PixelDCSCondTempAlg(name="PixelDCSCondTempAlg", ReadKey=tempFolder)

    if not hasattr(condSeq, "PixelSiPropertiesCondAlg"):
      from SiPropertiesTool.SiPropertiesToolConf import PixelSiPropertiesCondAlg
      condSeq += PixelSiPropertiesCondAlg(name="PixelSiPropertiesCondAlg")
    self.alg = getattr(condSeq, "PixelSiPropertiesCondAlg")

  def setTool(self):
    from AthenaCommon.AppMgr import ToolSvc
    if not hasattr(ToolSvc, "PixelSiPropertiesTool"):
      from SiPropertiesTool.SiPropertiesToolConf import SiPropertiesTool
      ToolSvc += SiPropertiesTool(name = "PixelSiPropertiesTool",
                                  DetectorName = "Pixel",
                                  ReadKey = "PixelSiliconPropertiesVector")
    self.tool = getattr(ToolSvc, "PixelSiPropertiesTool")

  def setup(self):
    self.setAlg()
    self.setTool()
