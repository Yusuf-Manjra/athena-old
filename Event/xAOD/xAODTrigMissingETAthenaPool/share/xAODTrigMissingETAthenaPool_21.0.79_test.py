# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

infile = 'aod/AOD-21.0.79/AOD-21.0.79-full.pool.root'
keys = [
    #xAOD::TrigMissingETAuxContainer_v1
    'HLT_xAOD__TrigMissingETContainer_EFJetEtSum',
    'HLT_xAOD__TrigMissingETContainer_EFMissingET_Fex_2sidednoiseSupp_PUC',
    'HLT_xAOD__TrigMissingETContainer_T2MissingET',
    'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET',
    'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_FEB',
    'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht',
    'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl',
    'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl_PS',
    'HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_topocl_PUC',
    'HLT_xAOD__TrigMissingETContainer_TrigL2MissingET_FEB',
         ]

include ('AthenaPoolUtilities/TPCnvTest.py')
