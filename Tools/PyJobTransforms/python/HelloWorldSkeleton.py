# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from AthExHelloWorld.HelloWorldConfig import HelloWorldCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg


def fromRunArgs(runArgs):
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    commonRunArgsToFlags(runArgs, ConfigFlags)

    processPreInclude(runArgs, ConfigFlags)
    processPreExec(runArgs, ConfigFlags)

    if '_ATHENA_GENERIC_INPUTFILE_NAME_' in ConfigFlags.Input.Files:
        ConfigFlags.Input.Files = []
    ConfigFlags.lock()

    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(HelloWorldCfg())

    processPostInclude(runArgs, ConfigFlags, cfg)
    processPostExec(runArgs, ConfigFlags, cfg)

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
