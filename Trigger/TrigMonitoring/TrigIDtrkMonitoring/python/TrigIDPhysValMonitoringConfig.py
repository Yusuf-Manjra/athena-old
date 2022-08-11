# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

def TrigIDPhysValMonitoringTool( legacy_monitoring=False ):

  # dataTypes: userDefined = 0, monteCarlo, collisions, cosmics
  if 'DQMonFlags' not in dir():
    from AthenaMonitoring.DQMonFlags import DQMonFlags   # noqa: F401

  # disable everything
  outputlist = []

  if 'rec' not in dir():
    from RecExConfig.RecFlags  import rec
    
  from AthenaConfiguration.AllConfigFlags import ConfigFlags
      
  mt_chains = True
  if ( ConfigFlags.Trigger.EDMVersion < 3 or legacy_monitoring ) :
    mt_chains = False
        
  if rec.doInDet:
    from TrigInDetAnalysisExample.TrigInDetAnalysisExampleConf import TrigTestPhysValMon
    from AthenaCommon.AppMgr import release_metadata
    d = release_metadata()

    def makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic = False, useOffline = False, doFS=False ):

      Monname = "TestIDPhysValMon" + name
      TestIDPhysValMon = TrigTestPhysValMon(name=Monname)
      TestIDPhysValMon.SliceTag = "HLT/IDMon/" + name
      TestIDPhysValMon.UseHighestPT = useHighestPT

      try:
        TestIDPhysValMon.EnableLumi = False
      except Exception:
        pass

      TestIDPhysValMon.buildNtuple = False
      TestIDPhysValMon.AnalysisConfig = "Tier0" #T0 Analysis
      # TestIDPhysValMon.AnalysisConfig = "nTuple" #nTuple Analysis

#     if (useOffline or rec.doTruth is False): # is rec.doTruth set correctly ?? 
      if (useOffline is True):
        TestIDPhysValMon.mcTruth = False
        TestIDPhysValMon.ntupleChainNames = ['Offline',name]
#       use default values ? 
#       TestIDPhysValMon.sctHitsOffline = 1
#       TestIDPhysValMon.pixHitsOffline = 1
        if (doFS is True):
          TestIDPhysValMon.sctHitsOffline = 6
          TestIDPhysValMon.pixHitsOffline = 4
          TestIDPhysValMon.blayerHitsOffline = 1
          TestIDPhysValMon.pixHolesOffline = 1
