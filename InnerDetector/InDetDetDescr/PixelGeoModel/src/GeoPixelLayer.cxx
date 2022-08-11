/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPixelLayer.h"
#include "GeoPixelLadder.h"
#include "GeoPixelTMT.h"
#include "GeoPixelSimpleStaveSupport.h"
#include "GeoPixelDetailedStaveSupport.h"
#include "GeoPixelStaveRingServices.h"
#include "GeoPixelLadderServices.h"
#include "GeoPixelPigtail.h"
#include "GeoPixelSiCrystal.h"
#include "GeoPixelStaveRing.h"

#include "InDetGeoModelUtils/ExtraMaterial.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"

#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoPara.h"
#include "GeoModelKernel/GeoSimplePolygonBrep.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoTransform.h"

#include "GeoModelKernel/GeoTubs.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <memory>
#include <sstream>

GeoPixelLayer::GeoPixelLayer(InDetDD::PixelDetectorManager* ddmgr,
                             PixelGeometryManager* mgr)
  : GeoVPixelFactory (ddmgr, mgr),
    m_supportPhysA (nullptr),
    m_supportPhysC (nullptr),
    m_supportMidRing (nullptr),
    m_xformSupportA (nullptr),
    m_xformSupportC (nullptr),
    m_xformSupportMidRing (nullptr)
{
}

