/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file TrkTruthTPCnv/test/PRD_MultiTruthCollectionCnv_p2_test.cxx
 * @date Feb, 2018
 * @brief Tests for PRD_MultiTruthCollectionCnv_p2.
 */


#undef NDEBUG
#include "TrkTruthTPCnv/PRD_MultiTruthCollectionCnv_p2.h"
#include "TrkTruthTPCnv/PRD_MultiTruthCollection_p2.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "StoreGate/StoreGateSvc.h"
#include "SGTools/TestStore.h"
#include "GeneratorObjectsTPCnv/initMcEventCollection.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include <cassert>
#include <iostream>


void compare (const HepMcParticleLink& p1,
              const HepMcParticleLink& p2)
{
  assert ( p1.isValid() == p2.isValid() );
  assert ( p1.barcode() == p2.barcode() );
  assert ( p1.eventIndex() == p2.eventIndex() );
  assert ( p1.getEventCollectionAsChar() == p2.getEventCollectionAsChar() );
  assert ( p1.cptr() == p2.cptr() );
  assert ( p1 == p2 );
}


void compare (const PRD_MultiTruthCollection& p1,
              const PRD_MultiTruthCollection& p2)
{
  assert (p1.size() == p2.size());
  PRD_MultiTruthCollection::const_iterator i1 = p1.begin();
  PRD_MultiTruthCollection::const_iterator i2 = p2.begin();
  for (; i1 != p1.end(); ++i1, ++i2) {
    assert (i1->first == i2->first);
    compare (i1->second, i2->second);
    assert (i1->second == i2->second);
  }
}


void testit (const PRD_MultiTruthCollection& trans1)
{
  MsgStream log (nullptr, "test");
  PRD_MultiTruthCollectionCnv_p2 cnv;
  Trk::PRD_MultiTruthCollection_p2 pers;
  cnv.transToPers (&trans1, &pers, log);
  PRD_MultiTruthCollection trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


void test1(std::vector<HepMC::GenParticlePtr> genPartVector)
{
  std::cout << "test1\n";

  PRD_MultiTruthCollection trans1;
  for (int i=0; i<10; i++) {
    auto pGenParticle = genPartVector.at(i);
    HepMcParticleLink trkLink(HepMC::barcode(pGenParticle),pGenParticle->parent_event()->event_number());
    const Identifier chanId(i+100);
    trans1.insert(trans1.end(), std::make_pair(chanId, trkLink) );
  }

  testit (trans1);
}


int main()
{
  ISvcLocator* pSvcLoc = nullptr;
  std::vector<HepMC::GenParticlePtr> genPartVector;
  if (!Athena_test::initMcEventCollection(pSvcLoc,genPartVector)) {
    std::cerr << "This test can not be run" << std::endl;
    return 0;
  }
  //SGTest::initTestStore();
  test1(genPartVector);
  return 0;
}
