/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGGERCNV_TRIGNAVIGATIONCNVTOOL_H
#define XAODTRIGGERCNV_TRIGNAVIGATIONCNVTOOL_H

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthAlgTool.h"

// Trigger include(s):
#include "TrigSteeringEvent/HLTResult.h" 

// Local include(s):
#include "xAODTriggerCnv/ITrigNavigationCnvTool.h"

namespace xAODMaker {

   /**
    *  @short Implementation of the ITrigNavigationCnvTool interface
    *
    *         This tool copies the contents of the "old" HLT::HLTResult
    *         object into an xAOD::TrigNavigation one.
    *
    * @author Tomasz Bold
    */
  class TrigNavigationCnvTool : public AthAlgTool,
				public virtual ITrigNavigationCnvTool {

  public:
    /// Regular AlgTool constructor
    TrigNavigationCnvTool( const std::string& type, const std::string& name,
                           const IInterface* parent );

    virtual ~TrigNavigationCnvTool();
    
    /// Function initialising the tool
    virtual StatusCode initialize() override;
    
    /// Function filling an xAOD::TrigNavigation object with information
    virtual StatusCode convert( const HLT::HLTResult* aod,
				xAOD::TrigNavigation* xaod ) const final;    

  private:
    
  }; // class TrigNavigationCnvTool
  
} // namespace xAODMaker

#endif // XAODTRIGGERCNV_TRIGNAVIGATIONCNVTOOL_H
