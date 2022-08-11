#
#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#


# 
## can only run if Hive enabled
#
from AthenaCommon.Logging import log as msg

from AthenaCommon.ConcurrencyFlags import jobproperties as jp
nThreads = jp.ConcurrencyFlags.NumThreads()
if (nThreads < 1) :
   msg.fatal('numThreads must be >0. Did you set the --threads=N option?')
   sys.exit(AthenaCommon.ExitCodes.CONFIGURATION_ERROR)

# setup the McEventSelector
import AthenaCommon.AtlasUnixGeneratorJob
svcMgr.EventSelector.FirstEvent=0
svcMgr.EventSelector.RunNumber=1
svcMgr.EventSelector.InitialTimeStamp=0
svcMgr.EventSelector.TimeStampInterval=1
svcMgr.EventSelector.FirstLB=1

#
## Override Setup for Hive
#

from AthenaServices.AthenaServicesConf import AthenaHiveEventLoopMgr
# svcMgr.AthenaHiveEventLoopMgr.OutputLevel = INFO

from StoreGate.StoreGateConf import StoreGateSvc
# svcMgr.StoreGateSvc.Dump = True
# svcMgr += StoreGateSvc("ConditionStore", Dump=True)

# from StoreGate.StoreGateConf import StoreGateSvc
# theApp.CreateSvc += ["StoreGateSvc/ConditionStore"]


from StoreGate.StoreGateConf import SG__HiveMgrSvc
# svcMgr.EventDataSvc.OutputLevel = INFO

from GaudiHive.GaudiHiveConf import AlgResourcePool
# svcMgr += AlgResourcePool( OutputLevel = INFO );

from AthenaCommon.AlgScheduler import AlgScheduler
AlgScheduler.OutputLevel( INFO )
AlgScheduler.ShowControlFlow( True )
AlgScheduler.ShowDataDependencies( True )
AlgScheduler.EnableConditions( True )
AlgScheduler.setDataLoaderAlg( "SGInputLoader" )

from IOVSvc.IOVSvcConf import CondSvc
svcMgr += CondSvc( OutputLevel=VERBOSE )

#-----------------------------------------------------------------------------#

#
## Uncomment following to avoid long waits when segfaulting,
## and add "Root.Stacktrace: no" to your .rootrc file
#
# import ROOT
# ROOT.SetSignalPolicy( ROOT.kSignalFast )

#-----------------------------------------------------------------------------#


# from StoreGate.StoreGateConf import StoreGateSvc
# theApp.CreateSvc += ["StoreGateSvc/ConditionStore"]

#
## AlgSequence
#

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from AthExHive.AthExHiveConf import *
topSequence+=AlgA(OutputLevel=DEBUG)
topSequence+=AlgB(OutputLevel=DEBUG, Key_R1="a1", Key_W1="a3")
topSequence+=AlgC("AlgC1", OutputLevel=DEBUG, Key_R1="a2", Key_CH="X1")
topSequence+=AlgC("AlgC2", OutputLevel=DEBUG, Key_R1="a1", Key_CH="X2")
topSequence+=AlgD("AlgD1", OutputLevel=DEBUG, Key_R1="a3", Key_CH1="X1", Key_CH2="X2")
#topSequence+=AlgD("AlgD2", OutputLevel=DEBUG, Key_R1="a1", Key_CH1="Y1", Key_CH2="Y2")

from AthenaCommon.AlgSequence import AthSequencer 
condSeq = AthSequencer("AthCondSeq") 

condSeq+=CondAlgX("CondAlgX1", OutputLevel=DEBUG, Key_CH="X1", Key_DB="X1")
condSeq+=CondAlgX("CondAlgX2", OutputLevel=DEBUG, Key_CH="X2", Key_DB="X2")

condSeq+=CondAlgY("CondAlgY1", OutputLevel=DEBUG, Key_CH1="Y1", Key_CH2="Y2", Key_DB1="Y1", Key_DB2="Y2")


condDbFile = "condDb.txt"
import os
import string
for dir in (".:"+os.environ.get('DATAPATH')).split (':'):
   cdb = os.path.join(dir,condDbFile)
   if (os.path.isfile( cdb ) ) :
      found = 1
      break

if (found == 0):
   msg.fatal('ASCII condDb file \"' + condDbFile + '\" not found')
   sys.exit(AthenaCommon.ExitCodes.CONFIGURATION_ERROR)
else:
   msg.info( "using ASCIICondDb file from " + cdb )

svcMgr += ASCIICondDbSvc( OutputLevel=DEBUG, CondFile = cdb )


#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------

theApp.EvtMax = 20

nProc = jp.ConcurrencyFlags.NumProcs()
if (nProc > 0) :

   #
   ## Basic setup for MP/Hive
   #
   import AthenaCommon.AtlasUnixGeneratorJob

   #
   ## For MP/Hive we need to set the chunk size
   #

   from AthenaCommon.Logging import log as msg
   if (theApp.EvtMax == -1) : 
      msg.fatal('EvtMax must be >0 for hybrid configuration')
      sys.exit(AthenaCommon.ExitCodes.CONFIGURATION_ERROR)

   if ( theApp.EvtMax % nProc != 0 ) :
      msg.warning('EvtMax[%s] is not divisible by nProcs[%s]: MP Workers will not process all requested events',theApp.EvtMax,nProc)

   chunkSize = int (theApp.EvtMax / nProc)

   from AthenaMP.AthenaMPFlags import jobproperties as jps 
   jps.AthenaMPFlags.PreCountedEvents=theApp.EvtMax
   jps.AthenaMPFlags.ChunkSize= chunkSize

   msg.info('AthenaMP workers will process %s events each',chunkSize)



printfunc ("==========================================================================================\n")

#
## set which Algorithms can be cloned
#

#  set algCardinality = 1 to disable cloning for all Algs
algCardinality = jp.ConcurrencyFlags.NumThreads()

if (algCardinality != 1):
   for alg in topSequence:
       name = alg.name()
       alg.Cardinality = algCardinality
           
