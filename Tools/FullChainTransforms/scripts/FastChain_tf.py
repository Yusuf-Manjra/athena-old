#! /usr/bin/env python

# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

#  FastChain_tf.py
#  One step transform to run SIM+DIGI as one job, then reco
#  to ESD/AOD output
#  Richard Hawkings, adapted from FullChain_tf.py by Graeme Stewart

import sys
import time

# Setup core logging here
from PyJobTransforms.trfLogger import msg
msg.info('logging set in %s', sys.argv[0])

from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import athenaExecutor
from PyJobTransforms.trfArgs import addAthenaArguments, addDetectorArguments, addTriggerArguments
from PyJobTransforms.trfDecorators import stdTrfExceptionHandler, sigUsrStackTrace
from RecJobTransforms.recTransformUtils import addRecoSubsteps, addAllRecoArgs
from SimuJobTransforms.simTrfArgs import addCommonSimTrfArgs, addBasicDigiArgs, addCommonSimDigTrfArgs, addTrackRecordArgs, addSim_tfArgs, addPileUpTrfArgs
from PATJobTransforms.PATTransformUtils import addPhysValidationFiles, addValidationArguments, appendPhysValidationSubstep

from PyJobTransforms.trfArgClasses import argFactory, argList, argRDOFile

@stdTrfExceptionHandler
@sigUsrStackTrace
def main():

    msg.info('This is %s', sys.argv[0])

    trf = getTransform()
    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()

    msg.info("%s stopped at %s, trf exit code %d", sys.argv[0], time.asctime(), trf.exitCode)
    sys.exit(trf.exitCode)

def getTransform():
    executorSet = set()

    addRecoSubsteps(executorSet)

    # Sim + Digi - factor these out into an importable function in time
    executorSet.add(athenaExecutor(name = 'EVNTtoRDO', skeletonFile = 'FullChainTransforms/FastChainSkeleton.EVGENtoRDO.py',
                                   skeletonCA = 'FullChainTransforms.FastChain_Skeleton',
                                   substep = 'simdigi', tryDropAndReload = False, perfMonFile = 'ntuple.pmon.gz',
                                   inData=['NULL','EVNT'],
                                   outData=['RDO','NULL'] ))

    # Overlay
    from OverlayConfiguration.OverlayTransformHelpers import addOverlayArguments
    executorSet.add(athenaExecutor(name='Overlay',
                                   skeletonFile='OverlayConfiguration/skeleton_LegacyOverlay.py',
                                   skeletonCA='OverlayConfiguration.OverlaySkeleton',
                                   substep='overlay',
                                   tryDropAndReload=False,
                                   perfMonFile='ntuple.pmon.gz',
                                   inData=['RDO_BKG', 'BS_SKIM', 'HITS'],
                                   outData=['RDO', 'RDO_SGNL']))

    # Sim + Overlay - execute with the argument --steering "doFCwOverlay"
    executorSet.add(athenaExecutor(name = 'EVNTtoRDOwOverlay', skeletonFile = 'FullChainTransforms/FastChainSkeleton.EVGENtoRDOwOverlay.py',
                                   substep = 'simoverlay', tryDropAndReload = False, perfMonFile = 'ntuple.pmon.gz',
                                   inData = ['NULL'], outData = ['NULL']))

    trf = transform(executor = executorSet, description = 'Fast chain ATLAS transform with ISF simulation, digitisation'
                    ' and reconstruction. Inputs can be EVNT, with outputs of RDO, ESD, AOD or DPDs.'
                    ' See https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/FastChainTf for more details.')

    # Common arguments
    addAthenaArguments(trf.parser)
    addDetectorArguments(trf.parser)
    addTriggerArguments(trf.parser)

    # Reconstruction arguments and outputs (use the factorised 'do it all' function)
    addAllRecoArgs(trf)

    # Simulation and digitisation options
    addCommonSimTrfArgs(trf.parser)
    addCommonSimDigTrfArgs(trf.parser)
    addBasicDigiArgs(trf.parser)
    addSim_tfArgs(trf.parser)
    # addForwardDetTrfArgs(trf.parser)
    addPileUpTrfArgs(trf.parser)
    addTrackRecordArgs(trf.parser)
    addFastChainTrfArgs(trf.parser)

    # Overlay arguments
    addOverlayArguments(trf.parser)

    # Add PhysVal
    addPhysValidationFiles(trf.parser)
    addValidationArguments(trf.parser)
    appendPhysValidationSubstep(trf)

    return trf

def addFastChainTrfArgs(parser):
    "Add transformation arguments for fast chain"
    parser.defineArgGroup('FastChain','Fast chain options')
    parser.add_argument('--preSimExec',type=argFactory(argList),nargs='+',
                        help='preExec before simulation step',
                        group='FastChain')
    parser.add_argument('--postSimExec',type=argFactory(argList),nargs='+',
                        help='postExec after simulation step',
                        group='FastChain')
    parser.add_argument('--preDigiExec',type=argFactory(argList),nargs='+',
                        help='preExec before digitisation step',
                        group='FastChain')
    parser.add_argument('--preSimInclude',type=argFactory(argList),nargs='+',
                        help='preInclude before simulation step',
                        group='FastChain')
    parser.add_argument('--postSimInclude',type=argFactory(argList),nargs='+',
                        help='postInclude after simulation step',
                        group='FastChain')
    parser.add_argument('--preDigiInclude',type=argFactory(argList),nargs='+',
                        help='preInclude before digitisation step',
                        group='FastChain')
    parser.defineArgGroup('MCOverlay', 'MC Overlay Options')
    parser.add_argument('--inputRDO_BKGFile', nargs='+',
                        type=argFactory(argRDOFile, io='input'),
                        help='Input background RDO for MC+MC overlay',
                        group='MCOverlay')


if __name__ == '__main__':
    main()
