// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// $Id: TrigCompositeAuxContainer_v2.h $
#ifndef XAODTRIGGER_VERSIONS_TRIGCOMPOSITEAUXCONTAINER_V2_H
#define XAODTRIGGER_VERSIONS_TRIGCOMPOSITEAUXCONTAINER_V2_H

// System include(s):
extern "C" {
#   include <stdint.h>
}
#include <vector>
#include <string>

// xAOD include(s):
#include "xAODCore/AuxContainerBase.h"

#include "xAODTrigger/TrigComposite.h"
#include "CxxUtils/sgkey_t.h"

namespace xAOD {

   /// Auxiliary store for TrigComposite containers
   ///
   /// This v2 changes the base class from ByteStreamAuxContainer_v1 to AuxContainerBase 
   /// for Run 3. It additionally adds "decisions" as a static member. Otherwise, it is the same.
   ///
   /// $Date: 2019-01-23 $
   ///
   class TrigCompositeAuxContainer_v2 : public AuxContainerBase {

   public:
      using sgkey_t = SG::sgkey_t;

      /// Default constuctor
      TrigCompositeAuxContainer_v2();

   private:
      std::vector< std::string > name;

      std::vector< std::vector< std::string > > linkColNames;
      std::vector< std::vector< sgkey_t > >     linkColKeys;
      std::vector< std::vector< uint16_t > >    linkColIndices;
      std::vector< std::vector< uint32_t > >    linkColClids;

      std::vector< std::vector< TrigCompositeUtils::DecisionID > >    decisions;

   }; // class TrigCompositeAuxContainer_v2

} // namespace xAOD

// Declare the inheritance of the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::TrigCompositeAuxContainer_v2, xAOD::AuxContainerBase );

#endif // XAODTRIGGER_VERSIONS_TRIGCOMPOSITEAUXCONTAINER_V2_H
