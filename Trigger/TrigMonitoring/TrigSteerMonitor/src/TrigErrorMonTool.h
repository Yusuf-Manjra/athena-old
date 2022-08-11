/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGSTEERMONITOR_TRIGERRORMONTOOL_H
#define TRIGSTEERMONITOR_TRIGERRORMONTOOL_H

#include "TrigSteerMonitor/ITrigErrorMonTool.h"
#include "TrigCostMonitor/ITrigCostSvc.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "GaudiKernel/EventContext.h"
#include "TrigCompositeUtils/AlgToChainTool.h"

class IAlgExecStateSvc;

/**
 * @class TrigErrorMonTool
 * @brief Retrieves and monitors all non-success status codes returned by algorithms
 **/
class TrigErrorMonTool : public extends<AthAlgTool, ITrigErrorMonTool> {
public:
  TrigErrorMonTool(const std::string& type, const std::string& name, const IInterface* parent);

  // ------------------------- IStateful methods -------------------------------
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  // ------------------------- ITrigErrorMonTool methods -----------------------
  /// Produce a subset of IAlgExecStateSvc::algExecStates with only non-success StatusCodes and fill relevant histograms
  virtual std::unordered_map<std::string_view, StatusCode> algExecErrors(const EventContext& eventContext) const override;

private:
  ServiceHandle<IAlgExecStateSvc> m_aess{this, "AlgExecStateSvc", "AlgExecStateSvc"};
  ServiceHandle<ITrigCostSvc>      m_trigCostSvcHandle{this, "TrigCostSvc", ""};
  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};
  ToolHandle<TrigCompositeUtils::AlgToChainTool> m_algToChainTool {this, "AlgToChainTool", "",  "Tool to retrieve chains for algorithm"};
};


#endif // TRIGSTEERMONITOR_TRIGERRORMONTOOL_H
