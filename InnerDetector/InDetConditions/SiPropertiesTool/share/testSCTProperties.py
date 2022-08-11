import AthenaCommon.AtlasUnixStandardJob

# Setup logger
from AthenaCommon.Logging import logging
msg = logging.getLogger("testSCTProperties")
msg.setLevel(logging.INFO)

#--------------------------------------------------------------
# Thread-specific setup
#--------------------------------------------------------------
from AthenaCommon.ConcurrencyFlags import jobproperties
numThreads = jobproperties.ConcurrencyFlags.NumThreads()
if numThreads > 0:
    from AthenaCommon.AlgScheduler import AlgScheduler
    AlgScheduler.CheckDependencies( True )
    AlgScheduler.ShowControlFlow( True )
    AlgScheduler.ShowDataDependencies( True )

# use auditors
from AthenaCommon.AppMgr import ServiceMgr
from GaudiCommonSvc.GaudiCommonSvcConf import AuditorSvc
ServiceMgr += AuditorSvc()
theAuditorSvc = ServiceMgr.AuditorSvc
theAuditorSvc.Auditors  += [ "ChronoAuditor"]
theAuditorSvc.Auditors  += [ "MemStatAuditor" ]
theApp.AuditAlgorithms=True

#--------------------------------------------------------------
# Load Geometry
#--------------------------------------------------------------
from AthenaCommon.GlobalFlags import globalflags
globalflags.DetDescrVersion="ATLAS-R2-2015-03-01-00"
globalflags.DetGeo="atlas"
globalflags.InputFormat="pool"
globalflags.DataSource="geant4"
msg.info(globalflags)

#--------------------------------------------------------------
# Set Detector setup
#--------------------------------------------------------------
# --- switch on InnerDetector
from AthenaCommon.DetFlags import DetFlags 
DetFlags.detdescr.SCT_setOn()
DetFlags.ID_setOn()
DetFlags.Calo_setOff()
DetFlags.Muon_setOff()
DetFlags.Truth_setOff()
DetFlags.LVL1_setOff()
DetFlags.SCT_setOn()
DetFlags.TRT_setOff()

# ---- switch parts of ID off/on as follows
#switch off tasks
DetFlags.pileup.all_setOff()
DetFlags.simulate.all_setOff()
DetFlags.makeRIO.all_setOff()
DetFlags.writeBS.all_setOff()
DetFlags.readRDOBS.all_setOff()
DetFlags.readRIOBS.all_setOff()
DetFlags.readRIOPool.all_setOff()
DetFlags.writeRIOPool.all_setOff()

import AtlasGeoModel.SetGeometryVersion
import AtlasGeoModel.GeoModelInit

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

#--------------------------------------------------------------
# Load IOVDbSvc
#--------------------------------------------------------------
IOVDbSvc = Service("IOVDbSvc")
from IOVDbSvc.CondDB import conddb
IOVDbSvc.GlobalTag="OFLCOND-MC16-SDR-18"
IOVDbSvc.OutputLevel = 3

# Setup the DCS folders and tool used in the sctSiliconConditionsTool
from SCT_ConditionsTools.SCT_DCSConditionsToolSetup import SCT_DCSConditionsToolSetup
sct_DCSConditionsToolSetup = SCT_DCSConditionsToolSetup()
sct_DCSConditionsToolSetup.setup()

# Set up SCT_SiliconConditionsTool
from SCT_ConditionsTools.SCT_SiliconConditionsToolSetup import SCT_SiliconConditionsToolSetup
sct_SiliconConditionsToolSetup = SCT_SiliconConditionsToolSetup()
sct_SiliconConditionsToolSetup.setDcsTool(sct_DCSConditionsToolSetup.getTool())
sct_SiliconConditionsToolSetup.setup()

# Set up SCT_SiPropertiesTool
from SiPropertiesTool.SCT_SiPropertiesToolSetup import SCT_SiPropertiesToolSetup
sct_SiPropertiesToolSetup = SCT_SiPropertiesToolSetup()
sct_SiPropertiesToolSetup.setSiliconTool(sct_SiliconConditionsToolSetup.getTool())
sct_SiPropertiesToolSetup.setup()

# Set up SCTSiPropertiesTestAlg
from SiPropertiesTool.SiPropertiesToolConf import SCTSiPropertiesTestAlg
job += SCTSiPropertiesTestAlg(SCTPropertiesTool=sct_SiPropertiesToolSetup.getTool())

if numThreads >= 2:
    from SCT_ConditionsAlgorithms.SCTCondAlgCardinality import sctCondAlgCardinality
    sctCondAlgCardinality.set(numThreads)
    job.SCTSiPropertiesTestAlg.Cardinality = numThreads

# Prepare EventSelector
import AthenaCommon.AtlasUnixGeneratorJob

ServiceMgr.EventSelector.RunNumber  = 300000 # MC16c 2017 run number
ServiceMgr.EventSelector.InitialTimeStamp  = 1500000000
ServiceMgr.EventSelector.TimeStampInterval = 0
theApp.EvtMax = 20

ServiceMgr.MessageSvc.OutputLevel = 3
