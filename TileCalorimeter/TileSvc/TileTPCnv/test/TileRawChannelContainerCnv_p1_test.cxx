/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TileTPCnv/test/TileRawChannelContainerCnv_p1_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2019
 * @brief Tests for TileRawChannelContainerCnv_p1.
 */


#undef NDEBUG
#include "TileTPCnv/TileRawChannelContainerCnv_p1.h"
#include "TileConditions/TileCablingService.h"
#include "TileIdentifier/TileHWID.h"
#include "IdDictParser/IdDictParser.h"
#include "TestTools/initGaudi.h"
#include "TestTools/leakcheck.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/MsgStream.h"
#include <cassert>
#include <iostream>


class TileCablingSvc
{
public:
  TileCablingSvc (IdDictParser& parser) ATLAS_NOT_THREAD_SAFE
  {
    tileid.set_do_neighbours (false);
    IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
    hwid.set_quiet (true);
    tbid.set_quiet (true);
    tileid.set_quiet (true);
    assert (hwid.initialize_from_dictionary (idd) == 0);
    assert (tbid.initialize_from_dictionary (idd) == 0);
    assert (tileid.initialize_from_dictionary (idd) == 0);
    TileCablingService* svc = TileCablingService::getInstance_nc();
    svc->setTileHWID (&hwid);
    svc->setTileTBID (&tbid);
    svc->setTileID (&tileid);
  }

  TileHWID hwid;
  TileTBID tbid;
  TileID   tileid;
};


void compare (const TileRawChannel& p1,
              const TileRawChannel& p2)
{
  assert (p1.identify() == p2.identify());

  assert (p1.size() == p2.size());
  for (int i=0; i < p1.size(); i++)
    assert (p1.amplitude(i) == p2.amplitude(i));
  assert (p1.sizeTime() == p2.sizeTime());
  for (int i=0; i < p1.sizeTime(); i++)
    assert (p1.time(i) == p2.time(i));
  assert (p1.sizeQuality() == p2.sizeQuality());
  for (int i=0; i < p1.sizeQuality(); i++)
    assert (p1.quality(i) == p2.quality(i));
  assert (p1.pedestal() == p2.pedestal());
}


void compare (const TileRawChannelCollection& p1,
              const TileRawChannelCollection& p2)
{
  assert (p1.identify() == p2.identify());
  // These aren't saved.
  //assert (p1.getLvl1Id() == p2.getLvl1Id());
  //assert (p1.getLvl1Type() == p2.getLvl1Type());
  //assert (p1.getDetEvType() == p2.getDetEvType());
  //assert (p1.getRODBCID() == p2.getRODBCID());
  assert (p1.size() == p1.size());
  for (size_t j = 0; j < p1.size(); j++) {
    compare (*p1[j], *p2[j]);
  }
}


void compare (const TileRawChannelContainer& p1,
              const TileRawChannelContainer& p2)
{
  assert (p1.get_hashType() == p2.get_hashType());
  assert (p1.get_unit() == p2.get_unit());

  TileRawChannelContainer::const_iterator it1 = p1.begin();
  TileRawChannelContainer::const_iterator it1e = p1.end();
  TileRawChannelContainer::const_iterator it2 = p2.begin();
  TileRawChannelContainer::const_iterator it2e = p2.end();
  while (it1 != it1e && it2 != it2e) {
    assert (it1.hashId() == it2.hashId());
    compare (**it1, **it2);
    ++it1;
    ++it2;
  }
  assert (it1 == it1e && it2 == it2e);
}


void testit (const TileRawChannelContainer& trans1)
{
  MsgStream log (0, "test");
  TileRawChannelContainerCnv_p1 cnv;
  TileRawChannelContainer_p1 pers;
  cnv.transToPers (&trans1, &pers, log);
  TileRawChannelContainer trans2;
  cnv.persToTrans (&pers, &trans2, log);

  compare (trans1, trans2);
}


std::unique_ptr<const TileRawChannelContainer>
makecont (const TileID& tileid)
{
  auto cont = std::make_unique<TileRawChannelContainer> (false,
                                                       TileFragHash::Default,
                                                       TileRawChannelUnit::ADCcounts);

  std::vector<std::unique_ptr<TileRawChannelCollection> > colls (100);
  for (int hi=2; hi <= 3; hi++) {
    auto coll = std::make_unique<TileRawChannelCollection>(IdentifierHash(hi));
    coll->setLvl1Id (hi + 10);
    coll->setLvl1Type (hi + 20);
    coll->setDetEvType (hi + 30);
    coll->setRODBCID (hi + 40);

    for (int i=0; i < 10; i++) {
      int offs = i*10 + hi*100;

      Identifier id = tileid.adc_id (1, hi*2 - 5, 2, i, 0, 0, 0, true);

      auto elt = std::make_unique<TileRawChannel>
        (TileCablingService::getInstance()->s2h_adc_id (id),
         std::vector<float> {offs + 1.5f},
         std::vector<float> {offs + 2.5f, offs + 3.5f},
         std::vector<float> {offs + 4.5f, offs + 5.5f, offs + 6.5f},
         offs + 7.5);

      int hash = cont->hashFunc().hash(elt->frag_ID());
      if (hash >= static_cast<int>(colls.size())) {
        colls.resize (hash+1);
      }
      if (!colls[hash]) {
        auto coll = std::make_unique<TileRawChannelCollection>(IdentifierHash(elt->frag_ID()));
        coll->setLvl1Id (hash + 10);
        coll->setLvl1Type (hash + 20);
        coll->setDetEvType (hash + 30);
        coll->setRODBCID (hash + 40);
        colls[hash] = std::move(coll);
      }

      colls[hash]->push_back (std::move (elt));
    }
  }

  for (size_t hash = 0; hash < colls.size(); hash++) {
    if (colls[hash]) {
      cont->addCollection (colls[hash].release(), hash).ignore();
    }
  }
  return cont;
}


void test1 ATLAS_NOT_THREAD_SAFE (const TileID& tileid)
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;

  std::unique_ptr<const TileRawChannelContainer> trans1 = makecont (tileid);

  testit (*trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "TileTPCnv/TileRawChannelContainerCnv_p1_test\n";
  IdDictParser parser;
  TileCablingSvc helpers(parser);
  test1 (helpers.tileid);
  return 0;
}
