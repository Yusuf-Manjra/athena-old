###############################################################
#
# Job options file to test conditions parameters tool
#
#==============================================================

# Setup logger
from AthenaCommon.Logging import logging
msg = logging.getLogger("testParameters")
msg.setLevel(logging.INFO)

#--------------------------------------------------------------
# Standard includes
#--------------------------------------------------------------
import AthenaCommon.AtlasUnixStandardJob

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

#--------------------------------------------------------------
# use auditors
#--------------------------------------------------------------
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
globalflags.DetDescrVersion="ATLAS-R1-2012-03-00-00"
globalflags.ConditionsTag="COMCOND-BLKPA-RUN1-09"
globalflags.DetGeo="atlas"
globalflags.InputFormat="pool"
globalflags.DataSource="data"
msg.info(globalflags)

from RecExConfig.RecFlags import rec
rec.projectName.set_Value_and_Lock("data12_8TeV")

#--------------------------------------------------------------
# Load IOVDbSvc
#--------------------------------------------------------------
IOVDbSvc = Service("IOVDbSvc")
from IOVDbSvc.CondDB import conddb
conddb.dbdata="COMP200"

if conddb.dbdata=="COMP200":
    from AtlasGeoModel.InDetGMJobProperties import InDetGeometryFlags
    InDetGeometryFlags.useDynamicAlignFolders = False

IOVDbSvc.GlobalTag=globalflags.ConditionsTag()
IOVDbSvc.OutputLevel = 3

#--------------------------------------------------------------
# Set Detector setup
#--------------------------------------------------------------
# --- switch on InnerDetector
from AthenaCommon.DetFlags import DetFlags
DetFlags.detdescr.SCT_setOn()
DetFlags.ID_setOff()
DetFlags.Calo_setOff()
DetFlags.Muon_setOff()
DetFlags.Truth_setOff()
DetFlags.LVL1_setOff()
DetFlags.SCT_setOn()

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

# Set up SCT cabling
runNumber=215643
rec.RunNumber.set_Value_and_Lock(runNumber)
from AthenaCommon.Include import include
include('InDetRecExample/InDetRecCabling.py')

#--------------------------------------------------------------
# Event selector settings. Use McEventSelector
#--------------------------------------------------------------
import AthenaCommon.AtlasUnixGeneratorJob
ServiceMgr.EventSelector.RunNumber = runNumber
# initial time stamp - this is number of seconds since 1st Jan 1970 GMT
ServiceMgr.EventSelector.InitialTimeStamp  = 1354748400 # LB 469 of run 215643, 2012-12-05 @ 11:00pm (UTC)
ServiceMgr.EventSelector.TimeStampInterval = 180 # increment of 3 minutes

theApp.EvtMax = 20

#--------------------------------------------------------------
# Set output lvl (VERBOSE, DEBUG, INFO, WARNING, ERROR, FATAL)
#--------------------------------------------------------------
ServiceMgr.MessageSvc.OutputLevel = 3

#--------------------------------------------------------------
# Load DCSConditions Alg and Service
#--------------------------------------------------------------
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from SCT_ConditionsTools.SCT_ConditionsParameterToolSetup import SCT_ConditionsParameterToolSetup
sct_ConditionsParameterToolSetup = SCT_ConditionsParameterToolSetup()
sct_ConditionsParameterToolSetup.setup()

from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConf import SCT_ConditionsParameterTestAlg
topSequence+= SCT_ConditionsParameterTestAlg(SCT_ConditionsParameterTool=sct_ConditionsParameterToolSetup.getTool())

if numThreads >= 2:
    from SCT_ConditionsAlgorithms.SCTCondAlgCardinality import sctCondAlgCardinality
    sctCondAlgCardinality.set(numThreads)
    topSequence.SCT_ConditionsParameterTestAlg.Cardinality = numThreads
