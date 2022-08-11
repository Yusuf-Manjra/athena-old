/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file SGComps/test/ProxyProviderSvc_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2016
 * @brief Regression tests for ProxyProviderSvc
 */

// FIXME: Very incomplete!

#undef NDEBUG
#include "SGTools/TransientAddress.h"
#include "SGTools/DataProxy.h"
#include "AthenaKernel/IProxyProviderSvc.h"
#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/IProxyRegistry.h"
#include "AthenaKernel/errorcheck.h"
#include "TestTools/initGaudi.h"
#include "GaudiKernel/ServiceHandle.h"
#include <vector>
#include <iostream>
#include <cassert>


class TestProvider
  : public IAddressProvider
{
public:
  virtual unsigned long addRef() override { std::abort(); }
  virtual unsigned long release() override { std::abort(); }
  virtual StatusCode queryInterface(const InterfaceID &/*ti*/, void** /*pp*/) override
  { std::abort(); }

  virtual StatusCode preLoadAddresses(StoreID::type storeID,
				      tadList& list)  override;

  virtual StatusCode loadAddresses(StoreID::type storeID,
				   tadList& list) override;

  virtual StatusCode updateAddress(StoreID::type storeID,
				   SG::TransientAddress* pTAd,
                                   const EventContext& ctx) override;


  void add (CLID clid,
            const std::string& name,
            const std::vector<CLID>& tclids = {});


public:
  int tListLen = -1;

private:
  std::vector<SG::TransientAddress> m_tads;
};


StatusCode TestProvider::preLoadAddresses(StoreID::type /*storeID*/,
                                          tadList& list)
{
  tListLen = list.size();
  for (const SG::TransientAddress& tad : m_tads) {
    list.push_back (new SG::TransientAddress (tad.clID(), tad.name()));
    for (CLID clid : tad.transientID()) {
      list.back()->setTransientID (clid);
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode TestProvider::loadAddresses(StoreID::type /*storeID*/,
                                       tadList& list)
{
  tListLen = list.size();
  for (const SG::TransientAddress& tad : m_tads) {
    list.push_back (new SG::TransientAddress (tad.clID(), tad.name()));
    for (CLID clid : tad.transientID()) {
      list.back()->setTransientID (clid);
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode TestProvider::updateAddress(StoreID::type /*storeID*/,
                                       SG::TransientAddress* /*pTAd*/,
                                       const EventContext& /*ctx*/)
{
  std::abort();
}


void TestProvider::add (CLID clid, const std::string& name,
                        const std::vector<CLID>& tclids)
{
  m_tads.emplace_back (clid, name);
  for (CLID tclid : tclids) {
    m_tads.back().setTransientID (tclid);
  }
}


class TestProxyRegistry
  : public IProxyRegistry
{
public:
  virtual ~TestProxyRegistry();

  virtual StatusCode addToStore(const CLID& id, SG::DataProxy* proxy) override;

  virtual StatusCode addAlias(const std::string& /*key*/, SG::DataProxy* /*proxy*/) override
  { std::abort(); }

  virtual StoreID::type storeID() const override
  { return StoreID::EVENT_STORE; }

  virtual SG::DataProxy* proxy(const SG::TransientAddress* /*tAD*/) const override
  { return nullptr; }

  virtual SG::DataProxy* proxy(const CLID& /*id*/, const std::string& /*key*/) const override
  { return nullptr; }

  virtual SG::DataProxy* proxy_exact(const CLID& /*id*/,
                                     const std::string& /*key*/) const override
  { return nullptr; }

  std::vector<SG::DataProxy*> proxies;
};


TestProxyRegistry::~TestProxyRegistry()
{
  for (SG::DataProxy* p : proxies) {
    p->release();
  }
}


StatusCode
TestProxyRegistry::addToStore (const CLID& id, SG::DataProxy* proxy)
{
  // First entry should be primary.
  if (proxies.empty() || proxies.back() != proxy) {
    assert (id == proxy->clID());
  }
  proxy->addRef();
  proxies.push_back (proxy);
  return StatusCode::SUCCESS;
}


static const int NPROVIDERS = 4;


void checkStore (const TestProxyRegistry& store, TestProvider providers[NPROVIDERS])
{
  typedef SG::DataProxy::CLIDCont_t CLIDCont_t;

  assert (store.proxies.size() == 5);
  assert (providers[0].tListLen == 0);
  assert (providers[1].tListLen == 0);
  assert (providers[2].tListLen == 2);
  assert (providers[3].tListLen == 2);

  assert (store.proxies[0]->clID() == 10);
  assert (store.proxies[0]->name() == "x1");
  assert (store.proxies[0]->provider() == &providers[1]);
  assert (store.proxies[0]->transientID() == CLIDCont_t{10});

  assert (store.proxies[1]->clID() == 11);
  assert (store.proxies[1]->name() == "x2");
  assert (store.proxies[1]->provider() == &providers[1]);
  assert (store.proxies[1]->transientID() == CLIDCont_t{11});

  assert (store.proxies[2]->clID() == 12);
  assert (store.proxies[2]->name() == "y1");
  assert (store.proxies[2]->provider() == &providers[3]);
  assert (store.proxies[2]->transientID() == (CLIDCont_t{5, 12, 20}));

  assert (store.proxies[3] == store.proxies[2]);
  assert (store.proxies[4] == store.proxies[2]);
}


void test1 (IProxyProviderSvc& svc)
{
  std::cout << "test1\n";

  TestProvider providers[NPROVIDERS];

  providers[1].add (10, "x1");
  providers[1].add (11, "x2");
  providers[3].add (12, "y1", {5, 20});

  for (TestProvider& p : providers)
    svc.addProvider (&p);

  TestProxyRegistry store;
  
  assert (svc.preLoadProxies (store).isSuccess());
  checkStore (store, providers);

  for (TestProvider& p : providers)
    p.tListLen = -1;
  store.proxies.clear();

  assert (svc.loadProxies (store).isSuccess());
  checkStore (store, providers);
}


int main()
{
  errorcheck::ReportMessage::hideErrorLocus();
  ISvcLocator* svcloc = 0;
  if (!Athena_test::initGaudi("ProxyProviderSvc_test.txt", svcloc)) {
    std::cerr << "This test can not be run" << std::endl;
    return 1;
  }  
  assert(svcloc);

  ServiceHandle<IProxyProviderSvc> svc ("ProxyProviderSvc", "test");
  if (svc.retrieve().isFailure()) {
    std::cerr << "Can't retrieve service." << std::endl;
    return 1;
  }

  test1 (*svc);
  return 0;
}
