/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



// $Id: SystematicsTool.cxx 768505 2016-08-17 18:13:04Z ssnyder $
#include <iostream>

// Infrastructure include(s):
#include "RootCoreUtils/Assert.h"

// Local include(s):
#include "PATInterfaces/SystematicsTool.h"
#include "AsgMessaging/StatusCode.h"
#include "PATInterfaces/SystematicVariation.h"
#include "PATInterfaces/SystematicRegistry.h"

// Other include(s):

namespace CP {

   SystematicsTool::SystematicsTool()
       : m_appliedSystematics(0) {

      RCU_NEW_INVARIANT( this );
   }

   bool SystematicsTool::
   isAffectedBySystematic( const SystematicVariation& systematic ) const {

      RCU_READ_INVARIANT( this );
      return ( m_affectingSystematics.find( systematic ) !=
               m_affectingSystematics.end() );
   }

   SystematicSet
   SystematicsTool::affectingSystematics() const {

      RCU_READ_INVARIANT( this );
      return m_affectingSystematics;
   }

   SystematicSet
   SystematicsTool::recommendedSystematics() const {

      RCU_READ_INVARIANT( this );
      return m_recommendedSystematics;
   }

   const SystematicSet&
   SystematicsTool::appliedSystematics() const {

      RCU_READ_INVARIANT( this );
      if(m_appliedSystematics == 0){
         std::cerr << "SystematicsTool::appliedSystematics WARNING: "
                   << "no systematic configuration has been applied yet. "
                   << "Returning NULL" << std::endl;
         static const SystematicSet dummy;
         return dummy;
      }
      return *m_appliedSystematics;
   }

   std::string SystematicsTool::appliedSystematicsString () const {

      RCU_READ_INVARIANT( this );
      return m_appliedSystematics->name();
   }

   StatusCode SystematicsTool::
   applySystematicVariation( const SystematicSet& systConfig ) {

      RCU_CHANGE_INVARIANT( this );

      // First, try to find this input set in the map
      auto itr = m_systFilterMap.find(systConfig);

      // This is a new input set, so we need to filter it
      if(itr == m_systFilterMap.end()){

         // Filter input systematics with the affecting systematics
         SystematicSet myAppliedSystematics;
         if( SystematicSet::
             filterForAffectingSystematics( systConfig,
                                            m_affectingSystematics,
                                            myAppliedSystematics ) !=
             StatusCode::SUCCESS ) {
            return StatusCode::FAILURE;
         }

         // Insert the new filtered set onto our filter map
         itr = m_systFilterMap.insert
           (std::make_pair(systConfig, myAppliedSystematics)).first;

      }

      // Apply the filtered systematics
      if( sysApplySystematicVariation( itr->second ) !=
          StatusCode::SUCCESS ) {
         return StatusCode::FAILURE;
      }

      // Store the filtered systematics
      m_appliedSystematics = &itr->second;
      return StatusCode::SUCCESS;
   }

   StatusCode SystematicsTool::
   addAffectingSystematic( const SystematicVariation& systematic, bool recommended ) {

      RCU_READ_INVARIANT( this );

      SystematicRegistry& reg = SystematicRegistry::getInstance();
      reg.registerSystematic( systematic );
      m_affectingSystematics.insert( systematic );
      if( recommended ) {
         if( addRecommendedSystematic(systematic) != StatusCode::SUCCESS ) {
            return StatusCode::FAILURE;
         }
      }
      return StatusCode::SUCCESS;
   }

   StatusCode SystematicsTool::
   addRecommendedSystematic( const SystematicVariation& systematic ) {

      RCU_READ_INVARIANT( this );

      SystematicRegistry& reg = SystematicRegistry::getInstance();
      m_recommendedSystematics.insert( systematic );
      if( reg.addSystematicToRecommended( systematic ) !=
          StatusCode::SUCCESS ) {
         return StatusCode::FAILURE;
      }
      return StatusCode::SUCCESS;
   }

   void SystematicsTool::
   addAffectingSystematics( const SystematicSet& systematics ) {

      RCU_READ_INVARIANT( this );

      SystematicRegistry& reg = SystematicRegistry::getInstance();
      m_affectingSystematics.insert( systematics );
      reg.registerSystematics( systematics );
   }

   StatusCode SystematicsTool::
   addRecommendedSystematics( const SystematicSet& systematics ) {

      RCU_READ_INVARIANT( this );

      SystematicRegistry& reg = SystematicRegistry::getInstance();
      m_recommendedSystematics.insert( systematics );
      if( reg.addSystematicsToRecommended( systematics ) !=
          StatusCode::SUCCESS ) {
         return StatusCode::FAILURE;
      }
      return StatusCode::SUCCESS;
   }

   void SystematicsTool::testInvariant() const {

     //RCU_INVARIANT( this != 0 );
   }

} // namespace CP
