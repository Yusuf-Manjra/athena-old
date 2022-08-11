#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger athenaHLT test of the Cosmic_run3_v1 menu on physics_Main stream from a cosmic run
# art-type: build
# art-include: master/Athena
# art-include: 22.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'data_cos_run2'  # TODO: update to R3 data recorded with 22.0.61 or later, ATR-25316
ex.max_events = 100
ex.args = '-c "setMenu=\'Cosmic_run3_v1\';doCosmics=True;doL1Sim=False;enableL1MuonPhase1=False;forceEnableAllChains=True;"'
ex.args += ' --dump-config-reload'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
