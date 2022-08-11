// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESS_TSTORE_H
#define XAODROOTACCESS_TSTORE_H

// System include(s):
#include <string>
#include <map>
#include <memory>
#include <vector>

// ROOT include(s):
#include <Rtypes.h>

// EDM include(s):
#include "AthContainers/ConstDataVector.h"
#include "CxxUtils/sgkey_t.h"

// Local include(s):
#include "AsgMessaging/StatusCode.h"

namespace xAOD {

   // Forward declaration(s):
   class THolder;
   class TEvent;

   /// A relatively simple transient store for objects created in analysis
   ///
   /// This is a very simple transient store for objects that are created
   /// during analysis, but don't have to be written to the output file(s).
   /// To make it easier for the analysis tools to communicate with each other
   /// similar to how they would do it in Athena (with StoreGateSvc), they can
   /// use this class.
   ///
   /// The usage of this class is highly optional in analysis, it should only
   /// be used if really necessary. (Passing around objects in analysis code
   /// directly is usually a better approach than using a store in my mind...)
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TStore {

      /// Make TEvent a friend of this class
      friend class TEvent;

   public:
      /// Default constructor
      TStore();
      /// Disallow copying the object
      TStore( const TStore& ) = delete;
      /// Destructor
      virtual ~TStore();

      /// Disallow copying the object
      TStore& operator=( const TStore& ) = delete;

      /// Set this as the active transient store in the application
      void setActive();

      /// @name Transient data accessor/modifier functions
      /// @{

      /// Function checking if an object is available from the store
      template< typename T >
      ::Bool_t contains( const std::string& key ) const;

      /// Function checking if an object with a given type is constant
      template< typename T >
      ::Bool_t isConst( const std::string& key ) const;

      /// Retrieve either a constant or non-constant object from the store
      template< typename T >
      StatusCode retrieve( const T*& obj, const std::string& key ) const;
      /// Retrieve a non-constant object from the store
      template< typename T >
      StatusCode retrieve( T*& obj, const std::string& key ) const;

      /// Add an object to the store
      template< typename T >
      StatusCode record( T* obj, const std::string& key );
      /// Add an object othe store, explicitly taking ownership of it
      template< typename T >
      StatusCode record( std::unique_ptr< T > obj, const std::string& key );

      /// prodive a list of keys associated with a type
      template< typename T >
      void keys( std::vector< std::string >& vkeys ) const;

      /// Remove an object from the store by name
      StatusCode remove( const std::string& key );
      /// Remove an object from the store by pointer
      StatusCode remove( void* ptr );

      /// Clear the store of all of its contents
      void clear();

      /// @}

      /// Print the current contents of the transient store
      void print() const;

   protected:
      /// Non-templated function implementing the containment check
      ::Bool_t contains( const std::string& key,
                         const std::type_info& ti ) const;
      /// Non-templated function implementing the const-ness check
      ::Bool_t isConst( const std::string& key,
                        const std::type_info& ti ) const;
      /// Function retrieving a non-const object in a non-template way
      void* getObject( const std::string& key, const std::type_info& ti ) const;
      /// Function retrieving a const object in a non-template way
      const void* getConstObject( const std::string& key,
                                  const std::type_info& ti ) const;
      /// Function recording an object that has a dictionary available
      StatusCode record( void* obj, const std::string& key,
                          const std::string& classname,
                          ::Bool_t isOwner = kTRUE );
      /// Function recording an object that has no dictionary
      StatusCode record( void* obj, const std::string& key,
                          const std::type_info& ti );
      /// Function doing the first step of recording a ConstDataVector object
      template< class T >
      StatusCode record( ConstDataVector< T >* obj, const std::string& key,
                          const std::type_info& ti );
      /// Function doing the second step of recording a ConstDataVector object
      StatusCode record( THolder* hldr, const std::string& key );

      /// @name Functions mostly used by TEvent in the TVirtualEvent functions
      /// @{

      /// Check if an object with a given hash is managed by the store
      ::Bool_t contains( SG::sgkey_t hash ) const;
      /// Check if an object with a given pointer is managed by the store
      ::Bool_t contains( const void* ptr ) const;
      /// Get the name corresponding to a hashed key
      const std::string& getName( SG::sgkey_t hash ) const;
      /// Get the name of a managed object
      const std::string& getName( const void* ptr ) const;
      /// Function determining the list keys associated with a type name
      void getNames(const std::string& targetClassName,
                    std::vector<std::string>& vkeys ) const;

      /// @}

      /// Type of the internal container storing all the objects
      typedef std::map< std::string, THolder* > Objects_t;
      /// Type of the internal storage for the hashed keys of the object names
      typedef SG::SGKeyMap< std::string > HashedKeys_t;

      /// The object storage
      Objects_t m_objects;
      /// The key map
      HashedKeys_t m_keys;

   }; // class TStore

} // namespace xAOD

// Include the template implementation.
#include "TStore.icc"

#endif // XAODROOTACCESS_TSTORE_H
