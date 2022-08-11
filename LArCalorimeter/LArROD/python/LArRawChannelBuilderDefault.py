# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from LArROD.LArRODFlags import larRODFlags
from AthenaCommon.GlobalFlags import globalflags
from LArByteStream.LArByteStreamConf import LArRawDataReadingAlg

def LArRawChannelBuilderDefault(forceIter=False):
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()

    if larRODFlags.readDigits() and globalflags.InputFormat() == 'bytestream':
        

        from LArRecUtils.LArADC2MeVCondAlgDefault import LArADC2MeVCondAlgDefault

        LArADC2MeVCondAlgDefault()

        
        from LArConditionsCommon.LArRunFormat import getLArFormatForRun
        from RecExConfig.AutoConfiguration import GetRunNumber
        runNum = GetRunNumber()
        if runNum is not None:
            lri=getLArFormatForRun(runNum)
        else:
            lri=None


        if not forceIter:
            if lri is not None and lri.runType() is not None and lri.runType()==0:
                forceIter=True

        if LArRawDataReadingAlg() not in topSequence:
            print ("Adding LArRawDataReaderAlg")
            topSequence+=LArRawDataReadingAlg()

        if (lri and lri.runType()==0): topSequence.LArRawDataReadingAlg.LArRawChannelKey=""
        
            
       
        if forceIter:
           from LArROD.LArRODConf import LArRawChannelBuilderIterAlg
           theLArRawChannelBuilder=LArRawChannelBuilderIterAlg()
           theLArRawChannelBuilder.minSample=2
           theLArRawChannelBuilder.maxSample=12
           theLArRawChannelBuilder.minADCforIterInSigma=4
           theLArRawChannelBuilder.minADCforIter=15
           theLArRawChannelBuilder.defaultPhase=12
           #from AthenaCommon.Constants import DEBUG
           #theLArRawChannelBuilder.OutputLevel=DEBUG
        else:
           from LArROD.LArRODConf import LArRawChannelBuilderAlg
           theLArRawChannelBuilder=LArRawChannelBuilderAlg()
        if larRODFlags.keepDSPRaw():
            theLArRawChannelBuilder.LArRawChannelKey=larRODFlags.RawChannelFromDigitsContainerName()

        obj = "AthenaAttributeList"
        db = 'LAR_ONL'
        if globalflags.DataSource() == 'data':
            from IOVDbSvc.CondDB import conddb
            if conddb.GetInstance() == 'COMP200':
                fld='/LAR/Configuration/DSPThreshold/Thresholds'
                theLArRawChannelBuilder.Run1DSPThresholdsKey='LArDSPThresholds'
                obj='LArDSPThresholdsComplete'
            else:
                fld="/LAR/Configuration/DSPThresholdFlat/Thresholds"
                theLArRawChannelBuilder.Run2DSPThresholdsKey=fld
            conddb.addFolder (db, fld, className=obj)
        else:   
           from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags
           if CommonGeometryFlags.Run() == "RUN1": # back to flat threshold
                theLArRawChannelBuilder.useDB = False
                theLArRawChannelBuilder.Run2DSPThresholdsKey=''
           else:
                fld="/LAR/NoiseOfl/DSPThresholds"
                theLArRawChannelBuilder.Run2DSPThresholdsKey=fld
                db = 'LAR_OFL'
                conddb.addFolder (db, fld, className=obj)

        topSequence += theLArRawChannelBuilder

