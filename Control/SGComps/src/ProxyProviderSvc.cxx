/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>
#include <string>
#include <vector>

#include "SGTools/DataProxy.h"
#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/IProxyRegistry.h"
#include "AthenaKernel/EventContextClid.h"
#include "AthenaKernel/BaseInfo.h"

#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/ListItem.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ServiceHandle.h"

#include "ProxyProviderSvc.h"

#include "boost/range/adaptor/reversed.hpp"
#include "TClass.h"

using namespace std;


namespace {


const SG::BaseInfoBase* getBaseInfo (CLID clid)
{
  const SG::BaseInfoBase* bi = SG::BaseInfoBase::find (clid);
  if (bi) return bi;

  // Try to force a dictionary load to get it defined.
  ServiceHandle<IClassIDSvc> clidsvc ("ClassIDSvc", "ProxyProviderSvc");
  if (!clidsvc.retrieve()) return nullptr;
  std::string name;
  if (!clidsvc->getTypeNameOfID (clid, name).isSuccess()) {
    return nullptr;
  }
  (void)TClass::GetClass (name.c_str());
  return SG::BaseInfoBase::find (clid);
}


} // anonyous namespace

ProxyProviderSvc::ProxyProviderSvc(const std::string& name, 
                                   ISvcLocator* svcLoc): 
  base_class(name, svcLoc){
  m_providerNames.declareUpdateHandler(&ProxyProviderSvc::providerNamesPropertyHandler, this);
}

ProxyProviderSvc::~ProxyProviderSvc() {}

StatusCode 
ProxyProviderSvc::initialize() 
{
  ATH_MSG_VERBOSE( "Initializing " << name() );

  const bool CREATEIF(true);
  // cache pointer to Persistency Service
  if (!(service("EventPersistencySvc", m_pDataLoader, CREATEIF)).isSuccess()) {
    m_pDataLoader = 0;
    ATH_MSG_ERROR("Could not	 get pointer to Persistency Service");
    return StatusCode::FAILURE;
  } else {
#ifdef DEBUGPPS
    ATH_MSG_VERBOSE("Got pointer to Persistency Service " << m_pDataLoader);
#endif
  }

  //get properties set;	
  if(!(AthService::initialize()).isSuccess()) {
    return StatusCode::FAILURE;
  }

  // Take care of any pending preLoadProxies requests.
  for (IProxyRegistry* reg : m_pendingLoad) {
    CHECK( doPreLoadProxies (*reg) );
  }
  m_pendingLoad.clear();

  return StatusCode::SUCCESS;
}


namespace {


/// Set the provider on any new TADs.
void setProviderOnList (ProxyProviderSvc::TAdList& tList,
                        IAddressProvider* provider,
                        StoreID::type storeID)
{
  for (SG::TransientAddress* tad : boost::adaptors::reverse (tList)) {
    if (tad->provider() != nullptr) break;
    tad->setProvider (provider, storeID);
  }
}


} // anonymous namespace


StatusCode 
ProxyProviderSvc::doPreLoadProxies(IProxyRegistry& store)
{
  if (m_providers.empty()) return StatusCode::SUCCESS;

  StoreID::type storeID = store.storeID();
  TAdList tList;
  for (IAddressProvider* provider : m_providers) {
    ATH_CHECK( provider->preLoadAddresses(storeID, tList) );
    setProviderOnList (tList, provider, storeID);
  }
  ATH_CHECK( addAddresses (store, tList) );
  return StatusCode::SUCCESS;
}


///IProxyProvider interface
/// add proxies (before Begin Event)
StatusCode 
ProxyProviderSvc::preLoadProxies(IProxyRegistry& store)
{
  // Due to initialization loops, it's possible for this to be called
  // before the service is fully initialized.  In that case, we may
  // skip calling some of the providers.  So we haven't been fully initialized,
  // don't do anything now; rather, remember the store, and call
  // preLoadProxies again for it at the end of initialize().
  if (FSMState() == Gaudi::StateMachine::OFFLINE) {
    m_pendingLoad.push_back (&store);
    return StatusCode::SUCCESS;
  }
  return doPreLoadProxies (store);
}


///IProxyProvider interface
///add proxies to the store to modify
StatusCode 
ProxyProviderSvc::loadProxies(IProxyRegistry& store)
{
  if (m_providers.empty()) return StatusCode::SUCCESS;

  StoreID::type storeID = store.storeID();
  TAdList tList;
  for (IAddressProvider* provider : m_providers) {
    ATH_CHECK( provider->loadAddresses(storeID, tList) );
    setProviderOnList (tList, provider, storeID);
  }
  ATH_CHECK( addAddresses (store, tList) );
  return StatusCode::SUCCESS;
}


/**
 * @brief Add lists of TADs to the store.
 * @param store Store to which to add.
 * @param tList List of TADs from all providers.
 */
StatusCode ProxyProviderSvc::addAddresses(IProxyRegistry& store, 
					  TAdList& tList)
{
  for (SG::TransientAddress* tad : tList) {
    SG::DataProxy* proxy = store.proxy_exact(tad->clID(), tad->name());
    /// if proxy exists, simply update the proxy with new TransientAddress, 
    /// else construct a new proxy
    if (0 != proxy) 
    {
      proxy->setAddress(tad->address());
      if (proxy->provider() == 0) {
        proxy->setProvider(tad->provider(), store.storeID());
      }
    }
    else {
      if ( 0 == addAddress(store, std::move(*tad)) ) return StatusCode::FAILURE;
    }
    delete tad;
  }
  
  return StatusCode::SUCCESS;
}


