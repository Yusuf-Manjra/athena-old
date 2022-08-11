/*
 * Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
 */
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/SurfaceHolders.h"
#include <iostream>

// Dummy classes that would contain a ptr to Surface that
// need special management

/// contains ptr to Surface
/// which is managed based on if the surface is free
/// or not
class ContainsPtrSurface : public Trk::SurfacePtrHolder
{
public:
  ContainsPtrSurface(const Trk::Surface& s)
    : Trk::SurfacePtrHolder(s)
  {}
  ContainsPtrSurface(const Trk::Surface* s)
    : Trk::SurfacePtrHolder(s)
  {}
  ContainsPtrSurface() = default;
  ContainsPtrSurface(const ContainsPtrSurface& other) = default;
  ContainsPtrSurface(ContainsPtrSurface&& other) noexcept = default;
  ContainsPtrSurface& operator=(const ContainsPtrSurface& other) = default;
  ContainsPtrSurface& operator=(ContainsPtrSurface&&) = default;
  ~ContainsPtrSurface() = default;
};

/// contains SurfaceUniquePtr
/// which is managed based on if the surface is free
/// or not
class ContainsUniqSurface : public Trk::SurfaceUniqHolder
{
public:
  ContainsUniqSurface(const Trk::Surface& s)
    : Trk::SurfaceUniqHolder(s)
  {}
  ContainsUniqSurface(const Trk::Surface* s)
    : Trk::SurfaceUniqHolder(s)
  {}
  ContainsUniqSurface() = default;
  ContainsUniqSurface(const ContainsUniqSurface& other) = default;
  ContainsUniqSurface(ContainsUniqSurface&& other) noexcept = default;
  ContainsUniqSurface& operator=(const ContainsUniqSurface& other) = default;
  ContainsUniqSurface& operator=(ContainsUniqSurface&&) = default;
  ~ContainsUniqSurface() = default;
};

/// contains ptr to Surface
/// which is managed based on if the surface
/// has associated detector element
class ContainsPtrSurfaceDetEl : public Trk::SurfacePtrHolderDetEl
{
public:
  ContainsPtrSurfaceDetEl(const Trk::Surface& s)
    : Trk::SurfacePtrHolderDetEl(s)
  {}
  ContainsPtrSurfaceDetEl(const Trk::Surface* s)
    : Trk::SurfacePtrHolderDetEl(s)
  {}
  ContainsPtrSurfaceDetEl() = default;
  ContainsPtrSurfaceDetEl(const ContainsPtrSurfaceDetEl& other) = default;
  ContainsPtrSurfaceDetEl(ContainsPtrSurfaceDetEl&& other) noexcept = default;
  ContainsPtrSurfaceDetEl& operator=(const ContainsPtrSurfaceDetEl& other) =
    default;
  ContainsPtrSurfaceDetEl& operator=(ContainsPtrSurfaceDetEl&&) = default;
  ~ContainsPtrSurfaceDetEl() = default;
};

/*
 * Test Tracking Geometry Owned Surface
 */
void
testPtr1()
{
  std::cout << "testPtr1" << '\n';
  auto owner = std::make_unique<Trk::PerigeeSurface>();
  owner->setOwner(Trk::TGOwn);
  // construct from reference
  ContainsPtrSurface test1(*owner);
  std::cout << (test1.surfacePtr() == owner.get()) << '\n';
  // construct from ptr
  ContainsPtrSurface test11(test1.release());
  std::cout << (test11.surfacePtr() == owner.get()) << '\n';
  std::cout << (test1.surfacePtr() == nullptr) << '\n';
  // copy ctor we still point to the same
  ContainsPtrSurface test2 = test11;
  std::cout << (test2.surfacePtr() == owner.get()) << '\n';
  // copy assignement  we still point to the same
  ContainsPtrSurface test3;
  test3 = test2;
  std::cout << (test2.surfacePtr() == owner.get()) << '\n';
  // Move ctor invalidates the moved from
  ContainsPtrSurface test4(std::move(test3));
  std::cout << (test4.surfacePtr() == owner.get()) << '\n';
  std::cout << (test3.surfacePtr() == nullptr) << '\n';
  // Move assignment invalidates the moved from
  ContainsPtrSurface test5(std::move(test4));
  std::cout << (test5.surfacePtr() == owner.get()) << '\n';
  std::cout << (test4.surfacePtr() == nullptr) << '\n';
  // destroy also 1,2,5
  test11.destroySurface();
  test2.destroySurface();
  test5.destroySurface();
  std::cout << (test11.surfacePtr() == nullptr) << '\n';
  std::cout << (test2.surfacePtr() == nullptr) << '\n';
  std::cout << (test5.surfacePtr() == nullptr) << '\n';
  // The owner should be still be there
  std::cout << (owner.get() != nullptr) << '\n';
  owner.reset();
  // Now the owner also goes
  std::cout << (owner.get() == nullptr) << '\n';
}

