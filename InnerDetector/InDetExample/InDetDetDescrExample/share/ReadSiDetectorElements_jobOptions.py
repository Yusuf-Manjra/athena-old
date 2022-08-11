###############################################################
#
# Loads Detector Description for Pixel and SCT
# and prints out various quanities for each detector element.
#
###############################################################


# Use Global flags and DetFlags.
from AthenaCommon.DetFlags import DetFlags
from AthenaCommon.GlobalFlags import globalflags

# Just the pixel and SCT
DetFlags.detdescr.pixel_setOn()
DetFlags.detdescr.SCT_setOn()

# MC or data - affects which conditions database instance is used
#globalflags.DataSource='geant4'
globalflags.DataSource='data'

isData = (globalflags.DataSource == 'data')

# Select the geometry version. 
globalflags.DetDescrVersion='ATLAS-R2-2016-01-00-01'

# For misalignments
from IOVDbSvc.CondDB import conddb
conddb.setGlobalTag('CONDBR2-BLKPA-2018-03')

from AtlasGeoModel.InDetGMJobProperties import InDetGeometryFlags
InDetGeometryFlags.useDynamicAlignFolders = True

# Initialize geometry
from AtlasGeoModel import GeoModelInit
from AtlasGeoModel import SetGeometryVersion

# Use MagneticFieldSvc
import MagFieldServices.SetupField

# Set up the job.
from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

# Load algorithms

from InDetDetDescrExample.InDetDetDescrExampleConf import ReadSiDetectorElements

ReadPixelElements = ReadSiDetectorElements("ReadPixelElements")
ReadPixelElements.ManagerName = "Pixel"
job += ReadPixelElements;

ReadSCTElements = ReadSiDetectorElements("ReadSCTElements")
ReadSCTElements.ManagerName = "SCT"
job += ReadSCTElements;


#
# Configure services
#
from AthenaCommon.AppMgr import ServiceMgr

#
# Pixel
#
# Load DCS service
from SiPropertiesTool.PixelSiPropertiesToolSetup import PixelSiPropertiesToolSetup
pixelSiPropertiesToolSetup = PixelSiPropertiesToolSetup()
pixelSiPropertiesToolSetup.setup()
pixelSiPropertiesTool = pixelSiPropertiesToolSetup.getTool()
from SiLorentzAngleTool.PixelLorentzAngleToolSetup import PixelLorentzAngleToolSetup
pixelLorentzAngleToolSetup = PixelLorentzAngleToolSetup()
pixelLorentzAngleTool = pixelLorentzAngleToolSetup.PixelLorentzAngleTool

ReadPixelElements.UseConditionsTools = True
ReadPixelElements.SiLorentzAngleTool = pixelLorentzAngleTool
ReadPixelElements.DetEleCollKey = "PixelDetectorElementCollection"

#
# SCT
#
# Silicon Lorentz angle tool
from SiLorentzAngleTool.SCTLorentzAngleToolSetup import SCTLorentzAngleToolSetup
sctLorentzAngleToolSetup = SCTLorentzAngleToolSetup()
sctLorentzAngleTool = sctLorentzAngleToolSetup.SCTLorentzAngleTool
sctSiliconConditionsTool = sctLorentzAngleToolSetup.sctSiliconConditionsTool

# Silicon properties tool
from SiPropertiesTool.SCT_SiPropertiesToolSetup import SCT_SiPropertiesToolSetup
sct_SiPropertiesToolSetup = SCT_SiPropertiesToolSetup()
sct_SiPropertiesToolSetup.setSiliconTool(sctSiliconConditionsTool)
sct_SiPropertiesToolSetup.setup()
sctSiPropertiesTool = sct_SiPropertiesToolSetup.getTool()

ReadSCTElements.UseConditionsTools = True
ReadSCTElements.SiLorentzAngleTool = sctLorentzAngleTool
ReadSCTElements.DetEleCollKey = "SCT_DetectorElementCollection"

print ReadPixelElements
print pixelLorentzAngleTool
print pixelSiPropertiesTool

print ReadSCTElements
print sctLorentzAngleTool
print sctSiPropertiesTool


#--------------------------------------------------------------
# Set output level threshold (DEBUG, INFO, WARNING, ERROR, FATAL)
#--------------------------------------------------------------
job.ReadPixelElements.OutputLevel = INFO
job.ReadSCTElements.OutputLevel = INFO

# To set global output level use 
# athena -l DEBUG

# Number of events to be processed (default is 10)
theApp.EvtMax = 1

# MC run number and timestamp for conditions data
# Simulation/RunDependentSim/RunDependentSimComps/python/RunDMCFlags.py
import AthenaCommon.AtlasUnixGeneratorJob
ServiceMgr.EventSelector.RunNumber = 330470
ServiceMgr.EventSelector.InitialTimeStamp  = 1500835236 # LB310 of Run330470
ServiceMgr.EventSelector.TimeStampInterval = 0

# ReadSiDetectorElement properties
#
# ManagerName:      Manager name.
# LoopOverElements: Set false to avoid loop over all elements
#
###############################################################
