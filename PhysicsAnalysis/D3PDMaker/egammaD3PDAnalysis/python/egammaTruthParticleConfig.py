# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#
# @file egammaD3PDAnalysis/python/egammaTruthParticleConfig.py
# @author scott snyder <snyder@bnl.gov>
# @date Mar, 2011
# @brief Configure algorithms to build filtered TruthParticleContainer
#        for egamma truth.
#


import egammaD3PDAnalysis
from D3PDMakerConfig.D3PDMakerFlags           import D3PDMakerFlags
from AthenaCommon.AlgSequence                 import AlgSequence
from RecExConfig.ObjKeyStore                  import cfgKeyStore
from RecExConfig.RecFlags                     import rec
from AthenaCommon                             import CfgMgr


def egammaTruthParticleConfig \
        (seq = AlgSequence(D3PDMakerFlags.PreD3PDAlgSeqName()),
         sgkey = 'egammaTruth',
         prefix = '',
         doPileup     = D3PDMakerFlags.TruthDoPileup(),
         **kwargs):

    if not rec.doTruth():
        return

    # Is the container already in SG?
    if cfgKeyStore.isInInput ('DataVector<xAOD::TruthParticle_v1>', sgkey):
        return

    algname = prefix + sgkey + 'Builder'
    if not hasattr (seq, algname):
        from MCTruthClassifier.MCTruthClassifierBase import getMCTruthClassifierExtrapolator
        from egammaRec.Factories import ToolFactory
        exten = ToolFactory (CfgMgr.Trk__ParticleCaloExtensionTool,
                             name="GSFParticleCaloExtensionTool",
                             Extrapolator = getMCTruthClassifierExtrapolator(),
                             StartFromPerigee = True)()

        seq += egammaD3PDAnalysis.egammaTruthAlg (
            algname,
            InputKey = D3PDMakerFlags.TruthSGKey(),
            OutputKey = sgkey,
            ParticleCaloExtensionTool = exten,
            AuxPrefix = D3PDMakerFlags.EgammaUserDataPrefix())

        cfgKeyStore.addTransient ('DataVector<xAOD::TruthParticle_v1>', sgkey)

    return



def egammaTruthParticleCfg (flags,
                            algname = 'egammaTruthBuilder',
                            sequenceName = None,
                            **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory

    seqkw = {'sequence': sequenceName} if sequenceName else {}
    acc = ComponentAccumulator (**seqkw)

    if 'ParticleCaloExtensionTool' not in kwargs:
        from MCTruthClassifier.MCTruthClassifierBase import getMCTruthClassifierCaloExtensionTool
        kwargs['ParticleCaloExtensionTool'] = getMCTruthClassifierCaloExtensionTool()

    kwargs.setdefault ('InputKey', D3PDMakerFlags.TruthSGKey())
    kwargs.setdefault ('OutputKey', 'egammaTruth')
    kwargs.setdefault ('AuxPrefix', D3PDMakerFlags.EgammaUserDataPrefix())

    # From egammaD3PDAnalysis
    alg = CompFactory.D3PD.egammaTruthAlg (algname, **kwargs)
    acc.addEventAlgo (alg)
    return acc