/*
 * Test Free Surface
 */
void
testPtr2()
{
  std::cout << "testPtr2" << '\n';
  auto free = std::make_unique<Trk::PerigeeSurface>();
  free->setOwner(Trk::noOwn);
  // construct from reference we clone
  ContainsPtrSurface test1(*free);
  std::cout << (test1.surfacePtr() != free.get()) << '\n';
  // construct from ptr we release
  ContainsPtrSurface test11(test1.release());
  std::cout << (test11.surfacePtr() != free.get()) << '\n';
  std::cout << (test1.surfacePtr() == nullptr) << '\n';
  // copy ctor will clone
  ContainsPtrSurface test2 = test11;
  std::cout << (test2.surfacePtr() != free.get()) << '\n';
  // copy assignement  will clone
  ContainsPtrSurface test3;
  test3 = test2;
  std::cout << (test2.surfacePtr() != free.get()) << '\n';
  // Move ctor invalidates the moved from
  ContainsPtrSurface test4(std::move(test3));
  std::cout << (test4.surfacePtr() != free.get()) << '\n';
  std::cout << (test3.surfacePtr() == nullptr) << '\n';
  // Move assignment invalidates the moved from
  ContainsPtrSurface test5(std::move(test4));
  std::cout << (test5.surfacePtr() != free.get()) << '\n';
  std::cout << (test4.surfacePtr() == nullptr) << '\n';
  // destroy  also 1,2,5
  test11.destroySurface();
  test2.destroySurface();
  test5.destroySurface();
  std::cout << (test11.surfacePtr() == nullptr) << '\n';
  std::cout << (test2.surfacePtr() == nullptr) << '\n';
  std::cout << (test5.surfacePtr() == nullptr) << '\n';
  // The free should be still be there
  std::cout << (free.get() != nullptr) << '\n';
  free.reset();
  // Now the free also goes
  std::cout << (free.get() == nullptr) << '\n';
}

/*
 * Test Tracking Geometry owned surface
 * But 
 * there is no associated detector element
 * and we specify detElemBased policy.
 *
 * So behaves like "free" surface
 */
void
testPtr3()
{
  std::cout << "testPtr3" << '\n';
  auto free = std::make_unique<Trk::PerigeeSurface>();
  free->setOwner(Trk::TGOwn);
  // construct from reference we clone
  ContainsPtrSurfaceDetEl test1(*free);
  std::cout << (test1.surfacePtr() != free.get()) << '\n';
  // construct from ptr we release
  ContainsPtrSurfaceDetEl test11(test1.release());
  std::cout << (test11.surfacePtr() != free.get()) << '\n';
  std::cout << (test1.surfacePtr() == nullptr) << '\n';
  // copy ctor will clone
  ContainsPtrSurfaceDetEl test2 = test11;
  std::cout << (test2.surfacePtr() != free.get()) << '\n';
  // copy assignement  will clone
  ContainsPtrSurfaceDetEl test3;
  test3 = test2;
  std::cout << (test2.surfacePtr() != free.get()) << '\n';
  // Move ctor invalidates the moved from
  ContainsPtrSurfaceDetEl test4(std::move(test3));
  std::cout << (test4.surfacePtr() != free.get()) << '\n';
  std::cout << (test3.surfacePtr() == nullptr) << '\n';
  // Move assignment invalidates the moved from
  ContainsPtrSurfaceDetEl test5(std::move(test4));
  std::cout << (test5.surfacePtr() != free.get()) << '\n';
  std::cout << (test4.surfacePtr() == nullptr) << '\n';
  // destroy  also 1,2,5
  test11.destroySurface();
  test2.destroySurface();
  test5.destroySurface();
  std::cout << (test11.surfacePtr() == nullptr) << '\n';
  std::cout << (test2.surfacePtr() == nullptr) << '\n';
  std::cout << (test5.surfacePtr() == nullptr) << '\n';
  // The free should be still be there
  std::cout << (free.get() != nullptr) << '\n';
  free.reset();
  // Now the free also goes
  std::cout << (free.get() == nullptr) << '\n';
}

