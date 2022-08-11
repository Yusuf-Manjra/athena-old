/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  DataModelAthenaPool/test/NavigableCnv_p1_test.cxx
 * @author scott snyder
 * @date Dec, 2009
 * @brief Regression test for @c NavigableCnv_p1.
 */

// FIXME: This is sadly incomplete.


#undef NDEBUG
#include "Navigation/Navigable.h"
#include "Navigation/NavigableTerminalNode.h"
#include "DataModelAthenaPool/NavigableCnv_p1.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/ThinningCache.h"
#include "AthenaKernel/ThinningDecisionBase.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <vector>
#include <cassert>
#include <iostream>


#include "SGTools/TestStore.h"


class MyVI
  : public DataVector<NavigableTerminalNode>
{
};


CLASS_DEF (MyVI, 104857385, 1)



typedef Navigable<MyVI> NI;
typedef Navigable<MyVI, int> NIpar;


void test1 (SGTest::TestStore& store)
{
  std::cout << "test1\n";
  MsgStream log (0, "test");
  SG::sgkey_t sgkey = store.stringToKey ("key", ClassID_traits<MyVI>::ID());

  NavigableCnv_p1<NI> cnv;
  NI ni1;
  ni1.insertElement (ElementLink<MyVI> ("key", 10));
  ni1.insertElement (ElementLink<MyVI> ("key", 20));
  NavigableCnv_p1<NI>::PersNavigable_t p1;
  cnv.transToPers (ni1, p1, log);
  assert (p1.m_links.m_links.size() == 1);
  assert (p1.m_links.m_links[0].m_SGKeyHash == sgkey);
  assert (p1.m_links.m_elementRefs.size() == 2);
  assert (p1.m_links.m_elementRefs[0].m_elementIndex == 10);
  assert (p1.m_links.m_elementRefs[0].m_nameIndex == 0);
  assert (p1.m_links.m_elementRefs[1].m_elementIndex == 20);
  assert (p1.m_links.m_elementRefs[1].m_nameIndex == 0);

  NI ni2;
  cnv.persToTrans (p1, ni2, log);
  assert (ni2.size() == 2);
  NI::object_iter it = ni2.begin();
  assert (it.getElement().index() == 10);
  assert (it.getElement().dataID() == "key");
  ++it;
  assert (it.getElement().index() == 20);
  assert (it.getElement().dataID() == "key");

  SG::ThinningDecisionBase dec;
  dec.resize (50);
  dec.keepAll();
  dec.thin (10);
  dec.buildIndexMap();

  SG::ThinningCache cache;
  cache.addThinning ("key",
                     std::vector<SG::sgkey_t> {ElementLink<MyVI> ("key", 10).key()},
                     &dec);
  cnv.transToPers (ni1, p1, &cache, log);
  assert (p1.m_links.m_links.size() == 1);
  assert (p1.m_links.m_links[0].m_SGKeyHash == sgkey);
  assert (p1.m_links.m_elementRefs.size() == 2);
  assert (p1.m_links.m_elementRefs[0].m_elementIndex == static_cast<uint32_t>(SG::ThinningDecisionBase::RemovedIdx));
  assert (p1.m_links.m_elementRefs[0].m_nameIndex == static_cast<uint32_t>(SG::ThinningDecisionBase::RemovedIdx));
  assert (p1.m_links.m_elementRefs[1].m_elementIndex == 19);
  assert (p1.m_links.m_elementRefs[1].m_nameIndex == 0);
}


void test2 (SGTest::TestStore& store)
{
  std::cout << "test2\n";
  MsgStream log (0, "test");
  SG::sgkey_t sgkey = store.stringToKey ("key", ClassID_traits<MyVI>::ID());

  NavigableCnv_p1<NIpar> cnv;
  NIpar ni1;
  ni1.insertElement (ElementLink<MyVI> ("key", 10), 101);
  ni1.insertElement (ElementLink<MyVI> ("key", 20), 102);
  NavigableCnv_p1<NIpar>::PersNavigable_t p1;
  cnv.transToPers (ni1, p1, log);
  assert (p1.m_links.m_links.size() == 1);
  assert (p1.m_links.m_links[0].m_SGKeyHash == sgkey);
  assert (p1.m_links.m_elementRefs.size() == 2);
  assert (p1.m_links.m_elementRefs[0].m_elementIndex == 10);
  assert (p1.m_links.m_elementRefs[0].m_nameIndex == 0);
  assert (p1.m_links.m_elementRefs[1].m_elementIndex == 20);
  assert (p1.m_links.m_elementRefs[1].m_nameIndex == 0);
  assert (p1.m_parameters.size() == 2);
  assert (p1.m_parameters[0] == 101);
  assert (p1.m_parameters[1] == 102);

  NIpar ni2;
  cnv.persToTrans (p1, ni2, log);
  assert (ni2.size() == 2);
  NIpar::object_iter it = ni2.begin();
  assert (it.getElement().index() == 10);
  assert (it.getElement().dataID() == "key");
  assert (it.getParameter() == 101);
  ++it;
  assert (it.getElement().index() == 20);
  assert (it.getElement().dataID() == "key");
  assert (it.getParameter() == 102);

  SG::ThinningDecisionBase dec;
  dec.resize (50);
  dec.keepAll();
  dec.thin (10);
  dec.buildIndexMap();

  SG::ThinningCache cache;
  cache.addThinning ("key",
                     std::vector<SG::sgkey_t> {ElementLink<MyVI> ("key", 10).key()},
                     &dec);
  cnv.transToPers (ni1, p1, &cache, log);
  assert (p1.m_links.m_links.size() == 1);
  assert (p1.m_links.m_links[0].m_SGKeyHash == sgkey);
  assert (p1.m_links.m_elementRefs.size() == 2);
  assert (p1.m_links.m_elementRefs[0].m_elementIndex == static_cast<uint32_t>(SG::ThinningDecisionBase::RemovedIdx));
  assert (p1.m_links.m_elementRefs[0].m_nameIndex == static_cast<uint32_t>(SG::ThinningDecisionBase::RemovedIdx));
  assert (p1.m_links.m_elementRefs[1].m_elementIndex == 19);
  assert (p1.m_links.m_elementRefs[1].m_nameIndex == 0);
  assert (p1.m_parameters.size() == 2);
  assert (p1.m_parameters[0] == 101);
  assert (p1.m_parameters[1] == 102);
}


int main()
{
  std::cout << "DataModelAthenaPool/NavigableCnv_p1_test\n";
  std::unique_ptr<SGTest::TestStore> testStore = SGTest::getTestStore();
  test1 (*testStore);
  test2 (*testStore);
  return 0;
}
