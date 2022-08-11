# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

infile = 'aod/AOD-21.0.79/AOD-21.0.79-full.pool.root'
keys = [
    #xAOD::JetTrigAuxContainer_v2
    'HLT_xAOD__JetContainer_EFJet',
    'HLT_xAOD__JetContainer_FarawayJet',
    'HLT_xAOD__JetContainer_GSCJet',
    'HLT_xAOD__JetContainer_SplitJet',
    'HLT_xAOD__JetContainer_SuperRoi',
    'HLT_xAOD__JetContainer_a10r_tcemsubjesISFS',
    'HLT_xAOD__JetContainer_a10tclcwsubjesFS',
    'HLT_xAOD__JetContainer_a10ttclcwjesFS',
    'HLT_xAOD__JetContainer_a3ionemsubjesFS',
    'HLT_xAOD__JetContainer_a4ionemsubjesFS',
    'HLT_xAOD__JetContainer_a4tcemsubjesISFS',
    'HLT_xAOD__JetContainer_a4tcemsubjesISFSftk',
    'HLT_xAOD__JetContainer_a4tcemsubjesISFSftkrefit',

    #xAOD::JetAuxContainer_v1
    'AntiKt4EMPFlowJets',
    'AntiKt4EMTopoJets',
    'AntiKt4LCTopoJets',
    'InTimeAntiKt4TruthJets',
    'OutOfTimeAntiKt4TruthJets',
         ]

include ('AthenaPoolUtilities/TPCnvTest.py')
