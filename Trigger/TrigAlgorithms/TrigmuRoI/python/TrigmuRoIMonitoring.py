# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def TrigmuRoIMonitoring(name = "TrigmuRoIMonitoring"):

    montool = GenericMonitoringTool(name, HistPath = name)

    montool.defineHistogram('RpcOutOfTime', type='TH1F', path='EXPERT', title="Distribution of the BCID difference for the Rpc RoI out of Time; (Muon RoI BCID - Event BCID)",xbins=21, xmin=-10.5, xmax=10.5)
    montool.defineHistogram('TgcOutOfTime', type='TH1F', path='EXPERT', title="Distribution of the BCID difference for the Tgc RoI out of Time; (Muon RoI BCID - Event BCID)",xbins=21, xmin=-10.5, xmax=10.5)
    montool.defineHistogram('EtaOutOfTime, PhiOutOfTime', type='TH2F', path='EXPERT', title="Eta vs Phi of the Mupon RoI out of time; Eta; Phi", xbins=108, xmin=-2.7, xmax=2.7, ybins=96, ymin=-3.1416, ymax=3.1416 )

    return montool
