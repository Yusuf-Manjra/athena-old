/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGMESSAGING_MSGSTREAM_H
#define ASGMESSAGING_MSGSTREAM_H

// System include(s):
#include <string>
#include <sstream>

// Local include(s):
#include "AsgMessaging/MsgLevel.h"
#include "AsgMessaging/INamedInterface.h"

#ifndef XAOD_STANDALONE
#   include "GaudiKernel/MsgStream.h"
#else // not XAOD_STANDALONE

/// A replacement of Gaudi's MsgStream class for ROOT analysis
///
/// The stream is constructed from a named object and uses
/// that name to prefix messages.
///
/// @author David Adams <dladams@bnl.gov>
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
/// $Revision: 803209 $
/// $Date: 2017-04-19 23:18:32 +0200 (Wed, 19 Apr 2017) $
///
class MsgStream : public std::ostringstream {

public:
   /// Constructor with parent tool pointer
   MsgStream( const INamedInterface* tool = 0 );
   /// Constructor with a name
   MsgStream( const std::string& name );
   /// Constructor resembling that of Gaudi's MsgStream
   MsgStream( void* msgSvc, const std::string& name, int buffer_length = 128 );

   /// @name Stream filling operators
   /// @{

   /// Operator accepting MsgStream stream modifiers
   MsgStream& operator<< ( MsgStream& ( *_f )( MsgStream& ) );
   /// Operator accepting std::ostream stream modifiers
   MsgStream& operator<< ( std::ostream& ( *_f )( std::ostream& ) );
   /// Operator accepting std::ios stream modifiers
   MsgStream& operator<< ( std::ios& ( *_f )( std::ios& ) );

   /// Operator accepting message level setting
   MsgStream& operator<< ( MSG::Level lvl );

   /// Operator accepting basically any kind of argument
   template< class T >
   MsgStream& operator<< ( const T& arg ) {
      ( * ( std::ostringstream* ) this ) << arg;
      return *this;
   }

   /// @}

   /// Function printing the current payload of the stream
   MsgStream& doOutput();

   /// Function setting the width of the message source printout
   static void setSourceWidth( size_t width );

   /// @name Message level manipulating functions
   /// @{

   /// Set the message level
   void setLevel( MSG::Level lvl );

   /// Check the message level
   bool msgLevel( MSG::Level lvl ) const;

   /// Get the current minimum output level of the stream
   MSG::Level level() const;

   /// declare the message level as property on the parent
   template<typename T> void
   declarePropertyFor (T& parent);

   /// Retrieve current stream output level
   MSG::Level currentLevel() const;

   /// @}

   /// The the name of the source of the messages
   const std::string& name() const;
   /// Set the name of the source of the messages
   void setName( const std::string& name );

private:
   /// Pointer to the tool printing messages
   const INamedInterface* m_tool;
   /// Message source name
   std::string m_name;
   /// Minimum level for the printed messages (this is an integer, so
   /// it can serve as a property)
   int m_lvl;
   /// The level of the message currently being assembled
   MSG::Level m_reqlvl;

}; // class MsgStream

/// MsgStream Modifier: endmsg. Calls the output method of the MsgStream
inline MsgStream& endmsg( MsgStream& s ) {
   return s.doOutput();
}

template<typename T> void MsgStream ::
declarePropertyFor (T& parent)
{
  parent.declareProperty ("OutputLevel", m_lvl, "output message level");
}
/// Backwards compatibility definition
#define endreq endmsg

#endif // not XAOD_STANDALONE
#endif // ASGMESSAGING_MSGSTREAM_H
