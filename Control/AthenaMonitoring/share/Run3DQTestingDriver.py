#!/usr/bin/env python
#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file DQTestingDriver.py
@author C. D. Burton
@author P. Onyisi
@date 2019-06-20
@brief Driver script to run DQ with new-style configuration on an ESD/AOD
'''

if __name__=='__main__':
    import sys
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.Enums import Format

    parser = ConfigFlags.getArgumentParser()
    parser.add_argument('--preExec', help='Code to execute before locking configs')
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--dqOffByDefault', action='store_true',
                        help='Set all DQ steering flags to False, user must then switch them on again explicitly')
    # keep for compatibility reasons
    parser.add_argument('--inputFiles',
                        help='Comma-separated list of input files (alias for --filesInput)')
    # keep for compatibility reasons
    parser.add_argument('--maxEvents', type=int,
                        help='Maximum number of events to process (alias for --evtMax)')
    parser.add_argument('--printDetailedConfig', action='store_true',
                        help='Print detailed Athena configuration')
    parser.add_argument('--perfmon', action='store_true',
                        help='Run perfmon')
    # change default
    parser.set_defaults(threads=1)
    args, _ = parser.parse_known_args()

    # Setup the Run III behavior
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = 1
    
    # default input if nothing specified
    ConfigFlags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/AthenaMonitoring/q431/21.0/f946/myESD.pool.root']
    ConfigFlags.Output.HISTFileName = 'ExampleMonitorOutput.root'
    if args.dqOffByDefault:
        from AthenaMonitoring.DQConfigFlags import allSteeringFlagsOff
        allSteeringFlagsOff()
    ConfigFlags.fillFromArgs(parser=parser)

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon import Constants
    if args.loglevel:
        log.setLevel(getattr(Constants, args.loglevel))

    # override Input.Files with result from our own arguments
    # if --filesInput was specified as well (!) this will override
    if args.inputFiles is not None:
        ConfigFlags.Input.Files = args.inputFiles.split(',')
    # if --evtMax was specified as well this will override
    if args.maxEvents is not None:
        ConfigFlags.Exec.MaxEvents = args.maxEvents

    if ConfigFlags.Input.Format is Format.BS:
        if ConfigFlags.DQ.Environment not in ('tier0', 'tier0Raw', 'online'):
            log.warning('Reading RAW file, but DQ.Environment set to %s',
                        ConfigFlags.DQ.Environment)
            log.warning('Will proceed but best guess is this is an error')
        log.info('Will schedule reconstruction, as best we know')
    else:
        if ConfigFlags.DQ.Environment in ('tier0', 'tier0Raw', 'online'):
            log.warning('Reading POOL file, but DQ.Environment set to %s',
                        ConfigFlags.DQ.Environment)
            log.warning('Will proceed but best guess is this is an error')

    # perfmon
    if args.perfmon:
        ConfigFlags.PerfMon.doFullMonMT=True

    if args.preExec:
        # bring things into scope
        from AthenaMonitoring.DQConfigFlags import allSteeringFlagsOff
        log.info('Executing preExec: %s', args.preExec)
        exec(args.preExec)

    if hasattr(ConfigFlags, "DQ") and hasattr(ConfigFlags.DQ, "Steering") and hasattr(ConfigFlags, "Detector"):
        if hasattr(ConfigFlags.DQ.Steering, "InDet"):
            if ((ConfigFlags.DQ.Steering.InDet, "doAlignMon") and ConfigFlags.DQ.Steering.InDet.doAlignMon) or \
               ((ConfigFlags.DQ.Steering.InDet, "doGlobalMon") and ConfigFlags.DQ.Steering.InDet.doGlobalMon) or \
               ((ConfigFlags.DQ.Steering.InDet, "doPerfMon") and ConfigFlags.DQ.Steering.InDet.doPerfMon):
                ConfigFlags.Detector.GeometryID = True

    # Just assume we want the full ID geometry, if we are reading in geometry
    ConfigFlags.Detector.GeometryPixel = True
    ConfigFlags.Detector.GeometrySCT = True
    ConfigFlags.Detector.GeometryTRT = True

    log.info('FINAL CONFIG FLAGS SETTINGS FOLLOW')
    if args.loglevel is None or getattr(Constants, args.loglevel) <= Constants.INFO:
        ConfigFlags.dump()
    ConfigFlags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(ConfigFlags)

    # add FPE auditor
    from AthenaConfiguration.ComponentFactory import CompFactory
    cfg.addAuditor(CompFactory.FPEAuditor())

    # add perfmon
    if args.perfmon:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge(PerfMonMTSvcCfg(ConfigFlags))

    if ConfigFlags.Input.Format is Format.BS:
        # attempt to start setting up reco ...
        from CaloRec.CaloRecoConfig import CaloRecoCfg
        cfg.merge(CaloRecoCfg(ConfigFlags))
    else:
        cfg.merge(PoolReadCfg(ConfigFlags))

    # load DQ
    from AthenaMonitoring.AthenaMonitoringCfg import AthenaMonitoringCfg
    dq = AthenaMonitoringCfg(ConfigFlags)
    cfg.merge(dq)

    # Force loading of conditions in MT mode
    if ConfigFlags.Concurrency.NumThreads > 0:
        if len([_ for _ in cfg._conditionsAlgs if _.name=="PixelDetectorElementCondAlg"]) > 0:
            beginseq = cfg.getSequence("AthBeginSeq")
            beginseq.Members.append(CompFactory.ForceIDConditionsAlg("ForceIDConditionsAlg"))
        if len([_ for _ in cfg._conditionsAlgs if _.name=="MuonAlignmentCondAlg"]) > 0:
            beginseq = cfg.getSequence("AthBeginSeq")
            beginseq.Members.append(CompFactory.ForceMSConditionsAlg("ForceMSConditionsAlg"))
    
    # any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=args.printDetailedConfig) # set True for exhaustive info

    sc = cfg.run()
    sys.exit(0 if sc.isSuccess() else 1)
