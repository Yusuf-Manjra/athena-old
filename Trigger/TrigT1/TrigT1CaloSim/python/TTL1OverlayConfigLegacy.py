# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr

def getLArTTL1Overlay(name="LArTTL1Overlay", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags
    if overlayFlags.isOverlayMT():
        kwargs.setdefault("BkgEmTTL1Key", overlayFlags.bkgPrefix() + "LArTTL1EM")
        kwargs.setdefault("SignalEmTTL1Key", overlayFlags.sigPrefix() + "LArTTL1EM")
        kwargs.setdefault("OutputEmTTL1Key", "LArTTL1EM")
        kwargs.setdefault("BkgHadTTL1Key", overlayFlags.bkgPrefix() + "LArTTL1HAD")
        kwargs.setdefault("SignalHadTTL1Key", overlayFlags.sigPrefix() + "LArTTL1HAD")
        kwargs.setdefault("OutputHadTTL1Key", "LArTTL1HAD")
    else:
        kwargs.setdefault("BkgEmTTL1Key", overlayFlags.dataStore() + "+LArTTL1EM")
        kwargs.setdefault("SignalEmTTL1Key", overlayFlags.evtStore() + "+LArTTL1EM")
        kwargs.setdefault("OutputEmTTL1Key", overlayFlags.outputStore() + "+LArTTL1EM")
        kwargs.setdefault("BkgHadTTL1Key", overlayFlags.dataStore() + "+LArTTL1HAD")
        kwargs.setdefault("SignalHadTTL1Key", overlayFlags.evtStore() + "+LArTTL1HAD")
        kwargs.setdefault("OutputHadTTL1Key", overlayFlags.outputStore() + "+LArTTL1HAD")

    return CfgMgr.LVL1__LArTTL1Overlay(name, **kwargs)


def getTileTTL1Overlay(name="TileTTL1Overlay", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags
    if overlayFlags.isOverlayMT():
        kwargs.setdefault("BkgTileTTL1Key", overlayFlags.bkgPrefix() + "TileTTL1Cnt")
        kwargs.setdefault("SignalTileTTL1Key", overlayFlags.sigPrefix() + "TileTTL1Cnt")
        kwargs.setdefault("OutputTileTTL1Key", "TileTTL1Cnt")
        kwargs.setdefault("BkgTileMBTSTTL1Key", overlayFlags.bkgPrefix() + "TileTTL1MBTS")
        kwargs.setdefault("SignalTileMBTSTTL1Key", overlayFlags.sigPrefix() + "TileTTL1MBTS")
        kwargs.setdefault("OutputTileMBTSTTL1Key", "TileTTL1MBTS")
    else:
        kwargs.setdefault("BkgTileTTL1Key", overlayFlags.dataStore() + "+TileTTL1Cnt")
        kwargs.setdefault("SignalTileTTL1Key", overlayFlags.evtStore() + "+TileTTL1Cnt")
        kwargs.setdefault("OutputTileTTL1Key", overlayFlags.outputStore() + "+TileTTL1Cnt")
        kwargs.setdefault("BkgTileMBTSTTL1Key", overlayFlags.dataStore() + "+TileTTL1MBTS")
        kwargs.setdefault("SignalTileMBTSTTL1Key", overlayFlags.evtStore() + "+TileTTL1MBTS")
        kwargs.setdefault("OutputTileMBTSTTL1Key", overlayFlags.outputStore() + "+TileTTL1MBTS")

    return CfgMgr.LVL1__TileTTL1Overlay(name, **kwargs)
