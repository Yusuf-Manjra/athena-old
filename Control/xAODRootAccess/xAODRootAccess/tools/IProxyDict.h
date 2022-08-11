// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESS_TOOLS_IPROXYDICT_H
#define XAODROOTACCESS_TOOLS_IPROXYDICT_H

#ifdef XAOD_STANDALONE

// System include(s):
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

/// The CLID typedef is taken from xAODCore
#include "xAODCore/CLASS_DEF.h"
#include "CxxUtils/sgkey_t.h"

namespace SG {

   /// Simple emulation of SG::DataProxy
   struct DataProxy {};

   /// Let's use a built-in type
   template< class T >
   using DataObjectSharedPtr = std::shared_ptr< T >;

} // namespace SG

/// Dummy implementation for DataObject
struct DataObject {};
/// Dummy implementation for InterfaceID
struct InterfaceID {};

/// Emulation of the offline IProxyDict interface
///
/// In order to simplify the implementation of xAOD::TEvent a bit, we create
/// a simple emulation of this offline class.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class IProxyDict {

public:
   /// Virtual destructor
   virtual ~IProxyDict() {}

   /// get proxy for a given data object address in memory
   virtual SG::DataProxy* proxy( const void* const pTransient ) const = 0;

   /// get proxy with given id and key. Returns 0 to flag failure
   virtual SG::DataProxy* proxy( const CLID& id,
                                 const std::string& key ) const = 0;

   /// Get proxy given a hashed key+clid.
   virtual SG::DataProxy* proxy_exact( SG::sgkey_t sgkey ) const = 0;

   /// Add a new proxy to the store.
   virtual StatusCode addToStore( CLID id, SG::DataProxy* proxy ) = 0;

   /// return the list of all current proxies in store
   virtual std::vector< const SG::DataProxy* > proxies() const = 0;

   /// Find the string corresponding to a given key.
   virtual SG::sgkey_t stringToKey( const std::string& str, CLID clid ) = 0;

   /// Find the string corresponding to a given key.
   virtual const std::string* keyToString( SG::sgkey_t key ) const = 0;

   /// Find the string and CLID corresponding to a given key.
   virtual const std::string* keyToString( SG::sgkey_t key,
                                           CLID& clid ) const = 0;

   /// Remember an additional mapping from key to string/CLID.
   virtual void registerKey( SG::sgkey_t key,
                             const std::string& str,
                             CLID clid ) = 0;

   /// Record an object in the store
   virtual SG::DataProxy*
   recordObject( SG::DataObjectSharedPtr< DataObject > obj,
                 const std::string& key,
                 bool allowMods,
                 bool returnExisting ) = 0;

   /// Increment the reference count of Interface instance
   virtual unsigned long addRef() = 0;

   /// Release Interface instance
   virtual long unsigned int release() = 0;

   /// Get the name of the instance
   virtual const std::string& name() const = 0;

   /// Set the void** to the pointer to the requested interface of the instance
   virtual StatusCode queryInterface( const InterfaceID&, void** ) = 0;

}; // class IProxyDict

#else

// If we're in an offline build, just take the declaration from AthenaKernel.
#include "AthenaKernel/IProxyDict.h"

#endif // XAOD_STANDALONE
#endif // XAODROOTACCESS_TOOLS_IPROXYDICT_H
