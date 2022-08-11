#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def fastL2EgammaClusteringAlg( flags, roisKey="EMCaloRoIs", doRinger=False):

    acc = ComponentAccumulator()
    # configure calo data access
    from TrigT2CaloCommon.TrigCaloDataAccessConfig import trigCaloDataAccessSvcCfg, CaloDataAccessSvcDependencies
    cdaSvcAcc = trigCaloDataAccessSvcCfg( flags )
    cdaSvc = cdaSvcAcc.getService("TrigCaloDataAccessSvc")
    acc.merge( cdaSvcAcc )

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(flags) )

    # configure tools (this can be simplified further,
    EgammaReEmEnFex, EgammaReHadEnFex, EgammaReSamp1Fex, EgammaReSamp2Fex=CompFactory.getComps("EgammaReEmEnFex","EgammaReHadEnFex","EgammaReSamp1Fex","EgammaReSamp2Fex",)

    samp2 = EgammaReSamp2Fex(name='ReFaAlgoSamp2FexConfig', MaxDetaHotCell=0.15, MaxDphiHotCell=0.15)
    acc.addPublicTool( samp2 )

    samp1 = EgammaReSamp1Fex('ReFaAlgoSamp1FexConfig')
    acc.addPublicTool( samp1 )

    sampe = EgammaReEmEnFex('ReFaAlgoEmEnFexConfig')
    sampe.ExtraInputs=[( 'LArOnOffIdMapping' , 'ConditionStore+LArOnOffIdMap' )]
    acc.addPublicTool( sampe )

    samph = EgammaReHadEnFex('ReFaAlgoHadEnFexConfig')
    samph.ExtraInputs=[('TileEMScale','ConditionStore+TileEMScale'),('TileBadChannels','ConditionStore+TileBadChannels')]
    acc.addPublicTool( samph )

    alg = CompFactory.T2CaloEgammaReFastAlgo("FastCaloL2EgammaAlg")
    from TrigEDMConfig.TriggerEDMRun3 import recordable
    alg.ClustersName   = recordable('HLT_FastCaloEMClusters')
    alg.RoIs           = roisKey
    alg.EtaWidth       = 0.2
    alg.PhiWidth       = 0.2
    alg.ExtraInputs = CaloDataAccessSvcDependencies
    alg.BCIDAvgKey     = "StoreGateSvc+CaloBCIDAverage"


    __fex_tools = [ samp2, samp1, sampe, samph] #, ring ]

    if doRinger:
      from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import RingerReFexConfig, AsymRingerReFexConfig
      ringer = RingerReFexConfig('ReFaAlgoRingerFexConfig')
      ringer.RingerKey = recordable("HLT_FastCaloRinger")
      ringer.ClustersName = alg.ClustersName
      acc.addPublicTool( ringer )
      __fex_tools.append(ringer)

      asymringer = AsymRingerReFexConfig('ReFaAlgoAsymRingerFexConfig')
      asymringer.RingerKey= "HLT_FastCaloAsymRinger"
      asymringer.trigDataAccessMT = cdaSvc
      asymringer.ClustersName = alg.ClustersName
      __fex_tools.append( asymringer )
    alg.IReAlgToolList = __fex_tools

    for t in __fex_tools:
        t.trigDataAccessMT = cdaSvc # set data access svc


    # set calibration
    from TrigT2CaloCalibration.EgammaCalibrationConfig import EgammaSshapeCalibrationBarrelConfig, EgammaHitsCalibrationBarrelConfig, \
                                                              EgammaGapCalibrationConfig, EgammaTransitionRegionsConfig
    alg.CalibListBarrel  = [EgammaSshapeCalibrationBarrelConfig()]
    alg.CalibListBarrel += [EgammaHitsCalibrationBarrelConfig()]
    alg.CalibListBarrel += [EgammaGapCalibrationConfig()]
    alg.CalibListBarrel += [EgammaTransitionRegionsConfig()]

    from TrigT2CaloCalibration.EgammaCalibrationConfig import EgammaSshapeCalibrationEndcapConfig, EgammaHitsCalibrationEndcapConfig, \
                                                              EgammaGapCalibrationConfig
    alg.CalibListEndcap  = [EgammaSshapeCalibrationEndcapConfig()]
    alg.CalibListEndcap += [EgammaHitsCalibrationEndcapConfig()]
    alg.CalibListEndcap += [EgammaGapCalibrationConfig()]

    acc.addEventAlgo( alg )
    return acc
