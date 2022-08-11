// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TestTools/leakcheck.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2016
 * @brief Pitifully simple leak checker for unit tests.
 *        Just verifies that allocations/deallocations
 *        match over an RAII region.
 *
 *        Overrides the standard new/delete functions, so should not be
 *        #included from any code that would be included in a library.
 */


#ifndef TESTTOOLS_LEAKCHECK_H
#define TESTTOOLS_LEAKCHECK_H

// Can't use CxxUtils/checker_macros.h here, since that would be circular dependency.
#ifdef ATLAS_GCC_CHECKERS
#pragma ATLAS no_check_thread_safety
#endif

// gcc11 can emit Wmismatched-new-delete for our new/delete
// overloads. This is sensitive to optimisations and
// particulary inlining.
// Specifically, it can inline the delete and not
// the new. So it ends up seeing a new matched
// with a free.
//
// We try to disable the diagnostic
// and also we try to avoid inlining the functions.
//
#if __GNUC__ == 11 
# pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif



#include <malloc.h>
#include <unordered_set>
#include <iostream>
#include <cassert>


namespace Athena_test {
std::unordered_set<void*>* allocs = nullptr;
struct LeakCheckDisable
{
  LeakCheckDisable() : m_allocs(allocs) { allocs = nullptr; }
  ~LeakCheckDisable() { allocs = m_allocs; }
  void insert(void* p) { if (m_allocs) m_allocs->insert(p); }
  void erase(void* p) { if (m_allocs) m_allocs->erase(p); }
  std::unordered_set<void*>* m_allocs;
};
} // namespace Athena_test

#if __GNUC__ == 11
[[gnu::noinline]]
#endif
void* newImpl(std::size_t size)
{
  void* ptr = malloc(size);
  if (Athena_test::allocs) {
    Athena_test::LeakCheckDisable disable;
    disable.insert(ptr);
  }
  return ptr;
}

void* operator new(std::size_t size){
  return newImpl(size);
}
#if __GNUC__ == 11
[[gnu::noinline]]
#endif
void deleteImpl (void* ptr) noexcept
{
  if (Athena_test::allocs) {
    Athena_test::LeakCheckDisable disable;
    disable.erase(ptr);
  }
  free(ptr);
}

void operator delete (void* ptr) noexcept
{
  deleteImpl(ptr);
}

void operator delete (void* ptr, size_t) noexcept
{
  deleteImpl(ptr);
}

namespace Athena_test {


struct Leakcheck
{
  Leakcheck() : m_old_allocs (allocs) { allocs = &m_allocs; }
  ~Leakcheck();
  std::unordered_set<void*>* m_old_allocs;
  std::unordered_set<void*> m_allocs;
};


// Not inline; this file should NOT be included in any library.
Leakcheck::~Leakcheck()
{
  allocs = m_old_allocs;
  if (!m_allocs.empty()) {
    std::cerr << "Leaks!\n";
    for (void* p : m_allocs)
      std::cerr << "  " << p << "\n";
    assert (m_allocs.empty());
  }
}


} // namespace Athena_test


#endif // not TESTTOOLS_LEAKCHECK_H
