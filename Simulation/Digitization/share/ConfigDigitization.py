from AthenaCommon.Logging import logging
logConfigDigitization = logging.getLogger( 'ConfigDigitization' )


#check job configuration
from Digitization.DigiConfigCheckers import checkDetFlagConfiguration
checkDetFlagConfiguration()

#Pool input
from AthenaCommon.AppMgr import ServiceMgr
if not hasattr(ServiceMgr, 'EventSelector'):
    import AthenaPoolCnvSvc.ReadAthenaPool
if hasattr(ServiceMgr, 'PoolSvc'):
    ServiceMgr.PoolSvc.MaxFilesOpen = 0 # Never close Input Files
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
if not athenaCommonFlags.DoFullChain:
    ServiceMgr.EventSelector.InputCollections = athenaCommonFlags.PoolHitsInput()
#Settings the following attributes reduces the job size slightly
#ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [  "TREE_BRANCH_OFFSETTAB_LEN ='100'" ]
#ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ "DEFAULT_BUFFERSIZE = '2048'" ]

#--------------------------------------------------------------
# Conditions Tag
#--------------------------------------------------------------
from Digitization.DigitizationFlags import digitizationFlags
if (digitizationFlags.IOVDbGlobalTag.statusOn and digitizationFlags.IOVDbGlobalTag.get_Value()!='default'):
    IOVDbSvc=theApp.service('IOVDbSvc')
    IOVDbSvc.GlobalTag=digitizationFlags.IOVDbGlobalTag.get_Value()
#--------------------------------------------------------------
# GeoModel
#--------------------------------------------------------------
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit
from AtlasGeoModel import SetupRecoGeometry

#--------------------------------------------------------------
# Magnetic field service
#--------------------------------------------------------------
try:
    import MagFieldServices.SetupField
except:
    #fall-back for 19.0.X releases.
    include( "BFieldAth/BFieldAth_jobOptions.py" )

#--------------------------------------------------------------
# Pileup configuration
#--------------------------------------------------------------
from Digitization.DigiConfigCheckers import syncDetFlagsAndDigitizationJobProperties
syncDetFlagsAndDigitizationJobProperties()

#--------------------------------------------------------------
# Configure Run and Lumi Block and Pile-Up Lumi overriding
# (if required)
#--------------------------------------------------------------
if digitizationFlags.RunAndLumiOverrideList.statusOn:
    if not(DetFlags.pileup.any_on()):
        AthError( "This job will try to override pile-up luminosity configuration, but no pile-up will be set up!" )
    include("Digitization/LumiBlockOverrides.py")
    if digitizationFlags.dataRunNumber.statusOn:
        logDigitization_flags.warning('digitizationFlags.RunAndLumiOverrideList has been set! digitizationFlags.dataRunNumber (set to %s) will be ignored. ', digitizationFlags.dataRunNumber.get_Value() )
else:
    include("Digitization/RunNumberOverride.py")

#-----------------------------------------------------------
# Check Beam and Digitization jobproperties are synchronised
#-----------------------------------------------------------
from Digitization.DigiConfigCheckers import syncBeamAndDigitizationJobProperties
syncBeamAndDigitizationJobProperties()

digitizationFlags.lockMostFlags()
#--------------------------------------------------------------
# More Pileup configuration
#--------------------------------------------------------------
if DetFlags.pileup.any_on() or digitizationFlags.doXingByXingPileUp():
    # protection for SteppingCache usage - currently incompatible with PileUpTools
    if digitizationFlags.SignalPatternForSteppingCache.statusOn and digitizationFlags.doXingByXingPileUp():
        raise RuntimeError("SteppingCache is incompatible with PileUpTools. Please switch off either digitizationFlags.SignalPatternForSteppingCache or digitizationFlags.doXingByXingPileUp.")
    from AthenaCommon.ConcurrencyFlags import jobproperties as jp
    if jp.ConcurrencyFlags.NumThreads() > 0:
        logConfigDigitization.error("Attempting to run pile-up digitization AthenaMT using %s threads!", str(jp.ConcurrencyFlags.NumThreads()))
        logConfigDigitization.error("Running pile-up digitization with AthenaMT is not supported. Please update your configuration. The job will fail now.")
        raise RuntimeError("Running pile-up digitization with AthenaMT is not supported. Please update your configuration.")
    include( "Digitization/ConfigPileUpEventLoopMgr.py" )