/*
 * Test Tracking Geometry Owned Surface
 */
void
testUniq1()
{
  std::cout << "testUniq1" << '\n';
  auto owner = std::make_unique<Trk::PerigeeSurface>();
  owner->setOwner(Trk::TGOwn);
  // construct from reference
  ContainsUniqSurface test1(*owner);
  std::cout << (test1.surfacePtr() == owner.get()) << '\n';
  // construct from ptr
  ContainsUniqSurface test11(test1.release());
  std::cout << (test11.surfacePtr() == owner.get()) << '\n';
  std::cout << (test1.surfacePtr() == nullptr) << '\n';
  // copy ctor we still point to the same
  ContainsUniqSurface test2 = test11;
  std::cout << (test2.surfacePtr() == owner.get()) << '\n';
  // copy assignement  we still point to the same
  ContainsUniqSurface test3;
  test3 = test2;
  std::cout << (test2.surfacePtr() == owner.get()) << '\n';
  // Move ctor invalidates the moved from
  ContainsUniqSurface test4(std::move(test3));
  std::cout << (test4.surfacePtr() == owner.get()) << '\n';
  std::cout << (test3.surfacePtr() == nullptr) << '\n';
  // Move assignment invalidates the moved from
  ContainsUniqSurface test5(std::move(test4));
  std::cout << (test5.surfacePtr() == owner.get()) << '\n';
  std::cout << (test4.surfacePtr() == nullptr) << '\n';
  // destroy  also 1,2,5
  test11.destroySurface();
  test2.destroySurface();
  test5.destroySurface();
  std::cout << (test11.surfacePtr() == nullptr) << '\n';
  std::cout << (test2.surfacePtr() == nullptr) << '\n';
  std::cout << (test5.surfacePtr() == nullptr) << '\n';
  // The owner should be still be there
  std::cout << (owner.get() != nullptr) << '\n';
  owner.reset();
  // Now the owner also goes
  std::cout << (owner.get() == nullptr) << '\n';
}

/*
 * Test Free Surface
 */
void
testUniq2()
{
  std::cout << "testPtr2" << '\n';
  auto free = std::make_unique<Trk::PerigeeSurface>();
  free->setOwner(Trk::noOwn);
  // construct from reference we clone
  ContainsUniqSurface test1(*free);
  std::cout << (test1.surfacePtr() != free.get()) << '\n';
  // construct from released ptr
  ContainsUniqSurface test11(test1.release());
  std::cout << (test11.surfacePtr() != free.get()) << '\n';
  std::cout << (test1.surfacePtr() == nullptr) << '\n';
  // copy ctor will clone
  ContainsUniqSurface test2 = test11;
  std::cout << (test2.surfacePtr() != free.get()) << '\n';
  // copy assignement  will clone
  ContainsUniqSurface test3;
  test3 = test2;
  std::cout << (test2.surfacePtr() != free.get()) << '\n';
  // Move ctor invalidates the moved from
  ContainsUniqSurface test4(std::move(test3));
  std::cout << (test4.surfacePtr() != free.get()) << '\n';
  std::cout << (test3.surfacePtr() == nullptr) << '\n';
  // Move assignment invalidates the moved from
  ContainsUniqSurface test5(std::move(test4));
  std::cout << (test5.surfacePtr() != free.get()) << '\n';
  std::cout << (test4.surfacePtr() == nullptr) << '\n';
  // destroy  also 1,2,5
  test11.destroySurface();
  test2.destroySurface();
  test5.destroySurface();
  std::cout << (test11.surfacePtr() == nullptr) << '\n';
  std::cout << (test2.surfacePtr() == nullptr) << '\n';
  std::cout << (test5.surfacePtr() == nullptr) << '\n';
  // The free should be still be there
  std::cout << (free.get() != nullptr) << '\n';
  free.reset();
  // Now the free also goes
  std::cout << (free.get() == nullptr) << '\n';
}

int
main()
{
  testPtr1();
  testPtr2();
  testPtr3();
  testUniq1();
  testUniq2();
  return 0;
}

