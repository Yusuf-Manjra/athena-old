/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONANALYSISINTERFACES_MUONANALYSISINTERFACESDICT_H
#define MUONANALYSISINTERFACES_MUONANALYSISINTERFACESDICT_H

#if defined(__GCCXML__) and not defined(EIGEN_DONT_VECTORIZE)
#define EIGEN_DONT_VECTORIZE
#endif  // __GCCXML__

// Includes for the dictionary generation:

#include "MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h"
#include "MuonAnalysisInterfaces/IMuonEfficiencyScaleFactors.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h"
#include "MuonAnalysisInterfaces/IMuonLRTOverlapRemovalTool.h"

#endif  // MUONANALYSISINTERFACES_MUONANALYSISINTERFACESDICT_H
