/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//
// This class builds one Si Crystal to be placed in one module
//
// The information on which layer to build is retrieved by the Geometry Manager
//
// Contained by: PixelModule_log
//
// Contains: nothing
//
#include "GeoPixelSiCrystal.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ReadoutGeometryBase/PixelDiodeMatrix.h"
#include "ReadoutGeometryBase/SiCommonItems.h"
#include "ReadoutGeometryBase/InDetDD_Defs.h"

#include <algorithm> //for std::min, std::max
#include <vector>

using namespace InDetDD;

GeoPixelSiCrystal::GeoPixelSiCrystal(InDetDD::PixelDetectorManager* ddmgr,
                                     PixelGeometryManager* mgr,
                                     bool isBLayer, bool isModule3D)
  : GeoVPixelFactory (ddmgr, mgr)
{
  // 
  //Builds the design for this crystal
  m_isBLayer = isBLayer;
  m_isModule3D = isModule3D;
 
  // Dimensions
  const double thickness = m_gmt_mgr->PixelBoardThickness(m_isModule3D);
  const double length = m_gmt_mgr->PixelBoardLength(m_isModule3D);
  const double width = m_gmt_mgr->PixelBoardWidth(m_isModule3D);

  int circuitsPhi = m_gmt_mgr->DesignCircuitsPhi(m_isModule3D); // Warning col/row naming opposite to chip
  int circuitsEta = m_gmt_mgr->DesignCircuitsEta(m_isModule3D); // Warning col/row naming opposite to chip
  int cellRowPerCirc = m_gmt_mgr->DesignCellRowsPerCircuit(m_isModule3D);
  int cellColPerCirc = m_gmt_mgr->DesignCellColumnsPerCircuit(m_isModule3D);
  int diodeRowPerCirc = m_gmt_mgr->DesignDiodeRowsPerCircuit(m_isModule3D);
  int diodeColPerCirc = m_gmt_mgr->DesignDiodeColumnsPerCircuit(m_isModule3D);
  int readoutSide = m_gmt_mgr->DesignReadoutSide(m_isModule3D);

  double etaPitchLongEnd =  m_gmt_mgr->DesignPitchZLongEnd(m_isModule3D);
  double etaPitchLong =  m_gmt_mgr->DesignPitchZLong(m_isModule3D);
  double phiPitch = m_gmt_mgr->DesignPitchRP(m_isModule3D);
  double etaPitch = m_gmt_mgr->DesignPitchZ(m_isModule3D);


  std::shared_ptr<const PixelDiodeMatrix> fullMatrix = makeMatrix(phiPitch, etaPitch, etaPitchLong, etaPitchLongEnd,
					     circuitsPhi, circuitsEta, diodeRowPerCirc, diodeColPerCirc);
 
  std::unique_ptr<PixelModuleDesign> p_barrelDesign2 = std::make_unique<PixelModuleDesign>(thickness,
							     circuitsPhi,
							     circuitsEta,
							     cellColPerCirc,
							     cellRowPerCirc,
							     diodeColPerCirc,
							     diodeRowPerCirc,
							     fullMatrix,
							     InDetDD::electrons,
							     readoutSide);

  // Multiple connections (ganged pixels)
  if (m_gmt_mgr->NumberOfEmptyRows() > 0) {
    int minRow = m_gmt_mgr->EmptyRows(0);
    int maxRow = minRow;
    for (int iConnect = 0; iConnect < m_gmt_mgr->NumberOfEmptyRows(); iConnect++){
      minRow = std::min(minRow, m_gmt_mgr->EmptyRows(iConnect));
      minRow = std::min(minRow, m_gmt_mgr->EmptyRowConnections(iConnect));
      maxRow = std::max(maxRow, m_gmt_mgr->EmptyRows(iConnect));
      maxRow = std::max(maxRow, m_gmt_mgr->EmptyRowConnections(iConnect));
    }
  
    std::vector <int> connections(maxRow-minRow+1);
    
    // We fill them all with a one to one correspondence first.
    for (unsigned int iRow = 0; iRow < connections.size(); iRow++){
      connections[iRow] = iRow +  minRow;
    }
    
    // Now make the connections.
    for (int iConnect = 0; iConnect < m_gmt_mgr->NumberOfEmptyRows(); iConnect++){
      connections[m_gmt_mgr->EmptyRows(iConnect)-minRow] = m_gmt_mgr->EmptyRowConnections(iConnect);
    }
   
    p_barrelDesign2->addMultipleRowConnection(minRow, connections);

  } else {
    // No empty rows.
    if (!m_gmt_mgr->ibl()) {
      m_gmt_mgr->msg(MSG::WARNING) << "GeoPixelSiCrystal: No ganged pixels" << endmsg;
    } else {
      if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) << "GeoPixelSiCrystal: No ganged pixels" << endmsg;
    }
  }
  
  // Check that the active area seems reasonable

  if ( (m_gmt_mgr->DesignRPActiveArea(m_isModule3D) > width) ||
       (m_gmt_mgr->DesignZActiveArea(m_isModule3D) >  length) || 
       (width - m_gmt_mgr->DesignRPActiveArea(m_isModule3D) > 4 * Gaudi::Units::mm) || 
       (length - m_gmt_mgr->DesignZActiveArea(m_isModule3D) > 4 * Gaudi::Units::mm) ) { 
    m_gmt_mgr->msg(MSG::WARNING) << "GeoPixelSiCrystal: Active area not consistent with sensor size. Sensor: " 
			       << width/Gaudi::Units::mm << " x " << length/Gaudi::Units::mm << ", Active: " 
			       << m_gmt_mgr->DesignRPActiveArea(m_isModule3D)/Gaudi::Units::mm << " x " << m_gmt_mgr->DesignZActiveArea(m_isModule3D)/Gaudi::Units::mm 
			       << endmsg;
  } else {
    if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) 
      << "GeoPixelSiCrystal: Sensor: "  
      << width/Gaudi::Units::mm << " x " << length/Gaudi::Units::mm << ", Active: " 
      << m_gmt_mgr->DesignRPActiveArea(m_isModule3D)/Gaudi::Units::mm << " x " << m_gmt_mgr->DesignZActiveArea(m_isModule3D)/Gaudi::Units::mm 
      << endmsg;		       
  }


  m_design = m_DDmgr->addDesign(std::move(p_barrelDesign2));
  
  
}
GeoVPhysVol* GeoPixelSiCrystal::Build() {
  //(sar) code moved from c'tor..
  const double thickness = m_gmt_mgr->PixelBoardThickness(m_isModule3D);
  const double length = m_gmt_mgr->PixelBoardLength(m_isModule3D);
  const double width = m_gmt_mgr->PixelBoardWidth(m_isModule3D);
  //
  std::string matName = m_gmt_mgr->getMaterialName("Sensor");
  const GeoMaterial* siMat = m_mat_mgr->getMaterial(matName);
  const GeoBox* siBox = new GeoBox(thickness*0.5,width*0.5,length*0.5);
  std::string logname{"siLog"};
  // There is not a strong need to give the blayer a different name but leave it for now. 
  if(m_isBLayer) logname = "siBLayLog";
  auto *logVolume = new GeoLogVol(logname,siBox,siMat);
  //(sar) ...to here


  GeoFullPhysVol* siPhys = new GeoFullPhysVol(logVolume);

  //
  // Add this to the list of detector elements:
  //
  int brl_ec=0;

  // Build the Identifier for the silicon:
  //
  if(m_gmt_mgr->isBarrel() ) brl_ec = 0;
  if(m_gmt_mgr->isEndcap() ) brl_ec = 2*m_gmt_mgr->GetSide();
  const PixelID * idHelper = m_gmt_mgr->getIdHelper();
  Identifier idwafer;
  idwafer = idHelper->wafer_id(brl_ec,m_gmt_mgr->GetLD(),m_gmt_mgr->Phi(),m_gmt_mgr->Eta());
 
  m_id=idwafer;
  SiDetectorElement * element = new SiDetectorElement(idwafer, m_design, siPhys, m_gmt_mgr->commonItems());
  
  
  // add the element to the manager
  m_DDmgr->addDetectorElement(element);
  return siPhys;
}
 