GeoVPhysVol* GeoPixelLayer::Build() {

  //
  // create a barrel layer
  //

  //
  // Ladder = TMT + Modules  (sometimes referred to as "active ladder" below)
  // LadderServices = Omega, cooling tube + fluid, glue, flat part of pigtail, connectors, type 0 cables 
  // 

  // Currently recognized stave layouts
  // 0 : Standard ATLAS
  // 1 : No ladder services
  // 2 : No ladder services or TMT
  // 3 : Rectangular services. 

  int staveLayout = m_gmt_mgr->PixelStaveLayout();
  // FIXME: Envelope needs to be checked.

  // Build the sensor first to use the same for all the module in the layer
  // Likewise the TMT and the "Ladder"
  bool isBLayer = false;
  if(m_gmt_mgr->GetLD() == 0) isBLayer = true;
  GeoPixelSiCrystal theSensor(m_DDmgr, m_gmt_mgr, isBLayer);
  std::unique_ptr<GeoPixelStaveSupport> staveSupport;
  if (staveLayout ==0 || staveLayout==1) {
    staveSupport = std::make_unique<GeoPixelTMT>( m_DDmgr, m_gmt_mgr );
  }
  if (staveLayout == 3) {
    staveSupport = std::make_unique<GeoPixelSimpleStaveSupport>( m_DDmgr, m_gmt_mgr );
  }
  else if (staveLayout >3 && staveLayout <7)
  {
    staveSupport = std::make_unique<GeoPixelDetailedStaveSupport>( m_DDmgr, m_gmt_mgr);
  }

  if (staveLayout >3 && staveLayout <7)
    {
      m_gmt_mgr->SetIBLPlanarModuleNumber(staveSupport->PixelNPlanarModule());
      m_gmt_mgr->SetIBL3DModuleNumber(staveSupport->PixelN3DModule());
    }

  if(!staveSupport)
    {
      m_gmt_mgr->msg(MSG::ERROR)<<"No stave support corresponding to the staveLayout "<<staveLayout<<" could be defined "<<endmsg; 
      return nullptr;
    }

  m_gmt_mgr->msg(MSG::INFO)<<"*** LAYER "<<m_gmt_mgr->GetLD()<<"  planar/3D modules : "<< staveSupport->PixelNPlanarModule()<<" "<<staveSupport->PixelN3DModule()<<endmsg;


  GeoPixelLadder pixelLadder(m_DDmgr, m_gmt_mgr, theSensor, staveSupport.get());

  //
  // layer radius, number of sectors and tilt used in various places
  //
  double layerRadius = m_gmt_mgr->PixelLayerRadius();
  double ladderTilt   = m_gmt_mgr->PixelLadderTilt();
  int nSectors = m_gmt_mgr->NPixelSectors();

  // Set numerology
  m_DDmgr->numerology().setNumPhiModulesForLayer(m_gmt_mgr->GetLD(),nSectors);
  m_DDmgr->numerology().setNumEtaModulesForLayer(m_gmt_mgr->GetLD(),m_gmt_mgr->PixelNModule());

  //
  // Calculate layerThicknessN: Thickness from layer radius to min radius of envelope
  // Calculate layerThicknessP: Thickness from layer radius to max radius of envelope
  //      
  double ladderHalfThickN = pixelLadder.thicknessN();
  double ladderHalfThickP = pixelLadder.thicknessP();
  double ladderHalfWidth = pixelLadder.width()/2;
  //std::cout << "ladderHalfThickN, ladderHalfThickP, ladderHalfWidth, ladderTilt, layerRadius:"
  //	      << ladderHalfThickN << " "<< ladderHalfThickP << " " << ladderHalfWidth 
  //	      << " " << ladderTilt << " " << layerRadius << std::endl;

  // Calculate distance of closest approach to a line following the surface of the ladder.
  double grad = -1/tan(std::abs(ladderTilt)); // Gradient of this line.
  // x1, y1 is the point on the center of ladder surface.
  double y1 = -ladderHalfThickN*sin(std::abs(ladderTilt));
  double x1 = -ladderHalfThickN*cos(std::abs(ladderTilt))+layerRadius;
  // x1, y2 is the point of closest approach.
  double y2 = (y1 - grad*x1)/(1+grad*grad);
  double x2 = -grad*y2;
  // dist is the distance between these two points. If this is further than the width of the ladder we 
  // can use the corner of the ladder.
  double distToClosestPoint = sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1));
  // distance of closest approach.
  double radClosest = (y1 - grad*x1)/(sqrt(1+grad*grad));

  //std::cout << "Distance of closest approach: " << radClosest << std::endl;
  //std::cout << "Distance along ladder surface from center to point of closest approach: " <<  distToClosestPoint << std::endl;

  // Calculate the radius of the corners of the ladder.
  GeoTrf::Vector3D ladderLowerCorner(-ladderHalfThickN, ladderHalfWidth, 0);
  GeoTrf::Vector3D ladderUpperCorner(ladderHalfThickP, -ladderHalfWidth, 0);
  ladderLowerCorner = GeoTrf::TranslateX3D(layerRadius) * GeoTrf::RotateZ3D(std::abs(ladderTilt)) * ladderLowerCorner;
  ladderUpperCorner = GeoTrf::TranslateX3D(layerRadius) * GeoTrf::RotateZ3D(std::abs(ladderTilt)) * ladderUpperCorner;

  double layerThicknessN = layerRadius - ladderLowerCorner.perp();
  double layerThicknessP = ladderUpperCorner.perp() - layerRadius; // Will be recalculated below in case of additional services
  //  std::cout<<"Max thickness : ladderhick "<<ladderHalfThickN<<"  "<<ladderHalfThickP<<std::endl;
  //  std::cout<<"Max thickness : layerthick "<<layerThicknessN<<"  "<<layerThicknessP<<std::endl;

  //std::cout << "Layer Envelope (using ladder corners): "
  //	    << layerRadius - layerThicknessN << " to " << layerRadius + layerThicknessP <<std::endl;

  // If distance of closest approach is within the ladder width we use that instead
  if (distToClosestPoint < ladderHalfWidth) {
    layerThicknessN = layerRadius - radClosest;
  }

  //std::cout << "Layer Envelope (ladder only):          " 
  //	    << layerRadius - layerThicknessN << " to " << layerRadius + layerThicknessP <<std::endl;

  //
  // Make ladder services and calculate envelope dimensions
  //
  // Variables that are used later
  int maxLadType = 0;
  std::vector<GeoVPhysVol *> ladderServicesArray;
  GeoTrf::Transform3D ladderServicesTransform(GeoTrf::Transform3D::Identity());
  GeoVPhysVol* pigtailPhysVol = nullptr;
  GeoTrf::Transform3D transPigtail(GeoTrf::Transform3D::Identity());
				
  // Only make services in non IBL geometries
  if (staveLayout == 0) {
    //
    // Make LadderServices 
    //
    // This volumes only needs to built a few times.
    // There are up to 4*2 types although we keep it general in case
    // more types are introduced.
    // The *2 as there are two cable orientations.
    
    // Get max ladder type
    //int maxLadType = 0;
    for(int iPhi = 0; iPhi < nSectors; iPhi++) {
      m_gmt_mgr->SetPhi(iPhi);
      int ladderType = m_gmt_mgr->PixelFluidOrient(m_gmt_mgr->GetLD(), iPhi);
      maxLadType = std::max(ladderType, maxLadType);
    }
    // Create the GeoPixelLadderServices types that are needed.
    // NB. vector initializes its contents to zero.
    //std::vector<GeoVPhysVol *> ladderServicesArray(2*(maxLadType+1));
    ladderServicesArray.resize(2*(maxLadType+1));
    GeoPixelLadderServices *firstLadderServices = nullptr;
    for(int iPhi = 0; iPhi < nSectors; iPhi++) {
      m_gmt_mgr->SetPhi(iPhi);
      int ladderType = m_gmt_mgr->PixelFluidOrient(m_gmt_mgr->GetLD(), iPhi);
      int biStave    = m_gmt_mgr->PixelBiStaveType(m_gmt_mgr->GetLD(), iPhi) % 2;  // Will be 1 or 2 -> Map to 1,0
      //int biStave    = iPhi % 2;  // Should only be 0 or 1
      if (ladderType < 0) std::cout << "ERROR: Unexpected value of ladderType: " << ladderType << std::endl;
      if (!ladderServicesArray[biStave*(maxLadType+1) + ladderType]) {
	GeoPixelLadderServices *ladderServices = new GeoPixelLadderServices(m_DDmgr, m_gmt_mgr, ladderType);
	// build the physical volume for each type
	ladderServicesArray[biStave*(maxLadType+1) + ladderType] = ladderServices->Build();
	if (!firstLadderServices) {
	  // We keep one as we use it later
	  firstLadderServices = ladderServices;
	} else {
	  // not needed once the physical volume is bult
	  delete ladderServices;
	}
      }
    }
    
    
    //
    // Calculate the layer envelope dimensions
    //
    
    //
    // Calculate layerThicknessP: Thickness from layer radius to max radius of envelope
    //

    double xCenter = 0.;
    double yCenter = 0.;
    if(firstLadderServices){
    
      // We need the dimensions of the GeoPixelLadderServices. They are all the same in this regards
      // so any of them will do -  use the first one.
      
      GeoTrf::Vector3D corner1 = firstLadderServices->envelopeCornerC1();
      GeoTrf::Vector3D corner2 = firstLadderServices->envelopeCornerC2();
      GeoTrf::Vector3D corner3 = firstLadderServices->envelopeCornerA1();
      GeoTrf::Vector3D corner4 = firstLadderServices->envelopeCornerA2();
      
      // translate relative to sensor center (center of tilt rotation),
      // then tilt then translate by radius of layer, then calculate r.
      double xLadderServicesOffset = m_gmt_mgr->PixelLadderServicesX();
      double yLadderServicesOffset = m_gmt_mgr->PixelLadderServicesY();
      // xCenter, yCenter or coordinates of ladder services relative to active layer center (center of tilt rotation)
      xCenter = (firstLadderServices->referenceX() + xLadderServicesOffset);
      yCenter = (firstLadderServices->referenceY() + yLadderServicesOffset);
      GeoTrf::Transform3D ladderSvcToglobal = GeoTrf::TranslateX3D(layerRadius) 
	* GeoTrf::RotateZ3D(ladderTilt) 
	* GeoTrf::Translate3D(xCenter, yCenter, 0);
      GeoTrf::Vector3D corner1global = ladderSvcToglobal * corner1;
      GeoTrf::Vector3D corner2global = ladderSvcToglobal * corner2;
      GeoTrf::Vector3D corner3global = ladderSvcToglobal * corner3;
      GeoTrf::Vector3D corner4global = ladderSvcToglobal * corner4;
      
      double rMaxTmp = std::max(corner1global.perp(), 
				std::max(corner2global.perp(), 
					 std::max(corner3global.perp(), corner4global.perp())));
      // Thickness from layer radius to max radius of envelope
      layerThicknessP = std::max(layerThicknessP, rMaxTmp - m_gmt_mgr->PixelLayerRadius());

      //std::cout << rMaxTmp << std::endl;
      //std::cout << layerThicknessP<< " "<<layerThicknessN <<std::endl;
      
      // No longer needed
      delete firstLadderServices;
      
    }
  
      
    //
    // Determine the position of the ladders and service volume.
    //
    // The active ladder (tmt+modules) has its reference point at the tilt rotation point. So we do not need any extra translation
    //
    // The tube and cooling have there reference point 
    // In y: center of cooling tube
    // In x: ~center of envelope (will change it to base of glue layer.)
    ladderServicesTransform = GeoTrf::Translate3D(xCenter, yCenter, 0);
      
    //
    // The Pigtail
    //
    GeoPixelPigtail pigtail (m_DDmgr, m_gmt_mgr);
    pigtailPhysVol = pigtail.Build();
    transPigtail = GeoTrf::Translate3D(m_gmt_mgr->PixelPigtailBendX() + m_gmt_mgr->PixelLadderCableOffsetX(), 
				  m_gmt_mgr->PixelPigtailBendY() + m_gmt_mgr->PixelLadderCableOffsetY(), 
				  0.);


  } 

  //
  // Layer dimensions from above, etc
  //
  double safety = 0.01 * Gaudi::Units::mm;
  double rmin =  m_gmt_mgr->PixelLayerRadius()-layerThicknessN - safety;
  double rmax =  m_gmt_mgr->PixelLayerRadius()+layerThicknessP + safety;
  double length = m_gmt_mgr->PixelLadderLength() + 4*m_epsilon; // Ladder has length m_gmt_mgr->PixelLadderLength() +  2*m_epsilon
  //std::cout << "rmin = " << rmin << ", rmax = " << rmax << std::endl;

  //  In case of IBL stave detailed descriptiob
  bool bAddIBLStaveRings=false;
  if(m_gmt_mgr->GetLD()==0&&m_gmt_mgr->ibl()&&m_gmt_mgr->PixelStaveLayout()>3&&m_gmt_mgr->PixelStaveLayout()<8)
    {
      bAddIBLStaveRings=true;
      double safety = 0.001 * Gaudi::Units::mm;
      double outerRadius = m_gmt_mgr->IBLSupportMidRingInnerRadius();  
      rmax=outerRadius-safety;

      if(m_gmt_mgr->PixelStaveAxe()==1) {
	double outerRadius = m_gmt_mgr->IBLSupportMidRingOuterRadius();  
	rmax=outerRadius+safety;
      }
      m_gmt_mgr->msg(MSG::INFO)<<"Layer IBL / stave ring :  outer radius max  "<<rmax<<endmsg;

    }