#     elif (rec.doTruth is True):
      elif (useOffline is False): # again is rec.doTruth being set correctly ?? 
        TestIDPhysValMon.mcTruth = True
        TestIDPhysValMon.ntupleChainNames = ['Truth']
        TestIDPhysValMon.sctHitsOffline = -1
        TestIDPhysValMon.pixHitsOffline = -1
        if (pdgid != 0): TestIDPhysValMon.SelectTruthPdgId = pdgid
      elif (cosmic):
        TestIDPhysValMon.mcTruth = False
        TestIDPhysValMon.ntupleChainNames = ['Offline']
        TestIDPhysValMon.sctHitsOffline = -1
        TestIDPhysValMon.pixHitsOffline = -1

      TestIDPhysValMon.ntupleChainNames += chainnames
      TestIDPhysValMon.releaseMetaData = d['nightly name'] + " " + d['nightly release'] + " " + d['date'] + " " + d['platform'] + " " + d['release']

      #from AthenaCommon.AppMgr import ToolSvc
      #ToolSvc += TestIDPhysValMon
      #Monname = "TrigTestPhysValMon/" + Monname
      return TestIDPhysValMon

    ############### Electrons ###############
    name = "Electron"
    pdgid = 11
    useHighestPT = True
    if mt_chains:
      chainnames = [
        "HLT_e.*idperf.*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",
        "HLT_e.*idperf.*:key=HLT_IDTrack_Electron_IDTrig",
        "HLT_e.*etcut.*:key=HLT_IDTrack_Electron_FTF:roi=HLT_Roi_FastElectron",
        "HLT_e.*etcut.*:key=HLT_IDTrack_Electron_IDTrig",
        "HLT_e.*gsf.*:key=HLT_IDTrack_Electron_GSF"
      ]
    else:
      chainnames = [
        "HLT_e.*idperf.*:InDetTrigTrackingxAODCnv_Electron_IDTrig",
        "HLT_e.*idperf.*:InDetTrigTrackingxAODCnv_Electron_FTF"
      ]
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]

    name = "Electron_offline"
    pdgid = 11
    useHighestPT = True
    cosmic=False
    useOffline=True

    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]



    ############### LRT Electrons ###############
    name = "Electron_LRT"
    pdgid = 11
    useHighestPT = True
    if mt_chains:
      chainnames = [  
        "HLT_e.*idperf_loose_lrtloose.*:HLT_IDTrack_ElecLRT_FTF:HLT_Roi_FastElectron_LRT", 
        "HLT_e.*idperf_loose_lrtloose.*:HLT_IDTrack_ElecLRT_IDTrig:HLT_Roi_FastElectron_LRT" 
      ]  

    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )] 


    name = "Electron_LRT_offline"
    pdgid = 11
    useHighestPT = True      
    cosmic=False          
    useOffline=True                
    
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]  


    ############### Muons ###############
    name = "Muon"
    pdgid = 13
    useHighestPT = True
    if mt_chains:
      chainnames = [
        "HLT_mu.*idperf.*:key=HLT_IDTrack_Muon_FTF:roi=HLT_Roi_L2SAMuon",
        "HLT_mu.*idperf.*:key=HLT_IDTrack_Muon_IDTrig:roi=HLT_Roi_L2SAMuon",
        "HLT_mu.*i.*:key=HLT_IDTrack_MuonIso_FTF:roi=HLT_Roi_MuonIso",
        "HLT_mu.*i.*:key=HLT_IDTrack_MuonIso_IDTrig:roi=HLT_Roi_MuonIso"
      ]
    else:
      chainnames = [
        "HLT_mu.*idperf.*:InDetTrigTrackingxAODCnv_Muon_IDTrig",
        "HLT_mu.*idperf.*:InDetTrigTrackingxAODCnv_Muon_FTF"
      ]

    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]


    name = "Muon_offline"
    pdgid = 13
    useHighestPT = True
    cosmic=False
    useOffline=True

    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]


    ############### LRT Muons ###############
    name = "Muon_LRT"
    pdgid = 13
    useHighestPT = True
    if mt_chains:
      chainnames = [
        "HLT_mu.*LRT.*:HLT_IDTrack_MuonLRT_FTF:HLT_Roi_L2SAMuon_LRT", 
        "HLT_mu.*LRT.*:HLT_IDTrack_MuonLRT_IDTrig:HLT_Roi_L2SAMuon_LRT"
      ]
      
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]                                                                           

    name = "Muon_LRT_offline"
    pdgid = 13
    useHighestPT = True 
    cosmic=False 
    useOffline=True 
    
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]     


    ############### Taus ###############
    name = "Tau"
    pdgid = 15
    useHighestPT = True
    if mt_chains:
      chainnames = [
        "HLT_tau.*idperf.*tracktwo.*:key=HLT_IDTrack_TauCore_FTF:roi=HLT_Roi_TauCore",
        "HLT_tau.*idperf.*tracktwo.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIso",
        "HLT_tau.*idperf.*tracktwo.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIso",
        "HLT_tau.*idperf.*tracktwo.*BDT.*:key=HLT_IDTrack_TauIso_FTF:roi=HLT_Roi_TauIsoBDT",
        "HLT_tau.*idperf.*tracktwo.*BDT.*:key=HLT_IDTrack_Tau_IDTrig:roi=HLT_Roi_TauIsoBDT"
      ]
    else:
      chainnames = [
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_Tau_IDTrig:roi=forID3",
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_Tau_FTF:roi=forID",
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_TauCore_FTF:roi=forID1",
        "HLT_tau.*idperf.*:key=InDetTrigTrackingxAODCnv_TauIso_FTF:roi=forID3"
      ]
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]


    name = "Tau_offline"
    pdgid = 15
    useHighestPT = True
    cosmic=False
    useOffline=True

    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]

    ############### Bjets ###############
    name = "Bjet_offline"
    pdgid = 5
    useHighestPT = False
    cosmic=False
    useOffline=True
    if mt_chains:
      chainnames = [
        "HLT_j45_pf_ftf_preselj20_L1J15:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
        "HLT_j.*_ftf.*boffperf.*:key=HLT_IDTrack_FS_FTF:roi=HLT_FSRoI:vtx=HLT_IDVertex_FS",
        "HLT_j.*b.*perf.*:key=HLT_IDTrack_Bjet_FTF",
        "HLT_j.*b.*perf.*:key=HLT_IDTrack_Bjet_IDTrig"
      ]
    else:
      chainnames = [
        "HLT_j.*b.*perf_split:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=TrigSuperRoi",
        "HLT_j.*b.*perf_split:InDetTrigTrackingxAODCnv_Bjet_IDTrig",
        "HLT_j.*b.*perf_split:InDetTrigTrackingxAODCnv_Bjet_FTF",
        "HLT_mu.*b.*perf_dr05:key=InDetTrigTrackingxAODCnv_BjetPrmVtx_FTF:roi=TrigSuperRoi",
        "HLT_mu.*b.*perf_dr05:InDetTrigTrackingxAODCnv_Bjet_IDTrig",
        "HLT_mu.*b.*perf_dr05:InDetTrigTrackingxAODCnv_Bjet_FTF"
      ]
      
    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]
      


    ############### Bjets ###############
    name = "Bjet"
    useHighestPT = False

    outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]
      

    if not mt_chains:
      ############### Bphys ###############
      name = "Bphys"
      pdgid = 0 # Doesn't make sense
      useHighestPT = False
      chainnames = [
        "HLT_.*Bmumux.*:InDetTrigTrackingxAODCnv_Bphysics_IDTrig",
        "HLT_.*Bmumux.*:InDetTrigTrackingxAODCnv_Bphysics_FTF"
      ]
      outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT )]
      
      ############### Bphys ###############
      name = "Bphys_offline"
      pdgid = 0 # Doesn't make sense
      useHighestPT = False
      cosmic=False
      useOffline=True
      chainnames = [
        "HLT_.*Bmumux.*:InDetTrigTrackingxAODCnv_Bphysics_IDTrig",
        "HLT_.*Bmumux.*:InDetTrigTrackingxAODCnv_Bphysics_FTF"
      ]
      outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic, useOffline )]
      
      ############### Cosmics ###############
      name = "Cosmic"
      useHighestPT = False
      pdgid = 0 # Not used for cosmic
      chainnames = [
        'HLT_.*id.*cosmic.*:InDetTrigTrackingxAODCnvIOTRT_CosmicsN_EFID',
        'HLT_.*id.*cosmic.*:InDetTrigTrackingxAODCnv_CosmicsN_EFID'
      ]
      outputlist += [makePhysvalMon(name, pdgid, chainnames, useHighestPT, cosmic=True)]
      
  return outputlist

