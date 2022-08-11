# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration


from AthenaCommon.Include import include
from IOVDbSvc.CondDB import conddb

if conddb.isMC:
    include("LArConditionsCommon/LArConditionsCommon_MC_jobOptions.py")
else:
    include("LArConditionsCommon/LArConditionsCommon_comm_jobOptions.py")

from LArRecUtils.LArRecUtilsConf import LArADC2MeVCondAlg 
from AthenaCommon.AlgSequence import AthSequencer
from LArCabling.LArCablingAccess import LArOnOffIdMappingSC
condSeq = AthSequencer("AthCondSeq")


def LArADC2MeVSCCondAlgDefault (name="LArADC2MeVSCCondAlgDefault"): 

    LArOnOffIdMappingSC()
    condSeq = AthSequencer("AthCondSeq")
    if hasattr (condSeq,"LArADC2MeVSCCondAlg"):
        return getattr(condSeq,"LArADC2MeVSCCondAlg")


    theADC2MeVCondAlg=LArADC2MeVCondAlg(LArADC2MeVKey = 'LArADC2MeVSC',isSuperCell=True)
 
    if conddb.isMC:
        from LArConditionsCommon.LArCondFlags import larCondFlags 
        if not larCondFlags.hasMphys():
            theADC2MeVCondAlg.LArMphysOverMcalKey="" #No MphysOVerMcal
        else:
            theADC2MeVCondAlg.LArMphysOverMcalKey="LArMphysOverMcalSym"

        if not larCondFlags.hasHVCorr():
            theADC2MeVCondAlg.LArHVScaleCorrKey=""
              
        theADC2MeVCondAlg.LAruA2MeVKey="LAruA2MeVSC"
        theADC2MeVCondAlg.LArDAC2uAKey="LArDAC2uASC"
        theADC2MeVCondAlg.LArRampKey="LArRampSC"

        theADC2MeVCondAlg.UseFEBGainTresholds=False
        
    else: # not MC
        from LArRecUtils.LArFebConfigCondAlgDefault import LArFebConfigCondAlgDefault
        LArFebConfigCondAlgDefault()
        
    condSeq+=theADC2MeVCondAlg
    return theADC2MeVCondAlg



