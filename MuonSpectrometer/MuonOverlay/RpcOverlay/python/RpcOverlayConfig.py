# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr

def getRpcOverlay(name="RpcOverlay", **kwargs):
    from OverlayCommonAlgs.OverlayFlags import overlayFlags

    if overlayFlags.isOverlayMT():
        kwargs.setdefault("BkgInputKey", overlayFlags.bkgPrefix() + "RPC_DIGITS")
        kwargs.setdefault("SignalInputKey", overlayFlags.sigPrefix() + "RPC_DIGITS")
        kwargs.setdefault("OutputKey", "RPC_DIGITS")
    else:
        kwargs.setdefault("BkgInputKey", overlayFlags.dataStore() + "+RPC_DIGITS")
        kwargs.setdefault("SignalInputKey", overlayFlags.evtStore() + "+RPC_DIGITS")
        kwargs.setdefault("OutputKey", overlayFlags.outputStore() + "+RPC_DIGITS")

    return CfgMgr.RpcOverlay(name, **kwargs)
