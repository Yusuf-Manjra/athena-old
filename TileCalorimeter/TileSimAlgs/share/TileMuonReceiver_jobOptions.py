#
#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#

# ****************************************************************************
# jobOptions file for setting up TilePulseForTileMuonReceiver algorithm
# ============================================================================

# The same pedestal and noise sigma values  for all channels 
# in simulation of TileMuonReceiver board will be used.
TileMuRcvNoiseSigma = 2.0
TileMuRcvPedestal = 11.73 # Should be the same in TileInfo and TileRawChannelBuilderMF

from AthenaCommon.AppMgr import ServiceMgr as svcMgr
svcMgr.TileInfoLoader.MuRcvNoiseSigma = TileMuRcvNoiseSigma
svcMgr.TileInfoLoader.MuRcvPed = TileMuRcvPedestal
# Uncomment for a local run BUT *COMMENT* this line prior to an upload to SVN
# svcMgr.TileCablingSvc.CablingType = 4 

from AthenaCommon import CfgMgr

# Set up TileCondToolPulseShape to be used in
# TilePulseForTileMuonReceiver and TileCondToolOfc
from TileConditions.TileCondToolConf import getTileCondToolMuRcvPulseShape
TileCondToolMuRcvPulseShape = getTileCondToolMuRcvPulseShape('FILE', 'TileCondToolMuRcvPulseShape')

# Set up TileCondToolOfc to be used in TileRawChannelBuilderMFi
TileCondToolMuRcvOfc = CfgMgr.TileCondToolOfc('TileCondToolMuRcvOfc'
                                              , OptFilterDeltaCorrelation = True
                                              , TileCondToolPulseShape = TileCondToolMuRcvPulseShape)

# Set up TileRawChannelBuilderMF to be used in TilePulseForTileMuonReceiver
TileMuRcvRawChannelBuilderMF = CfgMgr.TileRawChannelBuilderMF('TileMuRcvRawChannelBuilderMF'
                                                              , MF = 1
                                                              , PedestalMode = 0
                                                              , DefaultPedestal = TileMuRcvPedestal
                                                              , calibrateEnergy = jobproperties.TileRecFlags.calibrateEnergy()
                                                              , TileCondToolOfc = TileCondToolMuRcvOfc
                                                              , TileCondToolOfcOnFly = TileCondToolMuRcvOfc
                                                              , TimeMinForAmpCorrection = jobproperties.TileRecFlags.TimeMinForAmpCorrection()
                                                              , TimeMaxForAmpCorrection = jobproperties.TileRecFlags.TimeMaxForAmpCorrection())

#  Random number engine in TilePulseForTileMuonReceiver
from Digitization.DigitizationFlags import jobproperties

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

#  Set up TilePulseForTileMuonReceiver
from TileConditions.TileConditionsConf import TileCondToolNoiseSample
topSequence += CfgMgr.TilePulseForTileMuonReceiver('TilePulseForTileMuonReceiver'
                                                   , IntegerDigits = not jobproperties.Digitization.PileUpPresampling()
                                                   , UseCoolPedestal = False
                                                   , UseCoolPulseShapes = True
                                                   , TileCondToolPulseShape = TileCondToolMuRcvPulseShape
                                                   , TileRawChannelBuilderMF = TileMuRcvRawChannelBuilderMF
                                                   , TileCondToolNoiseSample = TileCondToolNoiseSample (TileOnlineSampleNoise = ''))
if jobproperties.Digitization.PileUpPresampling and 'LegacyOverlay' not in jobproperties.Digitization.experimentalDigi():
    from OverlayCommonAlgs.OverlayFlags import overlayFlags
    topSequence.TilePulseForTileMuonReceiver.MuonReceiverDigitsContainer = overlayFlags.bkgPrefix() + "MuRcvDigitsCnt"

#
# Thresholds
#

topSequence += CfgMgr.TileMuonReceiverDecision('TileMuonReceiverDecision'
# run 2 thresholds
                                                , MuonReceiverEneThreshCellD6Low = 500
                                                , MuonReceiverEneThreshCellD6andD5Low = 500
                                                , MuonReceiverEneThreshCellD6High = 600
                                                , MuonReceiverEneThreshCellD6andD5High = 600
# run 3 thresholds
                                                , MuonReceiverEneThreshCellD5 = 500
                                                , MuonReceiverEneThreshCellD6 = 500
                                                , MuonReceiverEneThreshCellD5andD6 = 500)


if jobproperties.Digitization.PileUpPresampling and 'LegacyOverlay' not in jobproperties.Digitization.experimentalDigi():
    from OverlayCommonAlgs.OverlayFlags import overlayFlags
    topSequence.TileMuonReceiverDecision.TileMuonReceiverContainer = overlayFlags.bkgPrefix() + "TileMuRcvCnt"
