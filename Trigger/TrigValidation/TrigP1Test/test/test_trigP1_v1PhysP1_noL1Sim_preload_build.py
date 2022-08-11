#!/usr/bin/env python
# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
#
# art-description: PhysicsP1_pp_run3_v1 menu athenaHLT test imitating partition with preloaded data at P1
# art-type: build
# art-include: master/Athena
# art-include: 22.0/Athena

from TrigP1Test.PreloadTest import test_trigP1_preload
import sys

sys.exit(test_trigP1_preload('PhysicsP1_pp_run3_v1').run())
