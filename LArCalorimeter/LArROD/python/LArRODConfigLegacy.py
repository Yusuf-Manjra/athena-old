# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr, CfgGetter

def getLArRawChannelBuilder(name="LArRawChannelBuilder" , **kwargs):

    kwargs.setdefault('LArRawChannelKey', "LArRawChannels")

    from Digitization.DigitizationFlags import digitizationFlags
    if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
        from OverlayCommonAlgs.OverlayFlags import overlayFlags
        kwargs.setdefault('LArDigitKey', overlayFlags.bkgPrefix() + 'LArDigitContainer_MC')
    else:
        kwargs.setdefault('LArDigitKey', 'LArDigitContainer_MC')

    #Call required Cond-Algos:
    from LArRecUtils.LArAutoCorrTotalCondAlgDefault import  LArAutoCorrTotalCondAlgDefault
    from LArRecUtils.LArOFCCondAlgDefault import LArOFCCondAlgDefault
    from LArRecUtils.LArADC2MeVCondAlgDefault import LArADC2MeVCondAlgDefault

    #The order matters when running w/o MT
    LArADC2MeVCondAlgDefault()
    LArAutoCorrTotalCondAlgDefault()
    LArOFCCondAlgDefault()


    kwargs.setdefault("ShapeKey","LArShapeSym")

    from LArROD.LArRODFlags import larRODFlags
    kwargs.setdefault('firstSample',larRODFlags.firstSample())
    iovDbSvc=CfgGetter.getService("IOVDbSvc")
    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")
    condLoader=condSeq.CondInputLoader

    from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags
    if CommonGeometryFlags.Run() == "RUN1": # back to flat threshold
       kwargs.setdefault('useDB', False)
       kwargs.setdefault('Run2DSPThresholdsKey', '')
    else:
       fld="/LAR/NoiseOfl/DSPThresholds"
       kwargs.setdefault('Run2DSPThresholdsKey', fld)
       iovDbSvc.Folders.append(fld+"<db>COOLOFL_LAR/OFLP200</db>")
       condLoader.Load.append(("AthenaAttributeList",fld))
    
    return CfgMgr.LArRawChannelBuilderAlg(name, **kwargs)

def getLArRawChannelBuilder_DigiHSTruth(name="LArRawChannelBuilder_DigiHSTruth" , **kwargs):

    kwargs.setdefault('LArRawChannelKey', "LArRawChannels_DigiHSTruth")
    kwargs.setdefault('LArDigitKey', 'LArDigitContainer_DigiHSTruth')

    #Call required Cond-Algos:
    from LArRecUtils.LArAutoCorrTotalCondAlgDefault import  LArAutoCorrTotalCondAlgDefault
    from LArRecUtils.LArOFCCondAlgDefault import LArOFCCondAlgDefault
    from LArRecUtils.LArADC2MeVCondAlgDefault import LArADC2MeVCondAlgDefault

    #The order matters when running w/o MT
    LArADC2MeVCondAlgDefault()
    LArAutoCorrTotalCondAlgDefault()
    LArOFCCondAlgDefault()

    kwargs.setdefault("ShapeKey","LArShapeSym")
    from LArROD.LArRODFlags import larRODFlags
    kwargs.setdefault('firstSample',larRODFlags.firstSample())
    iovDbSvc=CfgGetter.getService("IOVDbSvc")
    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")
    condLoader=condSeq.CondInputLoader


    from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags
    if CommonGeometryFlags.Run() == "RUN1": # back to flat threshold
       kwargs.setdefault('useDB', False)
       kwargs.setdefault('Run2DSPThresholdsKey', '')
    else:
       fld="/LAR/NoiseOfl/DSPThresholds"
       kwargs.setdefault('Run2DSPThresholdsKey', fld)
       iovDbSvc.Folders.append(fld+"<db>COOLOFL_LAR/OFLP200</db>")
       condLoader.Load.append(("AthenaAttributeList",fld))
    
    return CfgMgr.LArRawChannelBuilderAlg(name, **kwargs)
