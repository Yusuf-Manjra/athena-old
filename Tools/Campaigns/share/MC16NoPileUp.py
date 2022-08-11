# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.BeamFlags import jobproperties as bf
bf.Beam.numberOfCollisions.set_Value_and_Lock(0)

from AthenaCommon.Resilience import protectedInclude
protectedInclude('LArConfiguration/LArConfigRun2Old_NoPileup.py')

protectedInclude('PyJobTransforms/HepMcParticleLinkVerbosity.py')
