/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetDetDescrExample/ReadSiDetectorElements.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorManager.h"
#include "InDetReadoutGeometry/SiNumerology.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "ReadoutGeometryBase/SiIntersect.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "Identifier/Identifier.h"

#include "ReadoutGeometryBase/SiLocalPosition.h"


#include <vector>
#include <string>

using namespace InDetDD;


/////////////////////////////////////////////////////////////////////////////
//
// Prints out SiDetectorElement positions and other info.

/////////////////////////////////////////////////////////////////////////////

ReadSiDetectorElements::ReadSiDetectorElements(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator),
  m_managerName("Pixel"),  // or SCT
  m_doLoop(true),
  m_manager(nullptr),
  m_idHelper(nullptr),
  m_pixelIdHelper(nullptr),
  m_sctIdHelper(nullptr),
  m_first(true)
{  
  // Get parameter values from jobOptions file
  declareProperty("ManagerName", m_managerName);
  declareProperty("LoopOverElements", m_doLoop);
  declareProperty("DoInitialize", m_doInit = false);
  declareProperty("DoExecute",    m_doExec = true);
  declareProperty("UseConditionsTools", m_useConditionsTools = false);
  declareProperty("PrintProbePositions", m_printProbePositions = true);
  declareProperty("PrintTransforms", m_printTransforms = true);
  declareProperty("PrintDirections", m_printDirections = true);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

StatusCode ReadSiDetectorElements::initialize(){
  // Retrieve GeoModel Detector Elements
  // You can either get the SCT or pixel manager or the common base class
  // manager. In this example I get the base class.
  //  const SiDetectorManager * manager;
  // or
  //  const PixelDetectorManager * manager;
  //  const SCT_DetectorManager  * manager;

  ATH_CHECK( detStore()->retrieve(m_manager,m_managerName));
  if (m_managerName == "Pixel" || m_managerName == "ITkPixel") {
    //
    // Get Pixel ID helper
    //
    // Pixel ID helper: const PixelID * m_pixelIdHelper;
    ATH_CHECK(detStore()->retrieve(m_pixelIdHelper, "PixelID"));

    // If common pixel/SCT code can copy to pointer to AtlasDetectorID
    m_idHelper = m_pixelIdHelper;

  } else {   
    //
    // Get SCT ID helper
    //
    // SCT ID helper: const SCT_ID * m_sctIdHelper;
    ATH_CHECK(detStore()->retrieve(m_sctIdHelper, "SCT_ID"));

    // If common pixel/SCT code can copy to pointer to AtlasDetectorID
    m_idHelper = m_sctIdHelper;
  }

  if (m_useConditionsTools) {
    ATH_CHECK(m_siLorentzAngleTool.retrieve());
    ATH_CHECK(m_siConditionsTool.retrieve());
  } else {
    m_siLorentzAngleTool.disable();
    m_siConditionsTool.disable();
  }

  // Initialize ReadCondHandleKey
  ATH_CHECK(m_detEleCollKey.initialize());

  // Print during initialize
  if (m_doInit) {
    printAllElements(true);
    printRandomAccess(true);
  }
  return StatusCode::SUCCESS;
}


StatusCode ReadSiDetectorElements::execute() {
  // Only print out on first event
  if (m_first && m_doExec) {
    m_first = false;
    printAllElements(false);
    printRandomAccess(false);
    printDifference();
  }
  return StatusCode::SUCCESS;
}

void ReadSiDetectorElements::printAllElements(const bool accessDuringInitialization) {
  const bool useConditionStore = not accessDuringInitialization;
  const SiDetectorElementCollection* elements = nullptr;
  if (useConditionStore) {
    // Get SiDetectorElementCollection from ConditionStore
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> detEle(m_detEleCollKey);
    elements = detEle.retrieve();
    ATH_MSG_INFO("Going to read from Conditions Store using handle: " << m_detEleCollKey.key());
    if (elements==nullptr) {
      ATH_MSG_FATAL(m_detEleCollKey.fullKey() << " could not be retrieved");
      return;
    }
  } else {
    ATH_MSG_INFO("Going to read from detector manager: " << m_managerName);
    elements = m_manager->getDetectorElementCollection();
  }

  // There are various ways you can access the elements. eg
  // m_manager->getDetectorElement(idHash);
  // m_manager->getDetectorElement(identifier);
  //
  // or access the whole collection or the iterators. 
  if (m_doLoop) {
    for (const SiDetectorElement* element: *elements) {
      if (element) {
        ATH_MSG_ALWAYS(m_idHelper->show_to_string(element->identify()));
        // The id helper is also available through  the elements
        //
        // element->getIdHelper()->show(element->identify());
        //
        ATH_MSG_ALWAYS(" center (x,y,z)   = " << element->center().x() << "," << element->center().y() << "," << element->center().z());
	      ATH_MSG_ALWAYS(" center (r,phi,z) = " << element->center().perp() << "," << element->center().phi() << "," <<element->center().z());
	if(m_printProbePositions){
	  ATH_MSG_ALWAYS(" global (r,phi,z) position of (1,1)          = " <<element->globalPosition(Amg::Vector2D(1,1)).perp() << "," << element->globalPosition(Amg::Vector2D(1,1)).phi() <<","<< element->globalPosition(Amg::Vector2D(1,1)).z());
	  ATH_MSG_ALWAYS(" global (r,phi,z) position of (-1,-1)        = " <<element->globalPosition(Amg::Vector2D(-1,-1)).perp() << "," << element->globalPosition(Amg::Vector2D(-1,-1)).phi() <<","<< element->globalPosition(Amg::Vector2D(-1,-1)).z());
	  ATH_MSG_ALWAYS(" global (r,phi,z) hit position of (1,1,0)    = " <<element->globalPositionHit(Amg::Vector3D(1,1,0)).perp() << "," << element->globalPositionHit(Amg::Vector3D(1,1,0)).phi() <<","<< element->globalPositionHit(Amg::Vector3D(1,1,0)).z());
	  ATH_MSG_ALWAYS(" global (r,phi,z) hit  position of (-1,-1,0) = " <<element->globalPositionHit(Amg::Vector3D(-1,-1,0)).perp() << "," << element->globalPositionHit(Amg::Vector3D(-1,-1,0)).phi() <<","<< element->globalPositionHit(Amg::Vector3D(-1,-1,0)).z()); 
    ATH_MSG_ALWAYS(" Cell Id of (1,1)        = " <<element->cellIdOfPosition(Amg::Vector2D(1,1)).etaIndex() << "," << element->cellIdOfPosition(Amg::Vector2D(1,1)).phiIndex());
    ATH_MSG_ALWAYS(" Cell Id of (-1,-1)      = " <<element->cellIdOfPosition(Amg::Vector2D(-1,-1)).etaIndex() << "," << element->cellIdOfPosition(Amg::Vector2D(-1,-1)).phiIndex());

  }
        ATH_MSG_ALWAYS(" Normal = " <<element->normal().perp() << "," << element->normal().phi() <<","<< element->normal().z());
        ATH_MSG_ALWAYS(" sin(tilt), sin(stereo) = " <<  element->sinTilt() << " " 
                       << element->sinStereo());
        ATH_MSG_ALWAYS(" width, minWidth, maxWidth, length (mm) = " 
                       << element->width()/CLHEP::mm << " " 
                       << element->minWidth()/CLHEP::mm << " " 
                       << element->maxWidth()/CLHEP::mm << " " 
                       << element->length()/CLHEP::mm);
  
        // These are no longer accessed through the detector element.
        IdentifierHash hashId = element->identifyHash();
        if (m_useConditionsTools) {
          const EventContext &ctx = Gaudi::Hive::currentContext();
          ATH_MSG_ALWAYS(" Temperature (C), bias voltage, depletion voltage: "
                         << m_siConditionsTool->temperature(hashId, ctx) << " "
                         << m_siConditionsTool->biasVoltage(hashId, ctx) << " "
                         << m_siConditionsTool->depletionVoltage(hashId, ctx));
        

	  ATH_MSG_ALWAYS(" Lorentz correction (mm), tanLorentzPhi = "
			 << m_siLorentzAngleTool->getLorentzShift(hashId)/CLHEP::mm << " "
			 << m_siLorentzAngleTool->getTanLorentzAngle(hashId));

	}

        ATH_MSG_ALWAYS(" HashId, Id : " << hashId << "\t" << element->identify().getString());
	

        // Make some consistency tests for the identifier.
        Identifier idTest;
        IdentifierHash idHashTest;
        if (m_managerName == "Pixel" || m_managerName == "ITkPixel") {
          idTest = m_pixelIdHelper->wafer_id(hashId);
          idHashTest = m_pixelIdHelper->wafer_hash(idTest);
        } else if (m_sctIdHelper) {
          idTest = m_sctIdHelper->wafer_id(hashId);
          idHashTest = m_sctIdHelper->wafer_hash(idTest);
        }
        const SiDetectorElement * elementtest1 = nullptr;
        const SiDetectorElement * elementtest2 = nullptr;
        if (useConditionStore) {
          // SiDetectorElementCollection::getDetectorElement supports only IdentifierHash as the argument.
          if (m_managerName == "Pixel" || m_managerName == "ITkPixel") {
            elementtest1 = elements->getDetectorElement(m_pixelIdHelper->wafer_hash(element->identify()));
          } else {
            elementtest1 = elements->getDetectorElement(m_sctIdHelper->wafer_hash(element->identify()));
          }
          elementtest2 = elements->getDetectorElement(hashId);
        } else {
          elementtest1 = m_manager->getDetectorElement(element->identify());
          elementtest2 = m_manager->getDetectorElement(hashId);
        }
        bool idOK = true;
        if (idHashTest != hashId) {ATH_MSG_ALWAYS(" Id test 1 FAILED!"); idOK = false;}
        if (idTest != element->identify()) {ATH_MSG_ALWAYS(" Id test 2 FAILED!"); idOK = false;}
        if (elementtest1 != element) {ATH_MSG_ALWAYS(" Id test 3 FAILED!"); idOK = false;}
        if (elementtest2 != element) {ATH_MSG_ALWAYS(" Id test 4 FAILED!"); idOK = false;}
        if (idOK) ATH_MSG_ALWAYS(" ID tests OK") ;
      } else {
        // ATH_MSG_ALWAYS("Missing element!!!!!!!!!!!");
      }
   if(m_printTransforms){
      const GeoTrf::Transform3D mytrf = element->transform();
      const GeoTrf::Transform3D mytrfhit = element->transformHit();

      	ATH_MSG_ALWAYS("Transform: ");
        ATH_MSG_ALWAYS("|"<<mytrf(2,0)<<","<<mytrf(2,1)<<","<<mytrf(2,2)<<"|");
        ATH_MSG_ALWAYS("|"<<mytrf(1,0)<<","<<mytrf(1,1)<<","<<mytrf(1,2)<<"|");
        ATH_MSG_ALWAYS("|"<<mytrf(0,0)<<","<<mytrf(0,1)<<","<<mytrf(0,2)<<"|");
        ATH_MSG_ALWAYS("");
        ATH_MSG_ALWAYS("TransformHit: ");
        ATH_MSG_ALWAYS("|"<<mytrfhit(2,0)<<","<<mytrfhit(2,1)<<","<<mytrfhit(2,2)<<"|");
        ATH_MSG_ALWAYS("|"<<mytrfhit(1,0)<<","<<mytrfhit(1,1)<<","<<mytrfhit(1,2)<<"|");
        ATH_MSG_ALWAYS("|"<<mytrfhit(0,0)<<","<<mytrfhit(0,1)<<","<<mytrfhit(0,2)<<"|");
        ATH_MSG_ALWAYS("");
   }
   if(m_printDirections){
    ATH_MSG_ALWAYS("Depth Angle: "<<element->depthAngle());
    if(element->depthDirection()) ATH_MSG_ALWAYS("Depth Direction True");
    else ATH_MSG_ALWAYS("Depth Direction False");
    ATH_MSG_ALWAYS("Eta Angle: "<<element->etaAngle());
    if(element->etaDirection()) ATH_MSG_ALWAYS("Eta Direction True");
    else ATH_MSG_ALWAYS("Eta Direction False");
    ATH_MSG_ALWAYS("Phi Angle: "<<element->phiAngle());
    if(element->phiDirection()) ATH_MSG_ALWAYS("Phi Direction True");
    else ATH_MSG_ALWAYS("Phi Direction False");

    if(std::abs(element->depthAngle())<0.5) ATH_MSG_ALWAYS("BAD DEPTH DIRECTION!");
    if(std::abs(element->etaAngle())<0.5) ATH_MSG_ALWAYS("BAD ETA DIRECTION!");
    if(std::abs(element->phiAngle())<0.5) ATH_MSG_ALWAYS("BAD PHI DIRECTION!");
   }
  //add divider between elements for readability
	ATH_MSG_ALWAYS("-----------------------------");
    }
  }
  // Testing numerology
  const SiNumerology  siNumerology(m_manager->numerology());
  int nSides = 1;
  if (m_sctIdHelper) nSides = 2;
  int barrelCount = 0;
  int barrelCountError = 0;
  // Barrel
  for (int iBarrelIndex = 0; iBarrelIndex < siNumerology.numBarrels(); iBarrelIndex++) {
    int iBarrel = siNumerology.barrelId(iBarrelIndex);
    ATH_MSG_ALWAYS("Barrel: " << iBarrel);
    ATH_MSG_ALWAYS(" Num layers: " << siNumerology.numLayers());
    for (int iLayer = 0; iLayer < siNumerology.numLayers(); iLayer++) {
      ATH_MSG_ALWAYS(" Layer: " << iLayer);
      if (!siNumerology.useLayer(iLayer))ATH_MSG_ALWAYS("  Layer not present");
      ATH_MSG_ALWAYS("  Num Modules in Phi: " << siNumerology.numPhiModulesForLayer(iLayer));
      ATH_MSG_ALWAYS("  Num Modules in Eta: " << siNumerology.numEtaModulesForLayer(iLayer));
      for (int iPhi = 0; iPhi < siNumerology.numPhiModulesForLayer(iLayer); iPhi++) {
        for (int iEta = siNumerology.beginEtaModuleForLayer(iLayer); iEta < siNumerology.endEtaModuleForLayer(iLayer); iEta++) {
          if (!iEta && siNumerology.skipEtaZeroForLayer(iLayer)) continue;
          for (int iSide = 0; iSide < nSides; iSide++) {
            Identifier id;
            if (m_managerName == "Pixel" || m_managerName == "ITkPixel"){
              id = m_pixelIdHelper->wafer_id(iBarrel,iLayer,iPhi,iEta);
            } else {
              id = m_sctIdHelper->wafer_id(iBarrel,iLayer,iPhi,iEta,iSide);
            }
            const SiDetectorElement * element = nullptr;
            if (useConditionStore) {
              // SiDetectorElementCollection::getDetectorElement supports only IdentifierHash as the argument.
              if (m_managerName == "Pixel" || m_managerName == "ITkPixel") {
                element = elements->getDetectorElement(m_pixelIdHelper->wafer_hash(id));
              } else {
                element = elements->getDetectorElement(m_sctIdHelper->wafer_hash(id));
              }
            } else {
              element = m_manager->getDetectorElement(id);
            }
            barrelCount++;
            if (!element) {
              barrelCountError++;
              ATH_MSG_ALWAYS("   No element found for id: " << m_idHelper->show_to_string(id));
            } else {
              // For extra safety in case some strip modules do not have two sides (eg in future geometries) one could add.
              if (!element->otherSide()) iSide++;
              ATH_MSG_ALWAYS("   " << m_idHelper->show_to_string(id));
            }     
          } // iSide
        } // iEta
      } //iPhi
    } //iLayer
  } // Barrel

  int endcapCount = 0;
  int endcapCountError = 0;
  // Endcap
  for (int iEndcapIndex = 0; iEndcapIndex < siNumerology.numEndcaps(); iEndcapIndex++) {
    int iEndcap = siNumerology.endcapId(iEndcapIndex);
    ATH_MSG_ALWAYS("Endcap: " << iEndcap);
    ATH_MSG_ALWAYS(" Num disks: " << siNumerology.numDisks());
    for (int iDisk = 0; iDisk < siNumerology.numDisks(); iDisk++) {
      ATH_MSG_ALWAYS(" Disk: " << iDisk);
      if (!siNumerology.useDisk(iDisk))ATH_MSG_ALWAYS("  Disk not present");
      ATH_MSG_ALWAYS("  Num Rings: " << siNumerology.numRingsForDisk(iDisk));
      for (int iEta = 0; iEta < siNumerology.numRingsForDisk(iDisk); iEta++) {
        ATH_MSG_ALWAYS("  Ring: " << iEta); 
        ATH_MSG_ALWAYS("   Num Modules in Phi: " << siNumerology.numPhiModulesForDiskRing(iDisk,iEta));    
        for (int iPhi = 0; iPhi < siNumerology.numPhiModulesForDiskRing(iDisk,iEta); iPhi++) {
          for (int iSide = 0; iSide < nSides; iSide++) {
            Identifier id;
            if (m_managerName == "Pixel" || m_managerName == "ITkPixel") {
              id = m_pixelIdHelper->wafer_id(iEndcap,iDisk,iPhi,iEta);
            } else {
              id = m_sctIdHelper->wafer_id(iEndcap,iDisk,iPhi,iEta,iSide);
            }
            const SiDetectorElement * element = m_manager->getDetectorElement(id);
            endcapCount++;
            if (!element) {
              endcapCountError++;
              ATH_MSG_ALWAYS("    No element found for id: " << m_idHelper->show_to_string(id));
            } else {
              // For extra safety in case some strip modules do not have two sides (eg in future geometries) one could add.
              if (!element->otherSide()) iSide++;
              ATH_MSG_ALWAYS("    " << m_idHelper->show_to_string(id));
            }
          } // iSide
        } // iEta
      } //iPhi
    } //iDisk
  } // Endcap;

  ATH_MSG_ALWAYS("Number of barrel elements : " << barrelCount);
  ATH_MSG_ALWAYS("Number not found          : " << barrelCountError);
  ATH_MSG_ALWAYS("Number of endcap elements : " << endcapCount);
  ATH_MSG_ALWAYS("Number not found          : " << endcapCountError);
  
  // Maximums 
  ATH_MSG_ALWAYS("MaxNumBarrelEta:   " <<  siNumerology.maxNumBarrelEta());
  ATH_MSG_ALWAYS("MaxNumEndcapRings: " <<  siNumerology.maxNumEndcapRings());
  ATH_MSG_ALWAYS("MaxNumStrips:      " <<  siNumerology.maxNumStrips());
  ATH_MSG_ALWAYS("MaxNumPhiCells:    " <<  siNumerology.maxNumPhiCells());
  ATH_MSG_ALWAYS("MaxNumEtaCells:    " <<  siNumerology.maxNumEtaCells());

  ATH_MSG_ALWAYS("Num Designs: " <<  m_manager->numDesigns());
}


void ReadSiDetectorElements::printRandomAccess(const bool accessDuringInitialization) {
  ATH_MSG_INFO("printRandomAccess()");

  const bool useConditionStore = (m_managerName == "SCT" and (not accessDuringInitialization));
  const SiDetectorElementCollection* elements = nullptr;
  if (useConditionStore) {
    // Get SiDetectorElementCollection from ConditionStore
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> detEle(m_detEleCollKey);
    elements = detEle.retrieve();
    if (elements==nullptr) {
      ATH_MSG_FATAL(m_detEleCollKey.fullKey() << " could not be retrieved");
      return;
    }
  }

  // Some random access
  if (m_managerName == "Pixel" || m_managerName == "ITkPixel") {
    //const PixelID * idHelper = dynamic_cast<const PixelID *>(m_manager->getIdHelper());
    const PixelID * idHelper = m_pixelIdHelper;
    if (idHelper) {
      Identifier id;
      std::vector<SiCellId> cellIds;
      std::vector<Amg::Vector2D> positions;
      // wafer_id(barrel_ec, layer_disk, phi_module, eta_module)
      // A barrel element
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A Pixel Barrel element (non B-layer) "        );
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(0,1,15,-3);
      cellIds.emplace_back(32,8); // phi,eta
      //add a range of cells from 151 to 175
      for (int i(151);i != 176; ++i){
        cellIds.emplace_back(i,8); // phi,eta
      }
      cellIds.emplace_back(-1,1); // phi,eta
      cellIds.emplace_back(0,1); // phi,eta
      cellIds.emplace_back(1,-1); // phi,eta
      cellIds.emplace_back(1,0); // phi,eta
      cellIds.emplace_back(327,1); // phi,eta
      cellIds.emplace_back(328,1); // phi,eta
      cellIds.emplace_back(1,143); // phi,eta
      cellIds.emplace_back(1,144); // phi,eta
      positions.emplace_back(12.727*CLHEP::mm, 4.534*CLHEP::mm); // eta,phi
      testElement(id, cellIds, positions, elements);

      // A barrel element (B-Layer)
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A Pixel Barrel element (B-layer)     "        );
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(0,0,7,-3);
      cellIds.clear();
      positions.clear();
      cellIds.emplace_back(32,8); // phi,eta
      positions.emplace_back(12.727*CLHEP::mm, 4.534*CLHEP::mm); // eta,phi
      testElement(id, cellIds, positions, elements);

      // An endcap element
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A Pixel Endcap element"                       );
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(2,2,13,0);
      cellIds.emplace_back(182,75); // phi,eta
      positions.emplace_back(0*CLHEP::mm, 0*CLHEP::mm); // eta,phi
      positions.emplace_back(30.4*CLHEP::mm, 8.2*CLHEP::mm); // eta,phi - on edge
      positions.emplace_back(12*CLHEP::mm, -8.15*CLHEP::mm); // eta,phi - near edge
      positions.emplace_back(12*CLHEP::mm, -8.25*CLHEP::mm); // eta,phi - near edge
      positions.emplace_back(12*CLHEP::mm, -8.35*CLHEP::mm); // eta,phi - outside
      testElement(id, cellIds, positions, elements);

    }
  } else if (m_managerName == "SCT" || m_managerName == "ITkStrip") {
    
    //const SCT_ID * idHelper = dynamic_cast<const SCT_ID *>(m_manager->getIdHelper());
    const SCT_ID * idHelper = m_sctIdHelper;
    if (idHelper) {

      Identifier id;
      std::vector<SiCellId> cellIds;
      std::vector<Amg::Vector2D> positions;


      // wafer_id(barrel_ec, layer_disk, phi_module, eta_module, side)
      // A barrel element
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A SCT Barrel element"                         );
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(0,1,15,-3,0);
      cellIds.clear();
      positions.clear();
      cellIds.emplace_back(32); // phi,eta
      cellIds.emplace_back(1); // phi,eta
      cellIds.emplace_back(0); // phi,eta
      if (m_managerName == "SCT") {
        cellIds.emplace_back(-1); // phi,eta
        cellIds.emplace_back(-2); // phi,eta
        cellIds.emplace_back(-3); // phi,eta
      }
      cellIds.emplace_back(767); // phi,eta
      cellIds.emplace_back(768); // phi,eta
      positions.emplace_back(12.727*CLHEP::mm, 4.534*CLHEP::mm); // eta,phi
      testElement(id, cellIds, positions, elements);

      // A barrel element (other side of above)
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A SCT Barrel element (other side of above)   ");
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(0,1,15,-3,1);
      cellIds.clear();
      positions.clear();
      cellIds.emplace_back(32); // phi,eta
      positions.emplace_back(12.727*CLHEP::mm, 4.534*CLHEP::mm); // eta,phi
      testElement(id, cellIds, positions, elements);

      // A outer fwd
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A SCT Endcap element (outer type)"            );
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(2,3,15,0,0);
      cellIds.clear();
      positions.clear();
      cellIds.emplace_back(532); // phi,eta
      cellIds.emplace_back(0); // phi,eta
      if (m_managerName == "SCT") cellIds.emplace_back(-1); // phi,eta
      cellIds.emplace_back(767); // phi,eta
      cellIds.emplace_back(768); // phi,eta
      positions.emplace_back(12.727*CLHEP::mm, 20.534*CLHEP::mm); // eta,phi
      positions.emplace_back(12.727*CLHEP::mm, -20.534*CLHEP::mm); // eta,phi
      positions.emplace_back(3*CLHEP::mm, -25*CLHEP::mm); // eta,phi
      testElement(id, cellIds, positions, elements);

      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A SCT Endcap element (outer type) other side");
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(2,3,15,0,1);
      cellIds.clear();
      positions.clear();
      cellIds.emplace_back(532); // phi,eta
      positions.emplace_back(12.727*CLHEP::mm, 20.534*CLHEP::mm); // eta,phi
      positions.emplace_back(12.727*CLHEP::mm, -20.534*CLHEP::mm); // eta,phi
      positions.emplace_back(3*CLHEP::mm, -25*CLHEP::mm); // eta,phi
      testElement(id, cellIds, positions, elements);

      // A middle fwd
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A SCT Endcap element (middle type)"           );
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(2,1,15,1,0);
      cellIds.clear();
      positions.clear();
      cellIds.emplace_back(532); // phi,eta
      positions.emplace_back(12.727*CLHEP::mm, 4.534*CLHEP::mm); // eta,phi
      testElement(id, cellIds, positions, elements);

      // A truncated middle
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A SCT Endcap element (truncated middle type)" );
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(2,7,15,1,0);
      cellIds.clear();
      positions.clear();
      cellIds.emplace_back(532); // phi,eta
      positions.emplace_back(12.727*CLHEP::mm, 4.534*CLHEP::mm); // eta,phi
      testElement(id, cellIds, positions, elements);

      // A inner fwd
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS(" A SCT Endcap element (inner type)"            );
      ATH_MSG_ALWAYS("----------------------------------------------");
      id = idHelper->wafer_id(2,1,15,2,0);
      cellIds.clear();
      positions.clear();
      cellIds.emplace_back(532); // phi,eta
      positions.emplace_back(12.727*CLHEP::mm, 4.534*CLHEP::mm); // eta,phi
      testElement(id, cellIds, positions, elements);
    }
  } // if manager = Pixel,SCT
} 

void
ReadSiDetectorElements::printDifference() const {
  // Get SiDetectorElementCollection from ConditionStore
  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> detEle(m_detEleCollKey);
  const SiDetectorElementCollection* elementsC = detEle.retrieve();
  ATH_MSG_INFO("Going to read from Conditions Store using handle: " << m_detEleCollKey.key());
  if (elementsC==nullptr) {
    ATH_MSG_FATAL(m_detEleCollKey.fullKey() << " could not be retrieved");
    return;
  }

  // Get SiDetectorElementCollection from detector manager
  const SiDetectorElementCollection* elementsM = m_manager->getDetectorElementCollection();

  if (elementsC->size()!=elementsM->size()) {
    ATH_MSG_FATAL("Sizes of SiDetectorElementCollections are different");
  }

  SiDetectorElementCollection::const_iterator elementC = elementsC->begin();
  SiDetectorElementCollection::const_iterator elementM = elementsM->begin();
  SiDetectorElementCollection::const_iterator elementMe = elementsM->end();
  for (; elementM!=elementMe; ++elementC, ++elementM) {
    auto diff = (*elementC)->center()-(*elementM)->center();
    if (diff[0]!=0. or diff[1]!=0. or diff[2]!=0.) {
      ATH_MSG_ALWAYS("----------------------------------------------");
      ATH_MSG_ALWAYS("hash: " << (*elementC)->identifyHash());
      ATH_MSG_ALWAYS("center (store) " << (*elementC)->center().transpose());
      ATH_MSG_ALWAYS("center (manager) " << (*elementM)->center().transpose());
      ATH_MSG_ALWAYS("diff (store-manager) " << diff.transpose());
      ATH_MSG_ALWAYS("----------------------------------------------");
    }
  }
}

void
ReadSiDetectorElements::testElement(const Identifier & id, 
                                    const std::vector<SiCellId> & cellIdVec, 
                                    const std::vector<Amg::Vector2D> & positionsVec,
                                    const InDetDD::SiDetectorElementCollection* elements) const{
  ATH_MSG_ALWAYS("----------------------------------------------");
  const SiDetectorElement * element = nullptr;
  if (elements) {
    if (m_managerName == "Pixel" || m_managerName == "ITkPixel") {
      element = elements->getDetectorElement(m_pixelIdHelper->wafer_hash(id));
    } else {
      element = elements->getDetectorElement(m_sctIdHelper->wafer_hash(id));
    }
  } else {
    element = m_manager->getDetectorElement(id);
  }
  if (element) {
    IdentifierHash hashId = element->identifyHash();
    ATH_MSG_ALWAYS(element->getIdHelper()->show_to_string(id));
    ATH_MSG_ALWAYS(" width, minWidth, maxWidth, length, thickness (mm) = " 
                   << element->width()/CLHEP::mm << " " 
                   << element->minWidth()/CLHEP::mm << " " 
                   << element->maxWidth()/CLHEP::mm << " " 
                   << element->length()/CLHEP::mm << " "
                   << element->thickness()/CLHEP::mm
                   );
    ATH_MSG_ALWAYS(" average etaPitch = " << element->etaPitch()/CLHEP::micrometer << " microns");
    ATH_MSG_ALWAYS(" average phiPitch = " << element->phiPitch()/CLHEP::micrometer << " microns");
    ATH_MSG_ALWAYS(" rMin, rMax, zMin, zMax (mm), phiMin, phiMax (deg) = " 
                   << element->rMin()/CLHEP::mm << " "
                   << element->rMax()/CLHEP::mm << " "
                   << element->zMin()/CLHEP::mm << " "
                   << element->zMax()/CLHEP::mm << " "
                   << element->phiMin()/CLHEP::degree << " "
                   << element->phiMax()/CLHEP::degree
                   );
    ATH_MSG_ALWAYS(" center, normal, etaAxis, phiAxis = "
                   << element->center() << " "
                   << element->normal() << " "
                   << element->etaAxis() << " "
                   << element->phiAxis() 
                   );
    ATH_MSG_ALWAYS(" center: r (mm) = " <<  element->center().perp()/CLHEP::mm 
                   << ", phi (deg) = " <<  element->center().phi()/CLHEP::deg);
    if (m_useConditionsTools) {
      const EventContext &ctx = Gaudi::Hive::currentContext();
      ATH_MSG_ALWAYS(" Temperature (C), bias voltage, depletion voltage: "
                     << m_siConditionsTool->temperature(hashId, ctx) << " "
                     << m_siConditionsTool->biasVoltage(hashId, ctx) << " "
                     << m_siConditionsTool->depletionVoltage(hashId, ctx));
    }
    ATH_MSG_ALWAYS(" sin(tilt), tilt (deg), sin(stereo), stereo (deg) = " 
                   << element->sinTilt() << ", " 
                   << asin(element->sinTilt())/CLHEP::degree << ", "
                   << element->sinStereo() << ", " 
                   << asin(element->sinStereo())/CLHEP::degree);
    ATH_MSG_ALWAYS(" Neighbours: ");
    ATH_MSG_ALWAYS("  nextInEta: " << printElementId(element->nextInEta()) );
    ATH_MSG_ALWAYS("  prevInEta: " << printElementId(element->prevInEta()) );
    ATH_MSG_ALWAYS("  nextInPhi: " << printElementId(element->nextInPhi()) );
    ATH_MSG_ALWAYS("  prevInPhi: " << printElementId(element->prevInPhi()) );
    ATH_MSG_ALWAYS("  otherSide: " << printElementId(element->otherSide()) );

    for (unsigned int iTestCell = 0; iTestCell < cellIdVec.size(); iTestCell++) {
      SiCellId cellId = cellIdVec[iTestCell];
      ATH_MSG_ALWAYS(" cell [phiIndex.etaIndex] = " << cellId);
      
      // Test cell Id -> Identifier
      Identifier fullCellId = element->identifierFromCellId(cellId);
      ATH_MSG_ALWAYS(" identifier = ");
      element->getIdHelper()->show(fullCellId);
      
      // Test Identifier -> cell Id 
      SiCellId cellId2 = element->cellIdFromIdentifier(fullCellId);
      ATH_MSG_ALWAYS(" extracted cell id [phiIndex.etaIndex] = " << cellId2);
      
      InDetDD::SiLocalPosition localPosRaw1 = element->rawLocalPositionOfCell(cellId);
      InDetDD::SiLocalPosition localPosRaw2 = element->rawLocalPositionOfCell(fullCellId);
      ATH_MSG_ALWAYS(" raw localPosition (using cell id) (xPhi,xEta) = " 
                     << localPosRaw1.xPhi() << ", " << localPosRaw1.xEta());
      ATH_MSG_ALWAYS(" raw localPosition (using full id) (xPhi,xEta) = " 
                     << localPosRaw2.xPhi() << ", " << localPosRaw2.xEta());
      SiCellId cellIdRaw(element->cellIdOfPosition(localPosRaw1));
      ATH_MSG_ALWAYS(" corresponding cell (phiIndex,etaIndex) = " 
                     << cellIdRaw); 
      ATH_MSG_ALWAYS(" Number of connected cells (2 means ganged): " 
                     << element->numberOfConnectedCells(cellId));
      msg(MSG::ALWAYS) << " Connected cells";
      for (int iCell=0; iCell < element->numberOfConnectedCells(cellId) ; iCell++) {
        SiCellId connectedCellId =  element->connectedCell(cellId, iCell);
        msg(MSG::ALWAYS) << ", " << iCell << ": " << connectedCellId;
      }
      ATH_MSG_ALWAYS("In range: " << element->design().cellIdInRange(cellId));
    }
    
    for (unsigned int iTestPos = 0; iTestPos < positionsVec.size(); iTestPos++) {
      const InDetDD::SiLocalPosition & localPosOrig = positionsVec[iTestPos];
      ATH_MSG_ALWAYS(" Requested local pos (xPhi,xEta) = " << localPosOrig.xPhi() << ", " << localPosOrig.xEta());
      //lost out to HepGeom here
      Amg::Vector3D globalPos(element->globalPosition(localPosOrig));
      ATH_MSG_ALWAYS(" Global pos = " << globalPos << ", r (mm) = " << globalPos.perp()/CLHEP::mm<< ", phi (deg) = " << globalPos.phi()/CLHEP::degree);

      //...because i need a HepGeom::Point3D<double> to pass to element->localPosition...
      InDetDD::SiLocalPosition localPosNew(element->localPosition(globalPos));
      ATH_MSG_ALWAYS(" Returned local Pos (xPhi,xEta) =  " << localPosNew.xPhi() << ", " << localPosNew.xEta());
      // Some arbitrary tolerance picked out of the air.
      double tolerance = 100*CLHEP::micrometer;
      SiIntersect intersectState = element->inDetector(globalPos, tolerance, tolerance);
      ATH_MSG_ALWAYS(" Intersects (tolerance = " << tolerance/CLHEP::mm << " mm) " 
                     << " (in,out,nearBoundary,mayIntersect) : " 
                     << intersectState.in() << ","
                     << intersectState.out() << ","
                     << intersectState.nearBoundary() << ","
                     << intersectState.mayIntersect());
      ATH_MSG_ALWAYS(" Near bond gap: (tolerance = " << tolerance/CLHEP::mm << " mm) : " 
                     <<  element->nearBondGap(globalPos, tolerance));
      SiCellId returnedCellId = element->cellIdOfPosition(localPosNew);

      ATH_MSG_ALWAYS(" Returned cell Id [phiIndex.etaIndex] = " 
                     << returnedCellId); 
      ATH_MSG_ALWAYS(" using global position sin(tilt), tilt (deg), sin(stereo), stereo (deg) = "
                     << element->sinTilt(globalPos) << ", "
                     << asin(element->sinTilt(globalPos))/CLHEP::degree << ", "
                     << element->sinStereo(globalPos) << ", "
                     << asin(element->sinStereo(globalPos))/CLHEP::degree);
    }   
  } else { // element == 0
  
    ATH_MSG_ALWAYS(" ELEMENT MISSING!!!!!!!!!! ");
  }
  ATH_MSG_ALWAYS("----------------------------------------------");
}

std::string 
ReadSiDetectorElements::printElementId(const SiDetectorElement * element) const{
  if (element) {
    return element->getIdHelper()->show_to_string(element->identify());
  } else {
    return "NONE";
  }
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

StatusCode ReadSiDetectorElements::finalize() {
  // Part 1: Get the messaging service, print where you are
  ATH_MSG_INFO("finalize()");
  return StatusCode::SUCCESS;
}
