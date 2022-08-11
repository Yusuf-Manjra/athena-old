/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGTOOLS_TPROPERTY_H
#define ASGTOOLS_TPROPERTY_H

// System include(s):
#include <string>
#include <vector>

// Local include(s):
#include "AsgTools/ToolHandle.h"
#include "AsgTools/ToolHandleArray.h"
#include "AsgTools/Property.h"

/// Templated wrapper around user properties
///
/// This class is used to wrap user-defined tool properties, to be
/// able to set them through the PropertyMgr.
///
/// @author David Adams <dladams@bnl.gov>
///
/// $Revision: 802972 $
/// $Date: 2017-04-15 20:13:17 +0200 (Sat, 15 Apr 2017) $
///
template< typename T >
class TProperty : public Property {

public:
   /// Constructor from a reference and a type
   TProperty( T& val, Type type );

   /// Return the address of the property variable
   const T* pointer() const;

   /// Set value using that from another property
   virtual int setFrom( const Property& rhs );

   virtual StatusCode getString (std::string& result) const;

   virtual StatusCode getCastString (std::string& result) const;

   virtual StatusCode setString (const std::string& value);

private:
   /// The address of the wrapped variable
   T* m_ptr;

}; // class TProperty

/// @name Declare specialization(s) for TProperty::setFrom
/// @{

/// Special handling for strings to interpret the user supplied string
template<>
int TProperty< std::string >::setFrom( const Property& rhs );
/// Special handling for strings to get casts right

/// @}

/// @name Property creator helper function(s)
/// @{

/// Create a boolean propert
Property* createProperty( const bool& rval );

/// Create an integer property
Property* createProperty( const int& rval );

/// Create a float property
Property* createProperty( const float& rval );

/// Create a double property
Property* createProperty( const double& rval );

/// Create a string property
Property* createProperty( const std::string& rval );

/// Create an integer vector property
Property* createProperty( const std::vector< int >& rval );

/// Create a floating point vector property
Property* createProperty( const std::vector< float >& rval );

/// Create a string vector property
Property* createProperty( const std::vector< std::string >& rval );

/// Create a property for a default, unknown type
template< typename T >
Property* createProperty( const T& rval );

/// @}

// Include the template implementation:
#include "AsgTools/TProperty.icc"

#endif // ASGTOOLS_TPROPERTY_H
