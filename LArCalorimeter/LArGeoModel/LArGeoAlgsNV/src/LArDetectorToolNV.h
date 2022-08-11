/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file LArDetectorToolNV.h
 *
 * @brief Declaration of LArDetectorToolNV class
 *
 */

#ifndef LARGEOALGSNV_LARDETECTORTOOLNV_H
#define LARGEOALGSNV_LARDETECTORTOOLNV_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "AthenaKernel/IOVSvcDefs.h"

#include "LArGeoCode/LArAlignHelper.h"

class LArDetectorManager;

/**
 * @class LArDetectorToolNV
 *
 * @brief LArDetectorToolNV is a standard GeoModel tool, which calls LArDetectorFactory::create(),
 * stores LArDetectorManager to the Detector Store and also registers a callback function
 * align() which applies misalignments on top of the 'regular' geometry.
 *
 **/

class LArDetectorToolNV final : public GeoModelTool {
 public:
  // Standard Constructor
  LArDetectorToolNV( const std::string& type, const std::string& name, const IInterface* parent );
  
  // Standard Destructor
  virtual ~LArDetectorToolNV();
  
  // Create Method:
  virtual StatusCode create() override;
  
  // Clear Method
  virtual StatusCode clear() override;
  
  // Register callback function on ConDB object
  virtual StatusCode registerCallback() override;
  
  // Callback function itself
  virtual StatusCode align(IOVSVC_CALLBACK_ARGS) override;

 private:
  bool m_barrelSaggingOn;
  int  m_barrelVisLimit;
  int  m_fcalVisLimit;
  
  bool m_buildBarrel;
  bool m_buildEndcap;
  
  bool m_applyAlignments;
  
  const LArDetectorManager *m_manager;
  
  std::string m_geometryConfig; // FULL, SIMU, RECO
  
  std::string m_EMECVariantInner;
  std::string m_EMECVariantOuter;
  
  bool m_activateFT;
  bool m_enableMBTS;

  LArAlignHelper m_alignHelper;
};

#endif
