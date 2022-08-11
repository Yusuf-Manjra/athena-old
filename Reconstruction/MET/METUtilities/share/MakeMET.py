include( "PerfMonGPerfTools/DisablePerfMon_jobOFragment.py" ) #disable performance monitoring when running in MT mode for now

import AthenaPoolCnvSvc.ReadAthenaPool
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AthenaCommon.AppMgr import ServiceMgr
from AthenaCommon import CfgMgr

from AthenaCommon.ConcurrencyFlags import jobproperties as jp
nThreads = jp.ConcurrencyFlags.NumThreads()
nProc = jp.ConcurrencyFlags.NumProcs()


from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

if nThreads>=1:
	from AthenaCommon.AlgScheduler import AlgScheduler
	AlgScheduler.OutputLevel( INFO )
	AlgScheduler.ShowControlFlow( True )
	AlgScheduler.ShowDataDependencies( True )
	AlgScheduler.setDataLoaderAlg( 'SGInputLoader' )
	from SGComps.SGCompsConf import SGInputLoader
	topSequence+=SGInputLoader(OutputLevel=DEBUG, ShowEventDump=False)

from glob import glob
filelist = ["DAOD_JETM2.test.TruthAssoc.pool.root"]
#filelist = ["/afs/cern.ch/work/r/rsmith/public/METUtilities_testfiles/valid1.110401.PowhegPythia_P2012_ttbar_nonallhad.recon.AOD.e3099_s1982_s1964_r6006_tid04628718_00/AOD.04628718._000158.pool.root.1"]
ServiceMgr.EventSelector.InputCollections = filelist

from METUtilities.METMakerConfig import getMETMakerAlg
metAlg = getMETMakerAlg('AntiKt4EMTopo','Default')
metAlg.METName = 'MET_Reco_AntiKt4EMTopo'

metAlg_truth = getMETMakerAlg('Truth_AntiKt4EMTopo',20e3)
metAlg_truth.METSoftClName = 'SoftTruthAll'
metAlg_truth.METSoftTrkName = 'SoftTruthCharged'
metAlg_truth.METName = 'MET_Truth_AntiKt4EMTopo'

topSequence += metAlg
topSequence += metAlg_truth

write_xAOD = False
if write_xAOD:
    from OutputStreamAthenaPool.MultipleStreamManager import MSMgr
    xaodStream = MSMgr.NewPoolRootStream( "StreamXAOD", "xAOD.METMaker.pool.root" )
    #
    xaodStream.AddItem('xAOD::MissingETContainer#MET_Reco_AntiKt4EMTopo')
    xaodStream.AddItem('xAOD::MissingETAuxContainer#MET_Reco_AntiKt4EMTopoAux.')
    #
    xaodStream.AddItem('xAOD::MissingETContainer#MET_Truth_AntiKt4EMTopo')
    xaodStream.AddItem('xAOD::MissingETAuxContainer#MET_Truth_AntiKt4EMTopoAux.')
    #
    xaodStream.AddItem('xAOD::MissingETContainer#MET_Truth')
    xaodStream.AddItem('xAOD::MissingETAuxContainer#MET_TruthAux.')
    #
    xaodStream.Print()

# from OutputStreamAthenaPool.MultipleStreamManager import MSMgr
# HistoXAODStream = MSMgr.NewRootStream( streamName, fileName )

#from Valkyrie.JobOptCfg import ValgrindSvc
#svcMgr += ValgrindSvc( OutputLevel = DEBUG,
#                       ProfiledAlgs = ["METMakerAlg"],
#                       ProfiledIntervals = ["METMakerAlg.execute"])

from PerfMonComps.PerfMonFlags import jobproperties as pmon_properties
pmon_properties.PerfMonFlags.doSemiDetailedMonitoring=True

ServiceMgr.MessageSvc.defaultLimit = 9999999
theApp.EvtMax = -1
ServiceMgr.EventSelector.SkipEvents = 0

printint = max(theApp.EvtMax/250,1) if theApp.EvtMax>0 else 1000
svcMgr += CfgMgr.AthenaEventLoopMgr(EventPrintoutInterval=printint)

#  set algCardinality = 1 to disable cloning for algs with thread unsafe features
"""
algCardinality = nThreads
if (algCardinality > 1):
	for alg in topSequence:
		name = alg.name()
		print " -> looking at ",name
		if (name==metAlg.name()):
			alg.Cardinality = 1
			print " -> suppressing cloning for ", name
		else:
			alg.Cardinality = algCardinality			
"""