// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: JEMEtSumsContainer_v1.h 631127 2014-11-26 09:54:48Z gwatts $
#ifndef XAODTRIGL1CALO_VERSIONS_JEMETSUMSCONTAINER_V1_H
#define XAODTRIGL1CALO_VERSIONS_JEMETSUMSCONTAINER_V1_H

// EDM include(s):
#include "AthContainers/DataVector.h"
// Local include(s):
#include "xAODTrigL1Calo/versions/JEMEtSums_v1.h"

namespace xAOD {
   /// Define the JEMEtSums as a simple DataVector
   typedef DataVector< xAOD::JEMEtSums_v1 > JEMEtSumsContainer_v1;
}

#endif // XAODTRIGL1CALO_VERSIONS_JEMETSUMSCONTAINER_V1_H
  
