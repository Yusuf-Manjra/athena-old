/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
//
// Class TileGeoG4SD
// Sensitive detector for TileCal G4 simulations with TileGeoModel
//
// Author: Vakho Tsulaia <Vakhtang.Tsulaia@cern.ch>
//
// Major updates: July, 2013 (Sergey)
//
//************************************************************

// class header
#include "TileGeoG4SD.hh"
// package headers
#include "TileG4Interfaces/ITileCalculator.h"
#include "TileGeoG4SD/TileSDOptions.h"
#include "TileGeoG4SD/TileGeoG4LookupBuilder.hh"
#include "TileGeoG4SD/TileGeoG4Lookup.hh"
// Geant4 headers
#include "G4Step.hh"
#include "G4Geantino.hh"

TileGeoG4SD::TileGeoG4SD(G4String name, const std::string& hitCollectionName, ITileCalculator* tileCalculator)
  : G4VSensitiveDetector(name)
  , m_calc(tileCalculator)
  , m_HitColl(hitCollectionName)
{

  verboseLevel = std::max(verboseLevel, m_calc->GetOptions()->verboseLevel);

  //build tilecal ordinary look-up table
  m_lookup = m_calc->GetLookupBuilder();
  if (verboseLevel >= 5)
    G4cout << "Lookup built for Tile" << G4endl;
}

TileGeoG4SD::~TileGeoG4SD() {
}

void TileGeoG4SD::Initialize(G4HCofThisEvent* /*HCE*/) {
  if (!m_HitColl.isValid())
    m_HitColl = std::make_unique<TileHitVector>(m_HitColl.name());
}

G4bool TileGeoG4SD::ProcessHits(G4Step* aStep, G4TouchableHistory* /*ROhist*/) {

  if (aStep->GetTotalEnergyDeposit() == 0. && aStep->GetTrack()->GetDefinition() != G4Geantino::GeantinoDefinition()) {

    if (verboseLevel >= 10)
      G4cout << "ProcessHits: Edep=0" << G4endl;
    return false;
  }

  TileHitData hitData;
  if (! (m_calc->FindTileScinSection(aStep, hitData))) { //Search for the tilecal sub-section, its module and some identifiers

    if (verboseLevel >= 10)
      G4cout << "ProcessHits: FindTileScinSection(aStep) is false!" << G4endl;
    return false;
  }

  double deltaTime(0);

  if ( !(m_calc->MakePmtEdepTime(aStep, hitData, deltaTime)) ) { //calculation of pmtID, edep and scin_Time with aStep (Sergey)

    if (verboseLevel >= 10)
      G4cout << "ProcessHits: wrong pmtID_up,pmtID_down,edep_up,edep_down,"
             << "scin_Time_up,scin_Time_down:\t" << hitData.pmtID_up
             << "\t" << hitData.pmtID_down
             << "\t" << hitData.edep_up
             << "\t" << hitData.edep_down
             << "\t" << hitData.scin_Time_up
             << "\t" << hitData.scin_Time_down << G4endl;
    return false;
  }

  if ( !(m_calc->ManageScintHit(hitData, deltaTime)) ) { //create or update hit object in the collection

    G4cout << "ProcessHits: TileHit can not be produced; pmtID_up,pmtID_down,edep_up,edep_down,"
           << "scin_Time_up,scin_Time_down:\t" << hitData.pmtID_up
           << "\t" << hitData.pmtID_down
           << "\t" << hitData.edep_up
           << "\t" << hitData.edep_down
           << "\t" << hitData.scin_Time_up
           << "\t" << hitData.scin_Time_down << G4endl;
    return false;
  }

  return true;
}

void TileGeoG4SD::EndOfAthenaEvent() {
  m_lookup->ResetCells(&*m_HitColl);
}

