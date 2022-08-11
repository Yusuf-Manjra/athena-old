/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "src/LUCID_SensitiveDetector.h"

#include "gtest/gtest.h"

#include <vector>

#include "TestTools/initGaudi.h"

#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"

#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4DynamicParticle.hh"
#include "G4ThreeVector.hh"
#include "G4Box.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4SystemOfUnits.hh"

#include "G4AtlasTools/DerivedG4PhysicalVolume.h"
#include "G4AtlasTools/DerivedG4SensitiveDetectorTestSetting.h"

//set environment
class GaudiEnvironment : public ::testing::Environment {
  protected:
  virtual void SetUp() override {
    Athena_test::initGaudi("LUCID_G4_SD/optionForTest.txt", m_svcLoc);
  }
  ISvcLocator* m_svcLoc = nullptr;
};
class LUCID_SensitiveDetectortest : public ::testing::Test {	
  protected:
    virtual void SetUp() override {
    }

    virtual void TearDown() override {
    }
};
//end of environment setting

TEST_F( LUCID_SensitiveDetectortest, Initialize )
{
  G4HCofThisEvent hce;
  LUCID_SensitiveDetector sd1( "name1", "name1" );
  sd1.Initialize(&hce);
  ASSERT_TRUE(sd1.m_HitColl.isValid()); //test if the initialization of the smart pointer m_HitColl is successful
}

TEST_F( LUCID_SensitiveDetectortest, ProcessHits )
{
  G4HCofThisEvent hce;
  G4Step sp;
  G4TouchableHistory th;

  G4double totalenergydeposit = 0.8;
  std::vector<G4String> physicalname = {"BBBBBTubeGas50"};
  G4String logicalname = "BBBBBBBBBTubeGas";
  std::vector<G4int> copynos = {2009};
  G4ThreeVector preStepPos = G4ThreeVector(0,0,1);
  G4ThreeVector postStepPos = G4ThreeVector(0,0,2);
  G4double globaltime0 = 0.5;
  G4double kineticenergy0 = 1.5;
  G4double velocity0 = 99.93100;
  G4double globaltime = 5.0;
  G4double kineticenergy = 0.5;
  G4double globaltime1 = 0.5;
  G4double kineticenergy1 = 0.5;
  G4double velocity1 = 99.93100;
  G4double steplength = 1.0;
  G4double charge = 0.0;
  G4int encoding = 0;
  G4int antiencoding = 0;
  G4String astring = "Cerenkov";
  G4ProcessType atype = (G4ProcessType)0;
  G4String nop1 = "opticalphoton";
  G4String nop2 = "opticalphoton";
  G4String nop3 = "photon";
  DerivedG4SensitiveDetectorTestSetting(sp, totalenergydeposit, physicalname, logicalname, copynos, preStepPos, postStepPos, globaltime0, kineticenergy0, velocity0, globaltime, kineticenergy, globaltime1, kineticenergy1, velocity1, steplength, charge, encoding, antiencoding, astring, atype, nop1, nop2, nop3);

  LUCID_SensitiveDetector sd2( "name2", "name2" );
  sd2.Initialize(&hce);
  sd2.ProcessHits(&sp, &th);

  LUCID_SimHitCollection * a = sd2.m_HitColl.ptr();
  ASSERT_EQ( a->begin()->GetTubeID(), 50 ); //test the TubeID value of the Hit generated by the member function ProcessHits and stored in the smart point m_HitColl, it should be 50 based on my setting. The same below
  ASSERT_EQ( a->begin()->GetTrack(), 0 );
  ASSERT_TRUE( a->begin()->GetPdgCode() == 3 );
  ASSERT_TRUE( a->begin()->GetGenVolume() == 2 );
  ASSERT_TRUE( a->begin()->GetX() == 0 );
  ASSERT_TRUE( a->begin()->GetY() == 0 );
  ASSERT_TRUE( a->begin()->GetZ() == 1 );
  ASSERT_TRUE( a->begin()->GetEPX() == 0 );
  ASSERT_TRUE( a->begin()->GetEPY() == 0 );
  ASSERT_TRUE( a->begin()->GetEPZ() == 2 );
  ASSERT_TRUE( a->begin()->GetPreStepTime() == 0.5 );
  ASSERT_TRUE( a->begin()->GetPostStepTime() == 0.5 );
  ASSERT_FLOAT_EQ( a->begin()->GetWavelength(), 0.0024796838 );
  ASSERT_FLOAT_EQ( a->begin()->GetEnergy(), 500000 );
}

TEST_F( LUCID_SensitiveDetectortest, AddHit )
{

  G4HCofThisEvent hce;
  LUCID_SensitiveDetector sd3( "name3", "name3" );
  sd3.Initialize(&hce);

  short tubeID = 1;
  int   pdgCode = 2;
  int   track = 3;
  int   genVolume =4;
  float stepStartPosX = 1.0;
  float stepStartPosY = 2.0;
  float stepStartPosZ = 3.0;
  float stepEndPosX = 4.0;
  float stepEndPosY = 5.0;
  float stepEndPosZ = 6.0;
  float preStepTime = 7.0;
  float postStepTime = 8.0;
  float wavelength = 9.0;
  float energy = 10.0;
  sd3.AddHit( tubeID, pdgCode, track, genVolume, stepStartPosX, stepStartPosY, stepStartPosZ, stepEndPosX, stepEndPosY, stepEndPosZ, preStepTime, postStepTime, wavelength, energy);

  LUCID_SimHitCollection * a = sd3.m_HitColl.ptr();
  ASSERT_TRUE( a->begin()->GetTubeID() == 1 ); //test the TubeID value of the Hit added by the member function AddHit, it should be 1 since the input value is 1. Yhe same below
  ASSERT_TRUE( a->begin()->GetPdgCode() == 2 );
  ASSERT_TRUE( a->begin()->GetTrack() == 3 );
  ASSERT_TRUE( a->begin()->GetGenVolume() == 4 );
  ASSERT_TRUE( a->begin()->GetX() == 1.0 );
  ASSERT_TRUE( a->begin()->GetY() == 2.0 );
  ASSERT_TRUE( a->begin()->GetZ() == 3.0 );
  ASSERT_TRUE( a->begin()->GetEPX() == 4.0 );
  ASSERT_TRUE( a->begin()->GetEPY() == 5.0 );
  ASSERT_TRUE( a->begin()->GetEPZ() == 6.0 );
  ASSERT_TRUE( a->begin()->GetPreStepTime() == 7.0 );
  ASSERT_TRUE( a->begin()->GetPostStepTime() == 8.0 );
  ASSERT_FLOAT_EQ( a->begin()->GetWavelength(), 9.0 );
  ASSERT_FLOAT_EQ( a->begin()->GetEnergy(), 10.0 );

}

int main( int argc, char** argv ) {

  auto g=new GaudiEnvironment;
  ::testing::AddGlobalTestEnvironment(g);
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();

}