///create a new Proxy, overriding CLID and/or key
SG::DataProxy*
ProxyProviderSvc::addAddress(IProxyRegistry& store, 
			     SG::TransientAddress&& tAddr) 
{  
  //HACK! The proxies for all object those key starts with "HLTAutoKey" will be deleted at the end of each event (resetOnly=false)
  //hence avoiding the proxy explosion observed with trigger object for releases <= 14.1.0
  bool resetOnly(tAddr.name().substr(0,10) != std::string("HLTAutoKey"));
  // std::cout << "PPS:addAdress: proxy for key " << tAddr->name() << " has resetOnly " << resetOnly << std::endl;
  SG::DataProxy* dp = new SG::DataProxy(std::move(tAddr),
                                        m_pDataLoader, true, resetOnly );

  // Must add the primary CLID first.
  bool addedProxy = store.addToStore(dp->clID(), dp).isSuccess();
  //  ATH_MSG_VERBOSE("created proxy for " << tAddr->clID() << "/" << tAddr->name() << "using " << m_pDataLoader->repSvcType());
  if (!addedProxy) {
    ATH_MSG_ERROR ("Failed to add proxy to store "
                   << dp->clID() << "/" << dp->name());
    delete dp;
    dp = nullptr;
  }
  else {
    // loop over all the transient CLIDs:
    SG::TransientAddress::TransientClidSet tClid = dp->transientID();
    for (CLID clid : tClid) {
      if (clid != dp->clID()) {
        bool flag = (store.addToStore(clid, dp)).isSuccess();
        if (!flag) {
          ATH_MSG_ERROR ("Failed to add proxy to store for symlink "
                         << clid << "/" << dp->name());
        }
        addedProxy &= flag;
      }
    }

    // loop over all alias'
    for (const std::string& alias : dp->alias()) {
      (store.addAlias(alias, dp)).ignore();
    }
    
    // Add any other allowable conversions.
    const SG::BaseInfoBase* bi = getBaseInfo (dp->clID());
    if (bi) {
      for (CLID clid : bi->get_bases()) {
        if (std::find (tClid.begin(), tClid.end(), clid) == tClid.end()) {
	  store.addToStore (clid, dp).ignore();
          dp->setTransientID (clid);
        }
      }

      for (CLID clid : bi->get_copy_conversions()) {
        if (std::find (tClid.begin(), tClid.end(), clid) == tClid.end()) {
	  store.addToStore (clid, dp).ignore();
          dp->setTransientID (clid);
        }
      }
    }
  }

  return dp;
}

/// Use a provider to create a proxy for ID/KEY.
/// If successful, the new proxy will be added to DATASTORE
/// and returned; otherwise, return null.
SG::DataProxy* 
ProxyProviderSvc::retrieveProxy(const CLID& id, const std::string& key,
				IProxyRegistry& store)
{
  if ( !m_providers.empty() ) {
    const EventContext& ctx = contextFromStore (store);
    SG::TransientAddress pTAd (id, key);
    pAPiterator iProvider(m_providers.begin()), iEnd(m_providers.end());
    for (; iProvider != iEnd; ++iProvider) {
      if ( ((*iProvider)->updateAddress(store.storeID(),&pTAd,ctx)).isSuccess() ) 
	{
	  pTAd.setProvider(*iProvider, store.storeID());
	  return this->addAddress(store,std::move(pTAd));
	}
    }  
  }  
  
  return nullptr;
}

/**
 * @brief Retrieve the EventContext saved in store DS.
 * @param ds The store from which to retrieve the context.
 *
 * If there is no context recorded in the store, return a default-initialized
 * context.
 */
const EventContext& ProxyProviderSvc::contextFromStore (IProxyRegistry& ds) const
{
  SG::DataProxy* proxy = ds.proxy (ClassID_traits<EventContext>::ID(),
                                   "EventContext");
  if (proxy) {
    EventContext* ctx = SG::DataProxy_cast<EventContext> (proxy);
    if (ctx) return *ctx;
  }
  static const EventContext emptyContext;
  return emptyContext;
}


void 
ProxyProviderSvc::providerNamesPropertyHandler( Gaudi::Details::PropertyBase& /*theProp*/ ) {
  //add declared providers to the list;
  std::vector<std::string> providerNames = m_providerNames.value();

  // FIXME: AddressRemappingSvc needs to come at the end, if it's provided.
  auto it = std::find (providerNames.begin(), providerNames.end(),
                       "AddressRemappingSvc");
  if (it != providerNames.end() &&  it != providerNames.end()-1) {
    providerNames.erase (it);
    providerNames.push_back ("AddressRemappingSvc");
  }

  for (const std::string& pName : providerNames) {
    IService *pIS(0);
    IAddressProvider *pAP(0);
    ListItem tn(pName);
    if (!(service(tn.type(), tn.name(), pIS)).isSuccess() ||
	0 == (pAP = dynamic_cast<IAddressProvider*>(pIS))) {
      ATH_MSG_ERROR(" getting Address Provider "<< pName);
      throw GaudiException("Failed to locate address provider",
			   "ProxyProviderSvc::providerNamesPropertyHandle", 
			   StatusCode::FAILURE);

    } else {
      ATH_MSG_DEBUG(" added Address Provider "<< pIS->name());
    }
    ProxyProviderSvc::addProvider(pAP);
  }

}

