/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef egammaMonitorElectronAlgorithm_H
#define egammaMonitorElectronAlgorithm_H

#include "egammaMonitorBaseAlgorithm.h"

#include "StoreGate/ReadDecorHandleKeyArray.h"

class egammaMonitorElectronAlgorithm: public egammaMonitorBaseAlgorithm
{

  public:

    egammaMonitorElectronAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  protected:
    
    void filltopoElectronTrackCaloMatch(const EventContext& ctx) const;
    void filltopoElectronShowerShapes(const EventContext& ctx) const;
    void filltopoElectronIsolation(const EventContext& ctx) const;
     
  private:

    TString m_baseName;  
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronsKey{this, "ElectronKey", "Electrons", ""};
    SG::ReadDecorHandleKeyArray<xAOD::ElectronContainer> m_isoKey{this, "IsoVarKeys", { }, "The list of isolation variables to be monitored"};
    ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};
};
 


#endif
