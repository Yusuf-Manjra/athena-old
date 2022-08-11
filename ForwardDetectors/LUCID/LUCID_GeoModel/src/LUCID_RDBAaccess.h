/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LUCID_RDBAaccess_h
#define LUCID_RDBAaccess_h

#include "GaudiKernel/Algorithm.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"

class LUCID_RDBAccess { 

 public: 
  
  LUCID_RDBAccess();  
  ~LUCID_RDBAccess(); 
  
  void ReadDB(); 
  void SetParameters(); 
  void Print() const;
  static void CloseDB(); 
  
  double distanceToIP{};  

  double VJdistanceToIP{};
  double VJconelength{};
  double VJconeRadiusFront{};
  double VJconeRadiusBack{};
  double VJconeThickness{};
  double VJconeFrontRingThickness{};
  double VJconeFrontRingLength{};
  double VJconeFrontRingOverlap{};

  double cylinderLength{};  
  double cylinderRadius{};  
  double cylinderThickness{};  
  double pmtPosition{};  
  double pmtRadius{};  
  double pmtThickness{};  

  double quartzDensity{}; 

  double waveLengthStep{};  
  double waveLengthMin{};  
  double waveLengthMax{};  

 private: 
  
  IRDBRecordset_ptr m_lucidParams; 
  ISvcLocator* m_svcLocator{}; 
}; 

#endif
