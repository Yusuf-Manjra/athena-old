#**************   ESD list  ************************************************
from CaloRec.CaloRecFlags import jobproperties
from AthenaCommon.JobProperties import jobproperties
from AthenaCommon.Logging import logging

CaloESDList = []

CaloESDList += [ "CaloCellContainer#AllCalo" ]

# add explicitly E4', MBTS cells and trigger output to ESD
CaloESDList += [ "TileCellContainer#E4prContainer" ]
CaloESDList += [ "TileCellContainer#MBTSContainer" ]
CaloESDList += [ "TileTTL1Container#TileTTL1MBTS" ]

# explicitly list of CaloCluster related keys
CaloClusterItemList=[]
CaloClusterKeys=[]

from RecExConfig.AutoConfiguration import IsInInputFile

storedFwdTowersInESD = IsInInputFile("xAOD::CaloClusterContainer","CaloCalFwdTopoTowers")

CaloClusterKeys+=["CaloCalTopoClusters"]
if jobproperties.CaloRecFlags.doCaloTopoTower.get_Value():
    CaloClusterKeys+=["CaloCalTopoTowers"]
if jobproperties.CaloRecFlags.doCaloTopoSignal.get_Value():
    CaloClusterKeys+=["CaloCalTopoSignals"]
if jobproperties.CaloRecFlags.doCaloFwdTopoTower.get_Value() or storedFwdTowersInESD:
    CaloClusterKeys+=["CaloCalFwdTopoTowers"]

CaloClusterKeys+=["CombinedCluster"]
#CaloClusterKeys+=["EMTopoCluster430"]
CaloClusterKeys+=["EMTopoSW35"]

# reshuffl em 
#CaloClusterKeys+=["LArClusterEM"]

#CaloClusterKeys+=["LArClusterEM7_11Nocorr"]
#CaloClusterKeys+=["egClusterCollection"] Move to eg output list
#CaloClusterKeys+=["LArClusterEMSofte"]

if rec.Commissioning():
    # CaloClusterKeys += ["LArMuClusterCandidates","CaloTopoClusters","EMTopoCluster420","TileTopoCluster","MuonClusterCollection"]
    CaloClusterKeys += ["MuonClusterCollection"]    

for theKey in CaloClusterKeys:
    CaloClusterItemList+=["xAOD::CaloClusterContainer#"+theKey]
    CaloClusterItemList+=["xAOD::CaloClusterAuxContainer#"+theKey+"Aux."]
    CaloClusterItemList+=["CaloClusterCellLinkContainer#"+theKey+"_links"]

CaloESDList+=CaloClusterItemList
logRecoOutputItemList_jobOptions.info('CaloClusterItemList = ')
printfunc (CaloClusterItemList)

#CaloTowerContainer needed for jet->cell
#FIXME
CaloESDList += [ "CaloTowerContainer#CombinedTower" ]

# LAr thinned digits
CaloESDList += ["LArDigitContainer#LArDigitContainer_Thinned"]

# LAr FEB error summary data 
CaloESDList += ["LArFebErrorSummary#LArFebErrorSummary"]

# LAr thinned digits for LArMuId
if jobproperties.Beam.beamType() == "cosmics" :
    CaloESDList += ["LArDigitContainer#LArDigitContainer_IIC"]
# LAr thinned digits from EM clusters
CaloESDList += ["LArDigitContainer#LArDigitContainer_EMClust"]

# LAr noisy Feb/PA summary
CaloESDList += ["LArNoisyROSummary#LArNoisyROSummary"]

# Tile filtered digits, muons reconstructed offline or inside ROD
CaloESDList += ["TileDigitsContainer#TileDigitsFlt"]
CaloESDList += ["TileRawChannelContainer#TileRawChannelFlt"]
CaloESDList += ["TileMuContainer#TileMuObj"]
CaloESDList += ["TileL2Container#TileL2Cnt"]

# Tile TMDB output
CaloESDList += ["TileDigitsContainer#MuRcvDigitsCnt"]
CaloESDList += ["TileRawChannelContainer#MuRcvRawChCnt"]
CaloESDList += ["TileMuonReceiverContainer#TileMuRcvCnt"]

# Cosmic muons reconstructed with Tile muon fitter
if jobproperties.Beam.beamType() == 'cosmics' or jobproperties.Beam.beamType() == 'singlebeam' :
    CaloESDList +=["TileCosmicMuonContainer#TileCosmicMuonHT","TileCosmicMuonContainer#TileCosmicMuonMF"]


#**************   AOD list  ************************************************

#List of AOD moments: (copied from CaloClusterTopoGetter)

AODMoments=[ "SECOND_R" 
            ,"SECOND_LAMBDA"
            ,"CENTER_MAG"
            ,"CENTER_LAMBDA"
            ,"FIRST_ENG_DENS"
            ,"ENG_FRAC_MAX" 
            ,"ISOLATION"
            ,"ENG_BAD_CELLS"
            ,"N_BAD_CELLS"
            ,"BADLARQ_FRAC"
            ,"ENG_POS"
            ,"SIGNIFICANCE"
            ,"AVG_LAR_Q"
            ,"AVG_TILE_Q"
            ,"EM_PROBABILITY"
            ,"BadChannelList"
            ,"SECOND_TIME"
            ,"NCELL_SAMPLING" 
            ]