std::shared_ptr<const PixelDiodeMatrix>  GeoPixelSiCrystal::makeMatrix(double phiPitch, double etaPitch, double etaPitchLong, double etaPitchLongEnd,
						  int circuitsPhi, int circuitsEta, int diodeRowPerCirc, int diodeColPerCirc)
{
  // There are several different cases. Not all are used at the time of wrtiting the code but I
  // have tried to consider all possible cases for completeness. 
  //
  // end cell : middle cells : between chip 
  // --------------------------------------
  // long:normal:long (standard ATLAS case)
  // normal:normal:normal
  // normal:normal:long (> 2 chips)
  // normal:normal:long (2 chips)
  // end:normal:long    (not likely)
  // end:normal:normal  (not likely)
  // end:normal:end  (if single chip)

  std::shared_ptr<const PixelDiodeMatrix> fullMatrix = nullptr;
  
  if (etaPitchLongEnd == etaPitchLong && etaPitchLong != etaPitch) {
    // long:normal:long (standard ATLAS case)
    if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) <<  "GeoPixelSiCrystal: Making matrix (long:normal:long, Standard ATLAS case)" << endmsg;

    std::shared_ptr<const PixelDiodeMatrix> normalCell = PixelDiodeMatrix::construct(phiPitch, etaPitch); 
    std::shared_ptr<const PixelDiodeMatrix> bigCell = PixelDiodeMatrix::construct(phiPitch, etaPitchLong); 
    
    std::shared_ptr<const PixelDiodeMatrix> singleChipRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
							    bigCell, 
							    normalCell, 
							    diodeColPerCirc-2,
							    bigCell);

    std::shared_ptr<const PixelDiodeMatrix> singleRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
							nullptr, singleChipRow, circuitsEta, nullptr);

    fullMatrix = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir,
				      nullptr, singleRow, circuitsPhi*diodeRowPerCirc, nullptr);
  } else if (etaPitchLongEnd == etaPitchLong && (etaPitchLong == etaPitch || circuitsEta == 1)) {
    // normal:normal:normal
    if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) <<  "GeoPixelSiCrystal: Making matrix (normal:normal:normal)" << endmsg;
    std::shared_ptr<const PixelDiodeMatrix> normalCell = PixelDiodeMatrix::construct(phiPitch, etaPitch); 
    std::shared_ptr<const PixelDiodeMatrix> singleRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
							nullptr, normalCell, circuitsEta*diodeColPerCirc, nullptr);
    fullMatrix = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir,
				      nullptr, singleRow, circuitsPhi*diodeRowPerCirc, nullptr);
  } else if (etaPitchLongEnd == etaPitch &&  etaPitchLong != etaPitch && circuitsEta > 2) {
    if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) <<  "GeoPixelSiCrystal: Making matrix (normal:normal:long, > 2 chips)" << endmsg;
    // normal:normal:long: > 2 chips
    std::shared_ptr<const PixelDiodeMatrix> normalCell = PixelDiodeMatrix::construct(phiPitch, etaPitch); 
    std::shared_ptr<const PixelDiodeMatrix> bigCell = PixelDiodeMatrix::construct(phiPitch, etaPitchLong); 
    
    std::shared_ptr<const PixelDiodeMatrix> lowerSingleChipRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
								 nullptr, 
								 normalCell, 
								 diodeColPerCirc-1,
								 bigCell);
    std::shared_ptr<const PixelDiodeMatrix> middleSingleChipRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
								  bigCell, 
								  normalCell, 
								  diodeColPerCirc-2,
								  bigCell);
    std::shared_ptr<const PixelDiodeMatrix> upperSingleChipRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
								 bigCell, 
								 normalCell, 
								 diodeColPerCirc-1,
								 nullptr);
    std::shared_ptr<const PixelDiodeMatrix> singleRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
							lowerSingleChipRow, middleSingleChipRow, circuitsEta-2, upperSingleChipRow);
    fullMatrix = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir,
				      nullptr, singleRow, circuitsPhi*diodeRowPerCirc, nullptr);
  } else if (etaPitchLongEnd == etaPitch &&  etaPitchLong != etaPitch && circuitsEta == 2) {
    // normal:normal:long: 2 chips (current SLHC case)
    if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) <<  "GeoPixelSiCrystal: Making matrix (normal:normal:long, 2 chips)" << endmsg;
    std::shared_ptr<const PixelDiodeMatrix> normalCell = PixelDiodeMatrix::construct(phiPitch, etaPitch); 
    std::shared_ptr<const PixelDiodeMatrix> bigCell = PixelDiodeMatrix::construct(phiPitch, etaPitchLong); 
    
    std::shared_ptr<const PixelDiodeMatrix> lowerSingleChipRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
								 nullptr, 
								 normalCell, 
								 diodeColPerCirc-1,
								 bigCell);
    std::shared_ptr<const PixelDiodeMatrix> upperSingleChipRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
								 bigCell, 
								 normalCell, 
								 diodeColPerCirc-1,
								 nullptr);
    std::shared_ptr<const PixelDiodeMatrix> singleRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
							lowerSingleChipRow, upperSingleChipRow, 1, nullptr);
    fullMatrix = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir,
				      nullptr, singleRow, circuitsPhi*diodeRowPerCirc, nullptr);
  } else if (circuitsEta == 1 ||  (etaPitchLongEnd != etaPitch &&  etaPitchLong == etaPitch )){ // etaPitchLongEnd != etaPitch at this stage
    // end:normal:end  (for single chip)
    // end:normal:normal  (not likely)
    if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) <<  "GeoPixelSiCrystal: Making matrix (end:normal:end, single chips or end:normal:normal)" << endmsg;
    std::shared_ptr<const PixelDiodeMatrix> normalCell = PixelDiodeMatrix::construct(phiPitch, etaPitch); 
    std::shared_ptr<const PixelDiodeMatrix> bigCell = PixelDiodeMatrix::construct(phiPitch, etaPitchLongEnd); 
    
    std::shared_ptr<const PixelDiodeMatrix> singleRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
							    bigCell, 
							    normalCell, 
							    circuitsEta*diodeColPerCirc-2,
							    bigCell);
    fullMatrix = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir,
				      nullptr, singleRow, circuitsPhi*diodeRowPerCirc, nullptr);
  } else {
    // end:normal:long    (not likely)
    if (m_gmt_mgr->msgLvl(MSG::DEBUG)) m_gmt_mgr->msg(MSG::DEBUG) <<  "GeoPixelSiCrystal: Making matrix (end:normal:long)" << endmsg;
    std::shared_ptr<const PixelDiodeMatrix> normalCell = PixelDiodeMatrix::construct(phiPitch, etaPitch); 
    std::shared_ptr<const PixelDiodeMatrix> bigCell = PixelDiodeMatrix::construct(phiPitch, etaPitchLong); 
    std::shared_ptr<const PixelDiodeMatrix> endCell = PixelDiodeMatrix::construct(phiPitch, etaPitchLongEnd); 
    
    std::shared_ptr<const PixelDiodeMatrix> lowerSingleChipRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
								 endCell, 
								 normalCell, 
								 diodeColPerCirc-2,
								 bigCell);
    std::shared_ptr<const PixelDiodeMatrix> middleSingleChipRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
								  bigCell, 
								  normalCell, 
								  diodeColPerCirc-2,
								  bigCell);
    std::shared_ptr<const PixelDiodeMatrix> upperSingleChipRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
								 bigCell, 
								 normalCell, 
								 diodeColPerCirc-2,
								 endCell);
    std::shared_ptr<const PixelDiodeMatrix> singleRow = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir,
							lowerSingleChipRow, middleSingleChipRow, circuitsEta-2, upperSingleChipRow);
    fullMatrix = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir,
				      nullptr, singleRow, circuitsPhi*diodeRowPerCirc, nullptr);
    
  }

  return fullMatrix;
}
