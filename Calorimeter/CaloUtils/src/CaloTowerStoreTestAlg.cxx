/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  CaloTowerStoreTestAlg.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2013
 * @brief Regression test for CaloTowerStore.
 */

#undef NDEBUG

#include "CaloTowerStoreTestAlg.h"
#include "CaloUtils/CaloTowerStore.h"
#include <iostream>
#include <cmath>
#include <cassert>


/**
 * @brief Constructor.
 */
CaloTowerStoreTestAlg::CaloTowerStoreTestAlg (const std::string& name,
                                              ISvcLocator* pSvcLocator)
  : AthAlgorithm (name, pSvcLocator)
{
}


StatusCode CaloTowerStoreTestAlg::initialize()
{
  ATH_CHECK(m_caloMgrKey.initialize());
  return StatusCode::SUCCESS;
}

void
CaloTowerStoreTestAlg::iterate_full (const CaloTowerStore& store,
                                     test_tows_t& tows)
{
  CaloTowerStore::tower_iterator it = store.towers();
  size_t sz = store.size();
  tows.resize (sz);
  for (size_t i = 0; i < sz; ++i, ++it) {
    CaloTowerStore::cell_iterator cit = it.firstCell();
    CaloTowerStore::cell_iterator cit_end = it.lastCell();
    for (; cit != cit_end; ++cit)
      tows[i].push_back (std::make_pair (cit.hash(), cit.weight()));
  }
}


void
CaloTowerStoreTestAlg::iterate_subseg (const CaloTowerStore& store,
                                       const CaloTowerSeg::SubSeg& subseg,
                                       test_tows_t& tows)
{
  CaloTowerStore::tower_subseg_iterator it = store.towers(subseg);
  size_t sz = subseg.size();
  tows.resize (sz);
  for (size_t i = 0; i < sz; ++i, ++it) {
    size_t itower = it.itower();
    CaloTowerStore::cell_iterator cit = it.firstCell();
    CaloTowerStore::cell_iterator cit_end = it.lastCell();
    for (; cit != cit_end; ++cit)
      tows[itower].push_back (std::make_pair (cit.hash(), cit.weight()));
  }
}


void
CaloTowerStoreTestAlg::dump_tows (const test_tows_t& t)
{
  for (size_t i = 0; i < t.size(); i++) {
    const test_tow_t& tt = t[i];
    std::cout << " " << i << ": ";
    for (size_t j = 0; j < tt.size(); j++)
      std::cout << tt[j].first << "/" << tt[j].second << " ";
    std::cout << "\n";
  }
}

void
CaloTowerStoreTestAlg::test_subseg_iter (const CaloDetDescrManager* caloDDM,
					 const CaloTowerStore& store1,
                                         const std::vector<CaloCell_ID::SUBCALO>& calos,
                                         const CaloTowerSeg::SubSeg& subseg)
{
  CaloTowerSeg seg = subseg.segmentation();
  CaloTowerStore store2;
  if (!store2.buildLookUp (*caloDDM, seg, calos)) {
    std::abort();
  }

  assert (subseg.size() == store2.size());

  test_tows_t tows_full;
  iterate_full (store2, tows_full);

  test_tows_t tows_subseg;
  iterate_subseg (store1, subseg, tows_subseg);

  assert (tows_full == tows_subseg);
}


void CaloTowerStoreTestAlg::test1()
{
  std::cout << "test1\n";

  // Cannot do this in initialize: see ATLASRECTS-5012
  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};
  if(!caloMgrHandle.isValid()) std::abort();

  const CaloDetDescrManager* caloDDM = *caloMgrHandle;

  CaloTowerSeg seg (50, 64, -2.5, 2.5);
  std::vector<CaloCell_ID::SUBCALO> calos;
  calos.push_back (CaloCell_ID::LAREM);
  calos.push_back (CaloCell_ID::LARHEC);
  calos.push_back (CaloCell_ID::TILE);
  CaloTowerStore store;
  if (!store.buildLookUp (*caloDDM, seg, calos)) {
    std::abort();
  }

  CaloTowerSeg::SubSeg subseg1 = seg.subseg (0.7, 0.3, -0.2, 0.4);
  test_subseg_iter (caloDDM, store, calos, subseg1);

  CaloTowerSeg::SubSeg subseg2 = seg.subseg (0.7, 0.3, 3.1, 0.4);
  test_subseg_iter (caloDDM, store, calos, subseg2);
}


/** 
 * @brief Standard Gaudi execute method.
 */
StatusCode CaloTowerStoreTestAlg::execute()
{
  test1();
  return StatusCode::SUCCESS;
}
