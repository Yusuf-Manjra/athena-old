/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
@page CaloInterface_page CaloInterface
@author G.Unal

@section CaloInterface_CaloInterface Introduction

This package contains the interfaces of the various tools used in the calorimeter
reconstruction

@section CaloInterface_CaloInterface Class Overview
  
  The CaloInterface package contains of following classes:

  - ICaloBadChannelTool  : obsolete
  - ICaloCellMakerTool   : interface for cell making and correcting tools
  - ICaloCellSelector    : interface for generic cell selection tool
  - ICaloHVGeometryTool  : obsolete (for old HV correction)
  - ICaloHVScaleTool     : obsolete (for old HV corrections)
  - ICaloTowerBuilderToolBase.h  : interface for tower builder tools
  - ICellWeightTool.h            : interface base class for cell weight tools
  - IClusterCellWeightTool.h     : interface for tools weighting cells in cluster objects
  - IClusterClassificationTool.h  : interface for tools classifying cluster objects
  - IHadronicCalibrationTool.h    : extends ICellWeightTool for more refined hadronic calibration information
  - ICaloAffectedTool.h  : interface for tool providing affected region info for a given 4 momentum object
  - ICaloCellNormalizedQualityTool.h  : interface for tool computing a normalized pulse shape quality from the raw quality

*/
