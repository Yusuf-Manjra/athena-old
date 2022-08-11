/*                                                                                                                      
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// This file is really -*- C++ -*-.
#ifndef TRIGT1MUCTPIPHASE1_MUCTPI_ATHALG_H
#define TRIGT1MUCTPIPHASE1_MUCTPI_ATHALG_H

/*
class description
*/

// Athena/Gaudi include(s):
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

namespace LVL1MUCTPIPHASE1 {
  class MUCTPI_AthTool;
}
namespace LVL1MUCTPIPHASE1 {

  class MUCTPI_AthAlg : public AthReentrantAlgorithm
  {
    
  public:
    
    MUCTPI_AthAlg( const std::string& name, ISvcLocator* pSvcLocator );

    /// Regular Gaudi algorithm initialization function
    virtual StatusCode initialize() override;
    /// Regular Gaudi algorithm execute function
    virtual StatusCode execute(const EventContext& eventContext) const override;


  private:
    
    ToolHandle<MUCTPI_AthTool> m_muctpiTool{this, "MUCTPI_AthTool", "LVL1MUCTPIPHASE1::MUCTPI_AthTool/MUCTPI_AthTool", "Tool to steer the MUCTPI simulation"};
  };
}

#endif // TRIGT1MUCTPIPHASE1_MUCTPI_ATHALG_H
