/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file EventTPCnv/test/EventIDCnv_p2_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2016
 * @brief Regression tests.
 */

#undef NDEBUG
#include "EventTPCnv/EventIDCnv_p2.h"
#include "TestTools/leakcheck.h"
#include "EventInfo/EventID.h"
#include "CxxUtils/checker_macros.h"
#include <cassert>
#include <iostream>


void compare (const EventID& p1,
              const EventID& p2)
{
  assert (p1.run_number() == p2.run_number());
  assert (p1.event_number() == p2.event_number());
  assert (p1.time_stamp() == p2.time_stamp());
  assert (p1.time_stamp_ns_offset() == p2.time_stamp_ns_offset());
  assert (p1.lumi_block() == p2.lumi_block());
  assert (p1.bunch_crossing_id() == p2.bunch_crossing_id());
  assert (p1.detector_mask0() == p2.detector_mask0());
  assert (p1.detector_mask1() == p2.detector_mask1());
  assert (0 == p2.detector_mask2());
  assert (0 == p2.detector_mask3());
}


void testit (const EventID& trans1)
{
  EventIDCnv_p2 cnv;
  std::vector<unsigned int> pers;
  cnv.transToPers (&trans1, pers);
  
  EventID trans2;
  std::vector<unsigned int>::const_iterator it = pers.begin();
  cnv.persToTrans (it, &trans2);
  assert (it == pers.end());
  compare (trans1, trans2);
}


void test1 ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test1\n";
  Athena_test::Leakcheck check;
  

  EventID trans1 (1234, 2345, 3456, 4567, 5678, 6789,
                  4321, 5432, 6543, 7654);
  testit (trans1);
}


int main ATLAS_NOT_THREAD_SAFE ()
{
  test1();
  return 0;
}
