# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Getter for LArDigit from LArHit for MC digitization

from AthenaCommon.Logging import logging
from RecExConfig.Configured import Configured

class LArDigitGetter (Configured) :
    _outputType = "LArDigitContainer"
    _outputType_DigiHSTruth = "LArDigitContainer_DigiHSTruth"
    _output = { _outputType : "LArDigitContainer_MC" }
    _output_DigiHSTruth = { _outputType_DigiHSTruth : "LArDigitContainer_DigiHSTruth"}

    def configure(self):
        mlog = logging.getLogger( 'LArDigitGetter.py::configure :' )
        mlog.info ('entering')

        from AthenaCommon.AlgSequence import AlgSequence
        job = AlgSequence()

        try:
            from AthenaCommon import CfgGetter
            # if using new scheme for pileup
            from Digitization.DigitizationFlags import digitizationFlags
            if digitizationFlags.doXingByXingPileUp():
                # Defined in LArDigitizationConfigLegacy.py
                job.PileUpToolsAlg.PileUpTools += [ CfgGetter.getPrivateTool("LArPileUpTool", checkType=True) ]
                job.PileUpToolsAlg.PileUpTools["LArPileUpTool"].DoDigiTruthReconstruction = digitizationFlags.doDigiTruth()
            else:
                # Defined in LArDigitizationConfigLegacy.py
                job += CfgGetter.getAlgorithm("digitmaker1", tryDefaultConfigurable=True)
                job += CfgGetter.getAlgorithm("LArHitEMapToDigitAlg")
                job.digitmaker1.LArPileUpTool.DoDigiTruthReconstruction = digitizationFlags.doDigiTruth()
                # if pileup or overlay
                from AthenaCommon.DetFlags import DetFlags
                from LArDigitization.LArDigitizationConfigLegacy import isOverlay
                from OverlayCommonAlgs.OverlayFlags import overlayFlags
                if DetFlags.pileup.LAr_on() or (isOverlay() and not overlayFlags.isOverlayMT()):
                    from AthenaCommon.AppMgr import ServiceMgr
                    # Defined in LArDigitizationConfigLegacy.py
                    ServiceMgr.PileUpMergeSvc.Intervals += [ CfgGetter.getPrivateTool("LArRangeEM", checkType=True) ]
                    ServiceMgr.PileUpMergeSvc.Intervals += [ CfgGetter.getPrivateTool("LArRangeHEC", checkType=True) ]
                    ServiceMgr.PileUpMergeSvc.Intervals += [ CfgGetter.getPrivateTool("LArRangeFCAL", checkType=True) ]
        except Exception:
            import traceback
            traceback.print_exc()
            mlog.error ("ERROR Problem with configuration")
        
        return True

    def outputKey(cls):
        return cls._output[cls._outputType]

    def outputKey_DigiHSTruth(cls):
        return cls._output_DigiHSTruth[cls._outputType_DigiHSTruth]

    def outputType(cls):
        return cls._outputType

    def outputTypeKey(self):
        return str(self.outputType()+"#"+self.outputKey())

    def outputTypeKey_DigiHSTruth(self):
        return str(self.outputType()+"#"+self.outputKey_DigiHSTruth())

