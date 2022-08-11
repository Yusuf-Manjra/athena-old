#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger AthenaMT test running new-style job options
# art-type: build
# art-include: master/Athena
# art-include: 22.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Input

# Generate configuration run file
run = ExecStep.ExecStep('athena_run')
run.type = 'other'
run.input = 'data'
run.executable = 'runHLT_standalone_newJO.py'
run.args  = ' --filesInput='+Input.get_input('data').paths[0]
run.args += ' Trigger.triggerMenuSetup="Dev_pp_run3_v1"'
run.args += ' Trigger.doRuntimeNaviVal=True'
run.prmon = False


# The full test configuration
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [run]
test.check_steps = CheckSteps.default_check_steps(test)
test.get_step('MessageCount').log_regex = 'athena_run.log'

chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_data_newJO_build.new'
# In future, when this is primary test will enable strict reference checking 
# The code below does so.
# refcomp = CheckSteps.ChainCompStep("CountRefComp")
# refcomp.input_file = 'ref_data_newJO_build.new'
# refcomp.required = True # Final exit code depends on this step
# CheckSteps.add_step_after_type(test.check_steps, CheckSteps.ChainDumpStep, refcomp)

import sys
sys.exit(test.run())