// 
//    Build ladder
//   GeoVPhysVol *ladderPhys=pixelLadder.Build();
//   
//   Now make the layer envelope
//    
//   const GeoMaterial* air = m_mat_mgr->getMaterial("std::Air");
//   std::ostringstream lname;
//   lname << "Layer" << m_gmt_mgr->GetLD();
//   const GeoTube* layerTube = new GeoTube(rmin,rmax,0.5*length); //solid
//   const GeoLogVol* layerLog = new GeoLogVol(lname.str(),layerTube,air); //log volume
//   GeoFullPhysVol* layerPhys = new GeoFullPhysVol(layerLog); // phys vol


  GeoFullPhysVol* layerPhys = nullptr;
 
  //
  // A few variables needed below
  //  
  double angle=(nSectors>0)?(360./(double)nSectors)*Gaudi::Units::deg:(360.*Gaudi::Units::deg);
  GeoTrf::Transform3D transRadiusAndTilt = GeoTrf::TranslateX3D(layerRadius)*GeoTrf::RotateZ3D(ladderTilt);
  double phiOfModuleZero =  m_gmt_mgr->PhiOfModuleZero();

  // IBL rotations are defined vs the cooling pipe center...
  if(m_gmt_mgr->GetLD()==0&&m_gmt_mgr->ibl()&&m_gmt_mgr->PixelStaveAxe()==1)   
    {

      //  Point that defines the center of the cooling pipe
      GeoTrf::Vector3D centerCoolingPipe = m_gmt_mgr->IBLStaveRotationAxis() ;

      // Transforms
      GeoTrf::Transform3D staveTrf = GeoTrf::RotateZ3D(ladderTilt)*GeoTrf::Translate3D(-centerCoolingPipe.x(),-centerCoolingPipe.y(),-centerCoolingPipe.z());
      double staveRadius = m_gmt_mgr->IBLStaveRadius() ;

      transRadiusAndTilt = GeoTrf::TranslateX3D(staveRadius)*staveTrf;
    }

  //
  // Loop over the sectors and place everything
  //
  for(int ii = 0; ii < nSectors; ii++) {
    m_gmt_mgr->SetPhi(ii);

    // Build ladder
    GeoVPhysVol *ladderPhys=pixelLadder.Build();

    if(ii==0){
      if(bAddIBLStaveRings)
	{
	  const GeoSimplePolygonBrep* bRep = dynamic_cast<const GeoSimplePolygonBrep*> (ladderPhys->getLogVol()->getShape());
	  length = 2*bRep->getDZ();
	}
      
      //
      // Now make the layer envelope
      // 
      m_gmt_mgr->msg(MSG::INFO)<<"Layer "<<m_gmt_mgr->GetLD()<<" in/out radius "<<rmin<<"  "<<rmax<<endmsg;
      const GeoMaterial* air = m_mat_mgr->getMaterial("std::Air");
      std::ostringstream lname;
      lname << "Layer" << m_gmt_mgr->GetLD();
      const GeoTube* layerTube = new GeoTube(rmin,rmax,0.5*length); //solid
      const GeoLogVol* layerLog = new GeoLogVol(lname.str(),layerTube,air); //log volume
      layerPhys = new GeoFullPhysVol(layerLog); // phys vol
    }

    double phiOfSector = phiOfModuleZero + ii*angle;

    // transform equiv  RotZ(phiOfSector)*TransZ(layerRadius)*RotZ(tilt)
    GeoTrf::Transform3D ladderTransform = GeoTrf::RotateZ3D(phiOfSector) * transRadiusAndTilt;

    //
    // Place the active ladders
    //
    std::ostringstream nameTag; 
    nameTag << "Ladder" << ii;
    GeoNameTag * tag = new GeoNameTag(nameTag.str());
    GeoTransform* xform = new GeoTransform(ladderTransform);
    layerPhys->add(tag);
    layerPhys->add(new GeoIdentifierTag(ii) );
    layerPhys->add(xform);
    layerPhys->add(ladderPhys);   //pixelLadder.Build());

    if(m_gmt_mgr->DoServicesOnLadder() && staveLayout == 0) {

      //
      // Place the LadderServices volume (omega, tubes, connectors, cables for the layer)
      //
      // NB. PixelFluidOrient and PixelCableBiStave depends on phi
      int ladderType = m_gmt_mgr->PixelFluidOrient(m_gmt_mgr->GetLD(), ii);
      int biStave    = m_gmt_mgr->PixelBiStaveType(m_gmt_mgr->GetLD(), ii) % 2;  // Will be 1 or 2 -> Map to 1,0
      //int biStave    = m_gmt_mgr->PixelCableBiStave() % 2;
      //int biStave    = ii % 2;
      //std::cout << "Layer,phiModule,fluidtype,bistavetype,index: " 
      //	  << m_gmt_mgr->GetLD()  << ", "
      //	  << ii << ", "
      //	  << ladderType << ", "
      //	  << m_gmt_mgr->PixelBiStaveType(m_gmt_mgr->GetLD(), ii)  << ", "
      //          << biStave*(maxLadType+1) + ladderType << std::endl;


      // transform equiv to RotZ(angle)*TransZ(layerRadius)*RotZ(tilt)*transTubeAndCables      
      xform = new GeoTransform(ladderTransform * ladderServicesTransform);
      layerPhys->add(xform);
      layerPhys->add(ladderServicesArray[biStave*(maxLadType+1) + ladderType]);

      // Place the pigtail (the curved section)
      // transform equiv to RotZ(angle)*TransZ(layerRadius)*RotZ(tilt)*transPigtail      
      xform = new GeoTransform(ladderTransform*transPigtail);
      layerPhys->add(xform);
      layerPhys->add(pigtailPhysVol);

    }
  }
  //  delete staveSupport;
  
  if(layerPhys==nullptr)
  {
      m_gmt_mgr->msg(MSG::ERROR)<<"layerPhys = 0 in GeoPixelLayer in "<<__FILE__<<endmsg;
      std::abort();
  } 

  //
  // Extra Material. I don't think there is much room but we provide the hooks anyway   
  //
  InDetDD::ExtraMaterial xMat(m_gmt_mgr->distortedMatManager());
  xMat.add(layerPhys,"PixelLayer");
  std::ostringstream ostr; ostr << m_gmt_mgr->GetLD();
  xMat.add(layerPhys,"PixelLayer"+ostr.str());
  

  //
  // IBL stave rings area : stave ring + endblock + flex (with dogleg)
  //
  if(bAddIBLStaveRings)
    {
      m_gmt_mgr->msg(MSG::INFO) << "IBL stave ring support"<< endmsg;

      GeoPixelStaveRingServices staveRingService(m_DDmgr, m_gmt_mgr, pixelLadder, *staveSupport);
      staveRingService.Build();

      m_supportPhysA = staveRingService.getSupportA();
      m_supportPhysC = staveRingService.getSupportC();
      m_supportMidRing = staveRingService.getSupportMidRing();
	
      m_xformSupportA = staveRingService.getSupportTrfA();
      m_xformSupportC = staveRingService.getSupportTrfC();
      m_xformSupportMidRing = staveRingService.getSupportTrfMidRing();
      
      if(m_gmt_mgr->PixelStaveAxe()==1) {
	GeoNameTag *tagM = new GeoNameTag("Brl0M_StaveRing");         
	GeoTransform *xformSupportMidRing = new GeoTransform(GeoTrf::Transform3D::Identity());
	GeoVPhysVol *supportPhysMidRing = getSupportMidRing();
	layerPhys->add(tagM);
	layerPhys->add(xformSupportMidRing);
	layerPhys->add(supportPhysMidRing);
      }

    }


  return layerPhys;
}

