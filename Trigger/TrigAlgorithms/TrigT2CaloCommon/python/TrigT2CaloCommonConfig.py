# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from TrigT2CaloCommon.TrigT2CaloCommonConf import TrigCaloDataAccessSvc as _TrigCaloDataAccessSvc

class TrigCaloDataAccessSvc(_TrigCaloDataAccessSvc):
    __slots__ = ()

    def __init__(self, name='TrigCaloDataAccessSvc'):
        super(TrigCaloDataAccessSvc, self).__init__(name)

        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        from AthenaConfiguration.AllConfigFlags import ConfigFlags
        from AthenaCommon.GlobalFlags import globalflags
        from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
        from AthenaCommon.Logging import logging
        log = logging.getLogger(name)

        from RegionSelector.RegSelToolConfig import makeRegSelTool_TTEM, makeRegSelTool_TTHEC, makeRegSelTool_FCALEM, makeRegSelTool_FCALHAD, makeRegSelTool_TILE
        from AthenaCommon.AlgSequence import AthSequencer
        from LArCabling.LArCablingAccess import LArOnOffIdMapping, LArFebRodMapping
        LArFebRodMapping()
        LArOnOffIdMapping()
        self.RegSelToolEM = makeRegSelTool_TTEM()
        self.RegSelToolHEC = makeRegSelTool_TTHEC()
        self.RegSelToolFCALEM = makeRegSelTool_FCALEM()
        self.RegSelToolFCALHAD = makeRegSelTool_FCALHAD()
        self.RegSelToolTILE = makeRegSelTool_TILE()
        condseq = AthSequencer('AthCondSeq')
        condseq.RegSelCondAlg_TTEM.RegSelLUT="ConditionStore+RegSelLUTCondData_TTEM"
        from IOVDbSvc.CondDB import conddb
        from LArBadChannelTool.LArBadChannelAccess import LArBadChannelAccess
        if conddb.isMC:
           LArBadChannelAccess(dbString="<db>COOLOFL_LAR/OFLP200</db>/LAR/BadChannels/BadChannels")
        else:
           LArBadChannelAccess(dbString="<db>COOLONL_LAR/CONDBR2</db>/LAR/BadChannels/BadChannels")

        if ( globalflags.DatabaseInstance == "COMP200" and ConfigFlags.Trigger.calo.doOffsetCorrection ) :
            log.warning("Not possible to run BCID offset correction with COMP200")
        else:
            if ConfigFlags.Trigger.calo.doOffsetCorrection:
                if globalflags.DataSource()=='data' and athenaCommonFlags.isOnline():
                    log.info('Enable HLT calo offset correction for data')
                    conddb.addFolder("LAR_ONL","/LAR/ElecCalibFlat/OFC")
                    from LArRecUtils.LArRecUtilsConf import LArFlatConditionSvc
                    svcMgr += LArFlatConditionSvc()
                    svcMgr.LArFlatConditionSvc.OFCInput="/LAR/ElecCalibFlat/OFC"
                    svcMgr.ProxyProviderSvc.ProviderNames += [ "LArFlatConditionSvc" ]

                    from IOVDbSvc.CondDB import conddb
                    conddb.addFolder("LAR_ONL","/LAR/ElecCalibFlat/OFC",className = 'CondAttrListCollection')

                    from AthenaCommon.AlgSequence import AthSequencer
                    condSequence = AthSequencer("AthCondSeq")
                    from LArRecUtils.LArRecUtilsConf import LArFlatConditionsAlg_LArOFCFlat_ as LArOFCCondAlg
                    condSequence += LArOFCCondAlg (ReadKey="/LAR/ElecCalibFlat/OFC", WriteKey='LArOFC')
                    from LumiBlockComps.LuminosityCondAlgDefault import LuminosityCondAlgOnlineDefault
                    LuminosityCondAlgOnlineDefault()
                else:
                    log.info('Enable HLT calo offset correction for MC')

                # Place the CaloBCIDAvgAlg in HltBeginSeq
                from AthenaCommon.AlgSequence import AlgSequence
                topSequence = AlgSequence()
                from AthenaCommon.CFElements import findSubSequence
                hltBeginSeq = findSubSequence(topSequence, 'HLTBeginSeq')
                from CaloRec.CaloBCIDAvgAlgDefault import CaloBCIDAvgAlgDefault
                bcidAvgAlg = CaloBCIDAvgAlgDefault(sequence=hltBeginSeq)

                if not bcidAvgAlg:
                    log.info('Cannot use timer for CaloBCIDAvgAlg')
                else:
                    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
                    monTool = GenericMonitoringTool('MonTool')
                    monTool.defineHistogram('TIME_exec', path='EXPERT', type='TH1F', title="CaloBCIDAvgAlg execution time; time [ us ] ; Nruns", xbins=80, xmin=0.0, xmax=4000)
                    bcidAvgAlg.MonTool = monTool
                    log.info('using timer for CaloBCIDAvgAlg')


            else:
                log.info('Disable HLT calo offset correction')



        return
