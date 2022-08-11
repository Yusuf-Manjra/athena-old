#!/usr/bin/env python
"""Run tests on CSC_DigitizationConfigNew.py

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
import sys
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaConfiguration.AllConfigFlags import ConfigFlags
from MuonConfig.CSC_DigitizationConfig import CSC_DigitBuilderDigitToRDOCfg

# Set up logging
log.setLevel(DEBUG)
# Configure
ConfigFlags.Input.Files = defaultTestFiles.HITS_RUN2
ConfigFlags.Output.RDOFileName = "myRDO.pool.root"
ConfigFlags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
ConfigFlags.lock()
# Function tests
# Construct our accumulator to run
acc = MainServicesCfg(ConfigFlags)
acc.merge(PoolReadCfg(ConfigFlags))
acc.merge(CSC_DigitBuilderDigitToRDOCfg(ConfigFlags))
# Dump config
acc.getService("StoreGateSvc").Dump = True
acc.getService("ConditionStore").Dump = True
acc.printConfig(withDetails=True)
ConfigFlags.dump()
# Execute and finish
sc = acc.run(maxEvents=3)
# Success should be 0
sys.exit(not sc.isSuccess())

