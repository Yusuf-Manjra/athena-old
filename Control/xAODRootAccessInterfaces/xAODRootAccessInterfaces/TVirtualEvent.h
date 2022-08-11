// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESSINTERFACES_TVIRTUALEVENT_H
#define XAODROOTACCESSINTERFACES_TVIRTUALEVENT_H

#include "CxxUtils/sgkey_t.h"

// System include(s):
extern "C" {
#   include <stdint.h>
}
#include <string>
#include <vector>

// Forward declaration(s):
namespace std {
   class type_info;
}

namespace xAOD {

   /// @short Base interface for getting objects out of the input file
   ///
   /// In order for the smart pointers not to depend on the full xAOD::TEvent
   /// class, this interface collects all the functionality that is needed
   /// by the smart pointers to access the input objects.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TVirtualEvent {

   public:
      using sgkey_t = SG::sgkey_t;

      /// Virtual destructor to make vtable happy
      virtual ~TVirtualEvent() = default;

      /// Key for retrieving the "default" object of a given type
      static constexpr sgkey_t DEFAULT_KEY = ~static_cast<sgkey_t>(0);
      /// Mask for the keys, used mostly internally
      static constexpr sgkey_t KEY_MASK = DEFAULT_KEY >> 2;

      /// Function retrieving an object from the event (constant version)
      template< typename T >
      bool retrieve( const T*& obj, sgkey_t key = DEFAULT_KEY,
                     bool silent = false );
      /// Function retrieving an object from the event (constant version)
      template< typename T >
      bool retrieve( const T*& obj, const std::string& key,
                     bool silent = false );

      /// provide list of all keys associated with provided type.
      /// usage: event->keys( vec_to_fill, metadata )
      /// @param vkeys will be filled with the list of keys (may be empty)
      /// @param metadata (default false) look in metadata content if true
      template< typename T >
      void keys( std::vector< std::string >& vkeys,
                 bool metadata = false ) const;

      /// Function returning the hash describing an object's name/key
      virtual sgkey_t getHash( const std::string& key ) const = 0;
      /// Function returning the hash describing a known object
      virtual sgkey_t getKey( const void* obj ) const = 0;
      /// Function returning the key describing a known object
      virtual const std::string& getName( const void* obj ) const = 0;
      /// Function returning the key describing a known object
      virtual const std::string& getName( sgkey_t hash ) const = 0;

   protected:
      /// Function for retrieving an output object in a non-template way
      virtual void* getOutputObject( sgkey_t key,
                                     const std::type_info& ti ) = 0;
      /// Function for retrieving an input object in a non-template way
      virtual const void* getInputObject( sgkey_t key,
                                          const std::type_info& ti,
                                          bool silent = false ) = 0;
      /// Function to retrieve list of keys describing a type name
      virtual void getNames( const std::string& targetClassName,
                             std::vector<std::string>& vkeys,
                             bool metadata) const = 0;

   }; // class TVirtualEvent

} // namespace xAOD

// Include the template implementation:
#include "TVirtualEvent.icc"

#endif // XAODROOTACCESSINTERFACES_TVIRTUALEVENT_H
