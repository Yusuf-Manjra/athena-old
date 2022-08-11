# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from __future__ import print_function

from MuonCablingServers.MuonCablingServersConf import TGCcablingServerSvc
from AthenaCommon.GlobalFlags import globalflags
from AthenaCommon.AppMgr import ServiceMgr,theApp
from MuonByteStream.MuonByteStreamFlags import muonByteStreamFlags
from AthenaCommon.Configurable import Configurable

class TgcCablingServerConfig (TGCcablingServerSvc):

    def __init__(self,name = Configurable.DefaultName ):
        super(TgcCablingServerConfig ,self).__init__(name)

        # default in cxx is self.Atlas = True
        if ( globalflags.DetDescrVersion().startswith('DC1') or
             globalflags.DetDescrVersion().startswith('DC2') or
             globalflags.DetDescrVersion().startswith('DC3')) :
            self.Atlas = False
            print ("DC")

        if (muonByteStreamFlags.TgcDataType()=='m3' or
            muonByteStreamFlags.TgcDataType()=='oldSimulation') :
            self.Atlas = False
            if (muonByteStreamFlags.TgcDataType()=='m3'):
                print ("TgcDataType is set to m3")
            else:
                print ("TgcDataType is set to oldSim")


    def setDefaults(cls,handle):
        if hasattr(handle,'Atlas'):
            if handle.Atlas is not True:
                print ("TGCcabling Server uses the old TGC cabling schema")
            else:
                print ("TGCcabling Server uses the new 12-fold cabling schema")

if "TGCcablingInterface" not in theApp.Dlls:
    theApp.Dlls += [ "TGCcablingInterface" ]

ServiceMgr += TgcCablingServerConfig()