if jobproperties.CaloRecFlags.doExtendedClusterMoments.get_Value():
    AODMoments += ["LATERAL"
                   ,"LONGITUDINAL"
                   ,"CELL_SIGNIFICANCE"
                   ,"PTD"
                   ,"MASS"
                   ]

    if jobproperties.Rec.doHeavyIon() or jobproperties.Rec.doHIP():
        AODMoments += ["CELL_SIG_SAMPLING"]

try:
    from Digitization.DigitizationFlags import digitizationFlags
    if digitizationFlags.doDigiTruth():

      AODMoments+=["SECOND_R_DigiHSTruth"
                ,"SECOND_LAMBDA_DigiHSTruth"
                ,"CENTER_MAG_DigiHSTruth"
                ,"CENTER_LAMBDA_DigiHSTruth"
                ,"FIRST_ENG_DENS_DigiHSTruth"
                ,"ISOLATION_DigiHSTruth"
                ,"ENG_POS_DigiHSTruth"
                ,"AVG_LAR_Q_DigiHSTruth"
                ,"AVG_TILE_Q_DigiHSTruth"
                ,"ENERGY_DigiHSTruth"
                ,"ETA_DigiHSTruth"
                ,"PHI_DigiHSTruth"
                ]
      if jobproperties.CaloRecFlags.doExtendedClusterMoments.get_Value():
            AODMoments+=[ "SIGNIFICANCE_Truth"
                         ,"CELL_SIGNIFICANCE_Truth"
                         ,"PTD_Truth"
                 ]
            if jobproperties.Rec.doHeavyIon() or jobproperties.Rec.doHIP():
                AODMoments += ["CELL_SIG_SAMPLING"]

except:
    log = logging.getLogger('CaloRecOutputItemList')
    log.info('Unable to import DigitizationFlags in CaloRecOutputItemList_jobOptions. Expected in AthenaP1')


CaloAODList = []

# cells 
CaloAODList+=["CaloCellContainer#AODCellContainer"]

# CaloClusters 
CaloClusterItemList=[]
CaloClusterKeys=[]


CaloClusterKeys+=["CaloCalTopoClusters"]
if jobproperties.CaloRecFlags.doCaloTopoTower.get_Value():
    CaloClusterKeys+=["CaloCalTopoTowers"]
if jobproperties.CaloRecFlags.doCaloTopoSignal.get_Value():
    CaloClusterKeys+=["CaloCalTopoSignals"]


CaloClusterKeys+=["CombinedCluster"]
#CaloClusterKeys+=["EMTopoCluster430"]
CaloClusterKeys+=["EMTopoSW35"]

#CaloClusterKeys+=["egClusterCollection"] Moved to eg output item list
#CaloClusterKeys+=["LArClusterEMSofte"]



for theKey in CaloClusterKeys: #Fixme .. Apply this only to TopoClusters?
     CaloClusterItemList+=["xAOD::CaloClusterContainer#"+theKey]
     AuxListItem="xAOD::CaloClusterAuxContainer#"+theKey+"Aux"
     for moment in AODMoments:
         AuxListItem+="."+moment
     # for tau clusters
     if theKey == "CaloCalTopoClusters":
         AuxListItem += ".CellLink"
     if len(AODMoments)==0: AuxListItem+="." 
     CaloClusterItemList+=[AuxListItem]

# write the link only for egClusterColl
#CaloClusterItemList+=["CaloClusterCellLinkContainer#egClusterCollection_links"]

# for tau clusters (CaloCalTopoClusters within 0.2 of the tau axis)
CaloClusterItemList += ["CaloClusterCellLinkContainer#CaloCalTopoClusters_links"]

#CaloCalFwdTopoClusters are also clusters but with a dedicated sliming:
if jobproperties.CaloRecFlags.doCaloFwdTopoTower.get_Value() or storedFwdTowersInESD:
    theKey="CaloCalFwdTopoTowers"
    CaloClusterItemList+=["xAOD::CaloClusterContainer#"+theKey]
    AuxListItem="xAOD::CaloClusterAuxContainer#"+theKey+"Aux"
    for moment in ("CENTER_LAMBDA", 
                   #"CENTER_MAG",
                   "LONGITUDINAL",
                   #"FIRST_ENG_DENS",
                   #"ENG_FRAC_MAX",
                   "ENG_FRAC_EM",
                   #"PTD",
                   "SIGNIFICANCE",
                   "ENG_POS"):
         AuxListItem+="."+moment
    CaloClusterItemList+=[AuxListItem]

CaloAODList+=CaloClusterItemList

# E4' cells
CaloAODList+=["TileCellContainer#E4prContainer"]

# MBTS cells
CaloAODList+=["TileCellContainer#MBTSContainer"]

# muons in Tile
CaloAODList+=["TileMuContainer#TileMuObj"]

# LAr noisy Feb/PA summary
CaloAODList +=  ["LArNoisyROSummary#LArNoisyROSummary"]

