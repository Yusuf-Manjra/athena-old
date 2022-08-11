/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Primary include
#include "VoxelDensityTool.h"

// Geant4 includes
#include "G4LogicalVolumeStore.hh"

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
VoxelDensityTool::VoxelDensityTool(const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent)
  : GeometryConfigurationToolBase(type, name, parent)
{
}

StatusCode VoxelDensityTool::preGeometryConfigure() const
{
  ATH_MSG_DEBUG("VoxelDensityTool::preGeometryConfigure()");

  return StatusCode::SUCCESS;
}

StatusCode VoxelDensityTool::postGeometryConfigure() const
{
  ATH_MSG_INFO("VoxelDensityTool::postGeometryConfigure()");
  // Set VoxelDensity
  G4LogicalVolumeStore *logicalVolumeStore = G4LogicalVolumeStore::GetInstance();
  if (logicalVolumeStore->size() == 0) {
    ATH_MSG_ERROR( "G4 logical volume store is empty." );
  }
  const G4String muonSys("Muon::MuonSys");
  const G4String embSTAC("LArMgr::LAr::EMB::STAC");
  bool ilvMuonSys = false, ilvEmbSTAC = false;
  for (auto* ilv : *logicalVolumeStore ) {
    if ( ilv->GetName() == muonSys ) {
      ilv->SetSmartless( 0.1 );
      ATH_MSG_INFO( "Set VoxelDensity for Muon::MuonSys to 0.1" );
      ilvMuonSys = true;
    }
    else if ( ilv->GetName() == embSTAC ) {
      ilv->SetSmartless( 0.5 );
      ATH_MSG_INFO( "Set VoxelDensity for LArMgr::LAr::EMB::STAC to 0.5" );
      ilvEmbSTAC = true;
    }

    //Now for any volumes set via job property std::pair<volume name, value>...
    for (auto& volToSet:m_volumeVoxelDensityLevel) {
      if (ilv->GetName().contains(volToSet.first)) {
        ilv->SetSmartless(volToSet.second);
        ATH_MSG_INFO("Set VoxelDensity for "<<ilv->GetName()<<" to "<<volToSet.second);
      }
    }

  }
  if (ilvMuonSys == false) {
    ATH_MSG_INFO( "Muon::MuonSys not in G4 logical volume store. VoxelDensity not set." );
  }
  if (ilvEmbSTAC == false) {
    ATH_MSG_INFO( "LArMgr::LAr::EMB::STAC not in G4 logical volume store. VoxelDensity not set." );
  }

  return StatusCode::SUCCESS;
}