if DetFlags.pileup.any_on():
    logConfigDigitization.info("PILEUP CONFIGURATION:")
    logConfigDigitization.info(" -----> Luminosity = %s cm^-2 s^-1", jobproperties.Beam.estimatedLuminosity())
    logConfigDigitization.info(" -----> Bunch Spacing = %s ns", digitizationFlags.bunchSpacing.get_Value())

# in any case we need the PileUpMergeSvc for the digitize algos
if DetFlags.pileup.any_on():
    if not hasattr(ServiceMgr, 'PileUpMergeSvc'):
        from AthenaCommon import CfgGetter
        ServiceMgr += CfgGetter.getService("PileUpMergeSvc")

#--------------------------------------------------------------
# Subdetector-specific configuration
#--------------------------------------------------------------
include( "Digitization/DetectorDigitization.py" )

#--------------------------------------------------------------
# Random Number Engine and Seeds
#--------------------------------------------------------------
# attach digi and pileup seeds to random number service configurable and print them out
from AthenaCommon.ConfigurableDb import getConfigurable
ServiceMgr += getConfigurable(digitizationFlags.rndmSvc.get_Value())()
digitizationFlags.rndmSeedList.addtoService()
digitizationFlags.rndmSeedList.printSeeds()
rndmSvc = getConfigurable(digitizationFlags.rndmSvc.get_Value())()
rndmSvc.OutputLevel = WARNING
if digitizationFlags.readSeedsFromFile.get_Value():
    rndmSvc.Seeds=[]
    rndmSvc.ReadFromFile=True
    rndmSvc.FileToRead=digitizationFlags.rndmSeedInputFile.get_Value()
    logConfigDigitization.info("Random seeds for Digitization will be read from the file %s",digitizationFlags.rndmSeedInputFile.get_Value())

# write out a summary of the time spent

printNameAuditor=False

from AthenaCommon.AppMgr import theAuditorSvc
theApp.AuditAlgorithms=True  
theApp.AuditServices=True
theApp.AuditTools=True  

from GaudiAud.GaudiAudConf import ChronoAuditor, MemStatAuditor, NameAuditor
if 'ChronoAuditor/ChronoAuditor' not in theAuditorSvc.Auditors:
    theAuditorSvc += ChronoAuditor()
if 'MemStatAuditor/MemStatAuditor' not in theAuditorSvc.Auditors:
    theAuditorSvc += MemStatAuditor()
if printNameAuditor:
  from AthenaCommon.AppMgr import ServiceMgr
  ServiceMgr.MessageSvc.infoLimit = 0
  if 'MemStatAuditor/NameAuditor' not in theAuditorSvc.Auditors:
      theAuditorSvc += NameAuditor()
if 'FPEAuditor/FPEAuditor' not in theAuditorSvc.Auditors:
    from AthenaAuditors.AthenaAuditorsConf import FPEAuditor
    theAuditorSvc += FPEAuditor()
    import signal
    try:
        ServiceMgr.CoreDumpSvc.Signals.remove (signal.SIGFPE)
    except ValueError:
        pass

# Disable LOG printing
if hasattr(ServiceMgr, 'ChronoStatSvc'):
    ServiceMgr.ChronoStatSvc.ChronoPrintOutTable = False
    ServiceMgr.ChronoStatSvc.PrintUserTime       = False
    ServiceMgr.ChronoStatSvc.StatPrintOutTable   = False

# LSFTimeLimi. Temporary disable
# include( "LSFTimeKeeper/LSFTimeKeeperOptions.py" )

# LSFTimeKeeperSvc = Service( "LSFTimeKeeperSvc" )
# LSFTimeKeeperSvc.OutputLevel=2; # remaining CPU at each event
