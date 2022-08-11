/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONHYPO_TRIGMUONEFINVMASSHYPOTOOL_H 
#define TRIGMUONHYPO_TRIGMUONEFINVMASSHYPOTOOL_H 1
#include <string>
#include "AthenaBaseComps/AthAlgTool.h" 
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h" 
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "DecisionHandling/ComboHypoToolBase.h"

class StoreGateSvc;
class TrigMuonEFInvMassHypoTool: public ::ComboHypoToolBase {

 public:
  TrigMuonEFInvMassHypoTool(const std::string& type, const std::string & name, const IInterface* parent);
  virtual ~TrigMuonEFInvMassHypoTool();
 
  virtual StatusCode initialize() override;    

 private:
  bool executeAlg(const std::vector<Combo::LegDecision>& combinaiton) const override;

  // Properties:
  Gaudi::Property< double> m_invMassLow {
    this, "InvMassLow", -1.0, "Low threshold for invariant mass cut" };
  Gaudi::Property< double> m_invMassHigh {
    this, "InvMassHigh", -1.0, "High threshold for invariant mass cut" };
  Gaudi::Property< bool > m_acceptAll {
    this, "AcceptAll", false, "Ignore selection" };
  Gaudi::Property< bool > m_selOS {
    this, "SelectOppositeSign", false, "Select only oppositly charged pairs" };

  // Other members:   
  ToolHandle< GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };
};
#endif
