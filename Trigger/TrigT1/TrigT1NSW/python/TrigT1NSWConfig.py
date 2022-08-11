# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from TrigT1NSW.TrigT1NSWConf import NSWL1__NSWL1Simulation

class DefaultNSWL1Simulation(NSWL1__NSWL1Simulation):
    __slots__ = []

    def __init__(self, name = "DefaultNSWL1Simulation"):
        super( DefaultNSWL1Simulation, self ).__init__( name )

    def setDefaults(self, handle):
        # so far no additional defaults
        pass

class NSWL1Simulation(DefaultNSWL1Simulation):
    __slots__ = []

    def __init__(self, name = "NSWL1Simulation"):
        super( NSWL1Simulation, self ).__init__( name )

    def setDefaults(self, handle):
        #DefaultNSWL1Simulation.setDefault(handle)
        pass
