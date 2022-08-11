"""Main overlay transform configuration helpers

Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
"""

from PyJobTransforms.trfArgClasses import argBSFile, argFactory, argList, argRDOFile, argSubstep, argSubstepInt
from PyJobTransforms.trfExe import athenaExecutor


def addOverlayTrfArgs(parser):
    """Add common overlay command-line parser arguments."""
    parser.defineArgGroup('Overlay', 'Common Overlay Options')
    parser.add_argument('--detectors', nargs='*',
                        type=argFactory(argList),
                        help='Detectors autoconfiguration string',
                        group='Overlay')
    parser.add_argument('--skipSecondaryEvents', nargs='+',
                        type=argFactory(argSubstepInt, defaultSubstep='first'), 
                        help='Number of secondary input events to skip over in the first processing step (skipping substep can be overridden)',
                        group='Overlay')
    parser.add_argument('--outputRDO_SGNLFile', nargs='+',
                        type=argFactory(argRDOFile, io='output'),
                        help='The output RDO file of the MC signal alone',
                        group='Overlay')


def addDataOverlayTrfArgs(parser):
    """Add data overlay command-line parser arguments."""
    parser.defineArgGroup('DataOverlay', 'Data Overlay Options')
    parser.add_argument('--inputBS_SKIMFile', nargs='+',
                        type=argFactory(argBSFile, io='input'),
                        help='Input skimmed RAW BS for pileup overlay',
                        group='DataOverlay')
    parser.add_argument('--fSampltag',
                        type=argFactory(argSubstep, defaultSubstep='overlay'),
                        help='The cool tag for /LAR/ElecCalib/fSampl/Symmetry, see https://twiki.cern.ch/twiki/bin/viewauth/Atlas/LArCalibMCPoolCool',
                        group='DataOverlay')


def addMCOverlayTrfArgs(parser):
    """Add MC overlay command-line parser arguments."""
    parser.defineArgGroup('MCOverlay', 'MC Overlay Options')
    parser.add_argument('--inputRDO_BKGFile', nargs='+',
                        type=argFactory(argRDOFile, io='input'),
                        help='Input background RDO for MC+MC overlay',
                        group='MCOverlay')


def addOverlayArguments(parser, in_reco_chain=False):
    """Add all overlay command-line parser arguments."""
    # TODO: are forward detectors really needed?
    from SimuJobTransforms.simTrfArgs import addBasicDigiArgs  # , addForwardDetTrfArgs
    addBasicDigiArgs(parser)
    # addForwardDetTrfArgs(parser)
    addOverlayTrfArgs(parser)
    addMCOverlayTrfArgs(parser)
    if not in_reco_chain:
        addDataOverlayTrfArgs(parser)


def addOverlaySubstep(executor_set, in_reco_chain=False):
    executor = athenaExecutor(name='Overlay',
                              skeletonFile='OverlayConfiguration/skeleton_LegacyOverlay.py',
                              skeletonCA='OverlayConfiguration.OverlaySkeleton',
                              substep='overlay',
                              tryDropAndReload=False,
                              perfMonFile='ntuple.pmon.gz',
                              inData=['RDO_BKG', 'BS_SKIM', 'HITS'],
                              outData=['RDO', 'RDO_SGNL'])

    if in_reco_chain:
        executor.inData = []
        executor.outData = []

    executor_set.add(executor)


def appendOverlaySubstep(trf, in_reco_chain=False):
    """Add overlay transform substep."""
    executor = set()
    addOverlaySubstep(executor, in_reco_chain)
    trf.appendToExecutorSet(executor)
