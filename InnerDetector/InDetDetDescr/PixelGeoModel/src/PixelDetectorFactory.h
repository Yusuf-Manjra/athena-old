/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PixelDetectorFactory_h
#define PixelDetectorFactory_h 


#include "InDetGeoModelUtils/InDetDetectorFactoryBase.h" 
// readout includes:
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "ReadoutGeometryBase/InDetDD_Defs.h"
#include "CxxUtils/checker_macros.h"

class PixelSwitches;
class PixelGeometryManager;
class PixelGeoModelAthenaComps;

class PixelDetectorFactory : public InDetDD::DetectorFactoryBase {

 public:
  
  // Constructor:
  PixelDetectorFactory(PixelGeoModelAthenaComps * athenaComps,
		       const PixelSwitches & switches);
  
  // Destructor:
  ~PixelDetectorFactory();
  
  // Creation of geometry:
  virtual void create(GeoPhysVol *world);
  
  // Access to the results:
  virtual const InDetDD::PixelDetectorManager * getDetectorManager() const;


 private:  
  
  // Illegal operations:
  const PixelDetectorFactory & operator=(const PixelDetectorFactory &right);
  PixelDetectorFactory(const PixelDetectorFactory &right);
  // private data
  InDetDD::PixelDetectorManager         *m_detectorManager = nullptr; //ownership handed to caller
  std::unique_ptr<PixelGeometryManager>  m_geometryManager;

  void doChecks();

private:
  bool m_useDynamicAlignFolders;
  
};

// Class PixelDetectorFactory 
#endif


