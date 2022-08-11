/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// Main header
#include "G4AtlasTools/DetectorFieldManagerTool.h"

// From this package
#include "TightMuonElseNoFieldManager.h"
#include "SwitchingFieldManager.h"

// Geant4 includes
#include "G4ChordFinder.hh"
#include "G4FieldManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4MagIntegratorStepper.hh"
#include "G4MagneticField.hh"
#include "G4Version.hh"
#include "G4VIntegrationDriver.hh"

//-----------------------------------------------------------------------------
// Tool constructor
//-----------------------------------------------------------------------------
DetectorFieldManagerTool::DetectorFieldManagerTool(const std::string& type,
                                                   const std::string& name,
                                                   const IInterface* parent)
  : G4FieldManagerToolBase(type, name, parent)
{
}

//-----------------------------------------------------------------------------
// Initialize a field manager
//-----------------------------------------------------------------------------
StatusCode DetectorFieldManagerTool::initializeField()
{
  ATH_MSG_DEBUG("initializeField");

  if (m_fieldOn) {

    // If field manager already exists for current thread, error.
    // There is no foreseen use-case for this situation.
    if(m_fieldMgrHolder.get()) {
      ATH_MSG_ERROR("DetectorFieldManagerTool::initializeField() - " <<
                    "Field manager already exists!");
      return StatusCode::FAILURE;
    }

    // Retrieve the G4MagneticField
    G4MagneticField* field = m_fieldSvc->getField();

    // Create a new field manager
    G4FieldManager * fieldMgr = nullptr;
    if (m_muonOnlyField){
      // fieldMgr = new TightMuonElseNoFieldManager();
      fieldMgr = new SwitchingFieldManager(field);
    } else {
      fieldMgr = new G4FieldManager();
    }

    // Save it in the TL holder
    m_fieldMgrHolder.set(fieldMgr);

    // Configure the field manager
    fieldMgr->SetDetectorField(field);
    fieldMgr->CreateChordFinder(field);
    ATH_CHECK( setFieldParameters(fieldMgr) );

    // Create and configure the ChordFinder
    fieldMgr->CreateChordFinder(field);

#if G4VERSION_NUMBER < 1040
    ATH_MSG_DEBUG("Old style stepper setting");
    G4MagIntegratorStepper* stepper = getStepper(m_integratorStepper, field);
    G4MagInt_Driver* magDriver = fieldMgr->GetChordFinder()->GetIntegrationDriver();
    magDriver->RenewStepperAndAdjust(stepper);
#else
    ATH_MSG_DEBUG("New style stepper setting");
    G4VIntegrationDriver* driver = createDriverAndStepper(m_integratorStepper, field);
    G4ChordFinder* chordFinder = fieldMgr->GetChordFinder();
    chordFinder->SetIntegrationDriver(driver);
#endif

    // Assign the field manager to volumes
    if (!m_logVolumeList.empty()) {
      auto logVolStore = G4LogicalVolumeStore::GetInstance();
      for (const auto& volume: m_logVolumeList) {
        G4LogicalVolume* logicalVolume = logVolStore->GetVolume(volume);
        if (logicalVolume != nullptr) logicalVolume->SetFieldManager(fieldMgr, true);
        else
          ATH_MSG_WARNING("No volume called " << volume << " was found in the G4LogicalVolumeStore! Skipping this volume.");
      }
    }
    else if (!m_physVolumeList.empty()) {
      auto physVolStore = G4PhysicalVolumeStore::GetInstance();
      for (const auto& volume: m_physVolumeList) {
        G4VPhysicalVolume* physicalVolume = physVolStore->GetVolume(volume);
        if (physicalVolume != nullptr) physicalVolume->GetLogicalVolume()->SetFieldManager(fieldMgr, true);
        else
          ATH_MSG_WARNING("No volume called " << volume << " was found in the G4PhysicalVolumeStore! Skipping this volume.");
      }
    }
    else
      ATH_MSG_WARNING("No volumes are provided. Field manager is NOT assigned.");

  }

  return StatusCode::SUCCESS;
}
