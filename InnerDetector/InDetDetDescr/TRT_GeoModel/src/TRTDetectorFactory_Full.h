/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------//
//                                                                             //
//  TRTDetectorFactory_Full                                                    //
//                                                                             //
//  Authors:  Joe Boudreau, Andrei and Iouris Zalite, Thomas Kittelmann        //
//                                                                             //
//  This class creates the TRT Geometry. Cf. the README file                   //
//                                                                             //
//-----------------------------------------------------------------------------//

#ifndef TRTDetectorFactory_Full_h
#define TRTDetectorFactory_Full_h 1
#include "GeoPrimitives/GeoPrimitives.h"
#include "InDetGeoModelUtils/InDetDetectorFactoryBase.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"

#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h" //for Argon
#include "GeoModelKernel/GeoDefinitions.h"
#include "CxxUtils/checker_macros.h"

#include <string>

class GeoPhysVol;
class GeoFullPhysVol;
class TRTParameterInterface;
class InDetMaterialManager;

class TRTDetectorFactory_Full : public InDetDD::DetectorFactoryBase  {

 public:
  
  //--------------------------Public Interface:--------------------------------//
  //                                                                           //
  // Constructor:                                                              //
  TRTDetectorFactory_Full(InDetDD::AthenaComps * athenaComps,                  //
			  const ITRT_StrawStatusSummaryTool * sumTool,         //
			  bool useOldActiveGasMixture,                         //
			  bool DC2CompatibleBarrelCoordinates,                 //
			  int overridedigversion,                              //
			  bool alignable,                                      //
			  bool doArgon,                                        //
			  bool doKrypton,                                      //
			  bool useDynamicAlignmentFolders);                    //
  //                                                                           //
  // Destructor:                                                               //
  ~TRTDetectorFactory_Full();                                                  //
  //                                                                           //
  // Creation of geometry:                                                     //
  virtual void create(GeoPhysVol *world);                                      //
  //                                                                           //
  // Access to the results:                                                    //
  virtual const InDetDD::TRT_DetectorManager * getDetectorManager() const;     //
  //                                                                           //
  //---------------------------------------------------------------------------//

  const std::string& name() const { 
    static const std::string n("TRT_GeoModel::TRTDetectorFactory"); 
    return n;
  }

 typedef TRTCond::StrawStatusMultChanContainer StrawStatusContainer;
 private:  

  //---------------------------Illegal operations:---------------------------------//
  //                                                                               //
  const TRTDetectorFactory_Full & operator=(const TRTDetectorFactory_Full &right); //
  TRTDetectorFactory_Full(const TRTDetectorFactory_Full &right);                   //
  //                                                                               //
  //-------------------------------------------------------------------------------//

  // Gas mixture enumerator
  enum ActiveGasMixture
    {
    GM_XENON,
    GM_KRYPTON,
    GM_ARGON
    };

  ActiveGasMixture DecideGasMixture(int strawStatusHT);

  // private helper methods:
  const GeoShape * makeModule ( double length, const GeoTrf::Vector2D& corner1 ,  const GeoTrf::Vector2D& corner2, const GeoTrf::Vector2D& corner3,
				const GeoTrf::Vector2D& corner4, GeoTrf::Transform3D & absolutePosition, double shrinkDist=0 ) const;
  //GeoPhysVol * makeStraw( double& activeGasZPosition, bool hasLargeDeadRegion=false ) const;
  GeoPhysVol * makeStraw( double& activeGasZPosition, bool hasLargeDeadRegion=false, ActiveGasMixture gasMixture = GM_XENON);
  //GeoFullPhysVol  *makeStrawPlane( size_t w ) const;
  GeoFullPhysVol  *makeStrawPlane( size_t w , ActiveGasMixture gasMixture = GM_XENON);

  // private member data:
  InDetDD::TRT_DetectorManager                  *m_detectorManager = nullptr; // ownership handed to calleer.
  std::unique_ptr<InDetMaterialManager>         m_materialManager;
  std::unique_ptr<TRTParameterInterface>        m_data;

  bool m_useOldActiveGasMixture;
  bool m_DC2CompatibleBarrelCoordinates;
  int m_overridedigversion;
  bool m_alignable;
  const ITRT_StrawStatusSummaryTool* m_sumTool; // added for Argon
  bool m_strawsvcavailable;
  bool m_doArgon;
  bool m_doKrypton;
  bool m_useDynamicAlignFolders;

  GeoFullPhysVol* m_type1Planes[3] = {nullptr, nullptr, nullptr};
  GeoFullPhysVol* m_type2Planes[3] = {nullptr, nullptr, nullptr};
};

#endif // TRTDetectorFactory_Full_h
