/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// ViewDataVerifier.h 
// Header file for class ViewDataVerifier
// Author: B. Wynne <bwynne@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHVIEWS_VIEWDATAVERIFIER_H
#define ATHVIEWS_VIEWDATAVERIFIER_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/DataObjID.h"

namespace AthViews {

class ViewDataVerifier
  : public ::AthReentrantAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  ViewDataVerifier( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~ViewDataVerifier(); 

  // Assignment operator: 
  //ViewDataVerifier &operator=(const ViewDataVerifier &alg); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;

 private:

  /// Default constructor: 
  ViewDataVerifier();

  /// Containers to verify
  Gaudi::Property< DataObjIDColl > m_load{ this, "DataObjects", DataObjIDColl(), "Objects to confirm are found in this view" };
  
  // vars
}; 

} //> end namespace AthViews
#endif //> !ATHVIEWS_VIEWDATAVERIFIER_H
