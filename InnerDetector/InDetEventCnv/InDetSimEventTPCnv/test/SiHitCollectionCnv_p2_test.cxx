/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file InDetSimEventTPCnv/test/SiHitCollectionCnv_p2_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2016
 * @brief Tests for SiHitCollectionCnv_p2.
 */


#undef NDEBUG
#include "InDetSimEventTPCnv/InDetHits/SiHitCollectionCnv_p2.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/leakcheck.h"
#include <cassert>
#include <iostream>

#include "GeneratorObjectsTPCnv/initMcEventCollection.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/Operators.h"


void compare (const HepMcParticleLink& p1,
              const HepMcParticleLink& p2)
{
  assert ( p1.isValid() == p2.isValid() );
  assert ( HepMC::barcode(p1) == HepMC::barcode(p2) );
  assert ( p1.eventIndex() == p2.eventIndex() );
  assert ( p1.getEventCollectionAsChar() == p2.getEventCollectionAsChar() );
  assert ( p1.cptr() == p2.cptr() );
  assert ( p1 == p2 );
}

void compare (const SiHit& p1,
              const SiHit& p2)
{
  assert (p1.localStartPosition() == p2.localStartPosition());
  assert (p1.localEndPosition() == p2.localEndPosition());
  assert (p1.energyLoss() == p2.energyLoss());
  assert (p1.meanTime() == p2.meanTime());
  compare(p1.particleLink(), p2.particleLink());
  assert (p1.particleLink() == p2.particleLink());
  assert (p1.identify() == p2.identify());
}


void compare (const SiHitCollection& p1,
              const SiHitCollection& p2)
{
  //assert (p1.Name() == p2.Name());
  assert (p1.size() == p2.size());
  for (size_t i = 0; i < p1.size(); i++)
    compare (p1[i], p2[i]);
}


void testit (const SiHitCollection& trans1)
{
  MsgStream log (nullptr, "test");
  SiHitCollectionCnv_p2 cnv;
  SiHitCollection_p2 pers;
  cnv.transToPers (&trans1, &pers, log);
  SiHitCollection trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE (std::vector<HepMC::GenParticlePtr>& genPartVector)
{
  std::cout << "test1\n";
  auto particle = genPartVector.at(0);
  // Create HepMcParticleLink outside of leak check.
  HepMcParticleLink dummyHMPL(HepMC::barcode(particle),particle->parent_event()->event_number());
  assert(dummyHMPL.cptr()==particle);
  // Create DVL info outside of leak check.
  SiHitCollection dum ("coll");
  Athena_test::Leakcheck check;

  SiHitCollection trans1 ("coll");
  for (int i=0; i < 10; i++) {
    auto pGenParticle = genPartVector.at(i);
    HepMcParticleLink trkLink(HepMC::barcode(pGenParticle),pGenParticle->parent_event()->event_number());
    int o = i*100;
    trans1.Emplace (HepGeom::Point3D<double> (10.5+o, 11.5+o, 12.5+o),
                    HepGeom::Point3D<double> (13.5+o, 14.5+o, 15.5+o),
                    16.5+o,
                    17.5+o,
                    trkLink,
                    19+o);

  }

  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  ISvcLocator* pSvcLoc = nullptr;
  std::vector<HepMC::GenParticlePtr> genPartVector;
  if (!Athena_test::initMcEventCollection(pSvcLoc, genPartVector)) {
    std::cerr << "This test can not be run" << std::endl;
    return 0;
  }

  test1(genPartVector);
  return 0;
}
