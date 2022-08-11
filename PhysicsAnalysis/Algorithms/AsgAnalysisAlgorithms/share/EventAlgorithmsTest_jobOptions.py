# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#
# @author Tadej Novak

# User options, which can be set from command line after a "-" character
# athena EventAlgorithmsTest_jobOptions.py - --myOption ...
from AthenaCommon.AthArgumentParser import AthArgumentParser
athArgsParser = AthArgumentParser()
athArgsParser.add_argument("--data-type", action = "store", dest = "data_type",
                           default = "mc",
                           help = "Type of input to run over. Valid options are 'data', 'mc', 'afii'")
athArgs = athArgsParser.parse_args()

dataType = athArgs.data_type
if not dataType in ["data", "mc", "afii"] :
    raise Exception ("invalid data type: " + dataType)

print("Running on data type: " + dataType)

inputfile = {"data": 'ASG_TEST_FILE_DATA',
             "mc":   'ASG_TEST_FILE_MC',
             "afii": 'ASG_TEST_FILE_MC_AFII'}

# Set up the reading of the input file:
import AthenaPoolCnvSvc.ReadAthenaPool
theApp.EvtMax = 500
testFile = os.getenv ( inputfile[dataType] )
svcMgr.EventSelector.InputCollections = [testFile]

# Needed for filtering, Athena only for now
from EventBookkeeperTools.CutFlowHelpers import CreateCutFlowSvc
CreateCutFlowSvc(svcName="CutFlowSvc", seq=athAlgSeq, addMetaDataToAllOutputFiles=False)

from AsgAnalysisAlgorithms.AsgAnalysisAlgorithmsTest import makeEventAlgorithmsSequence
algSeq = makeEventAlgorithmsSequence (dataType)
print( algSeq ) # For debugging

# Add all algorithms from the sequence to the job.
athAlgSeq += algSeq

# Set up a histogram output file for the job:
ServiceMgr += CfgMgr.THistSvc()
ServiceMgr.THistSvc.Output += [
    "ANALYSIS DATAFILE='EventAlgorithmsTest." + dataType + ".hist.root' OPT='RECREATE'"
    ]

# Reduce the printout from Athena:
include( "AthAnalysisBaseComps/SuppressLogging.py" )
