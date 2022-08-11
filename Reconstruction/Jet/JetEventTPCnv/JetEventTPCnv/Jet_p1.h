///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Jet_p1.h 
// Header file for class Jet_p1
// Author: S.Binet<binet@cern.ch>
// Date:   March 2007
/////////////////////////////////////////////////////////////////// 
#ifndef RECTPCNV_JET_P1_H 
#define RECTPCNV_JET_P1_H 

// STL includes
//#include <vector>

// DataModelAthenaPool includes
#include "DataModelAthenaPool/Navigable_p1.h"

// EventCommonTPCnv includes
#include "EventCommonTPCnv/P4PxPyPzE_p1.h"

// forward declarations
class JetCnv_p1;

class Jet_p1 
{
  /////////////////////////////////////////////////////////////////// 
  // Friend classes
  /////////////////////////////////////////////////////////////////// 

  // Make the AthenaPoolCnv class our friend
  friend class JetCnv_p1;

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
public: 

  /** Default constructor: 
   */
  Jet_p1();

  /** Destructor: 
   */
  ~Jet_p1() = default;

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
private: 

  /// the navigable part 
  Navigable_p1<uint32_t, double> m_nav;

  /// the 4-mom part
  P4PxPyPzE_p1 m_momentum;

}; 

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

inline Jet_p1::Jet_p1()
{}

#endif //> RECTPCNV_JET_P1_H
