# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

infile = 'aod/AOD-21.0.79/AOD-21.0.79-full.pool.root'
keys = [
    #xAOD::ElectronAuxContainer_v3
    'Electrons',
    'ForwardElectrons',

    #xAOD::PhotonAuxContainer_v3
    'Photons',
         ]

include ('AthenaPoolUtilities/TPCnvTest.py')
