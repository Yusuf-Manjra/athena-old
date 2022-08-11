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

#
## Override Setup for Hive
#

from AthenaServices.AthenaServicesConf import AthenaHiveEventLoopMgr
# svcMgr.AthenaHiveEventLoopMgr.OutputLevel = INFO

from StoreGate.StoreGateConf import SG__HiveMgrSvc
# svcMgr.EventDataSvc.OutputLevel = INFO

from GaudiHive.GaudiHiveConf import AlgResourcePool
# svcMgr += AlgResourcePool( OutputLevel = INFO );

from AthenaCommon.AlgScheduler import AlgScheduler
AlgScheduler.OutputLevel( INFO )
AlgScheduler.ShowControlFlow( True )
AlgScheduler.ShowDataDependencies( True )

# ThreadPoolService thread local initialization
from GaudiHive.GaudiHiveConf import ThreadPoolSvc
svcMgr += ThreadPoolSvc("ThreadPoolSvc")
svcMgr.ThreadPoolSvc.ThreadInitTools = ["ThreadInitTool"]

#---------------------------------------------------------------------------------#

#
## Uncomment following to avoid long waits when segfaulting,
## and add "Root.Stacktrace: no" to your .rootrc file
#
# import ROOT
# ROOT.SetSignalPolicy( ROOT.kSignalFast )

#---------------------------------------------------------------------------------#


#
## AlgSequence
#

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

from AthExHive.AthExHiveConf import *
topSequence+=HiveAlgA(OutputLevel=DEBUG,Time=20)
topSequence+=HiveAlgB(OutputLevel=DEBUG,Time=10)
topSequence+=HiveAlgC(OutputLevel=DEBUG,Time=190, Key_W1="C1")
topSequence+=HiveAlgD(OutputLevel=DEBUG,Time=10)
topSequence+=HiveAlgE(OutputLevel=DEBUG,Time=30, Key_R1="C1")
topSequence+=HiveAlgG(OutputLevel=DEBUG,Time=10)
topSequence+=HiveAlgF(OutputLevel=DEBUG,Time=30)

topSequence+=HiveAlgV(OutputLevel=DEBUG,Time=30)
topSequence.HiveAlgV.Key_RV = [ "a1", "a2", "d1", "e1", "C1" ]
topSequence.HiveAlgV.Key_WV = [ "V1", "V2", "V3" ]

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
           
