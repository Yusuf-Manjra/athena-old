/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
#include "TrigErrorMonTool.h"
#include "AthenaKernel/AthStatusCode.h"
#include "GaudiKernel/IAlgExecStateSvc.h"

// =============================================================================
// Standard constructor
// =============================================================================
TrigErrorMonTool::TrigErrorMonTool(const std::string& type, const std::string& name, const IInterface* parent)
: base_class(type, name, parent) {}

// =============================================================================
// Implementation of IStateful::initialize
// =============================================================================
StatusCode TrigErrorMonTool::initialize() {
  ATH_CHECK(m_monTool.retrieve(DisableTool{m_monTool.name().empty()}));
  ATH_CHECK(m_algToChainTool.retrieve(DisableTool{m_algToChainTool.name().empty()}));
  ATH_CHECK(m_aess.retrieve());
  
  if (!m_trigCostSvcHandle.empty()) ATH_CHECK(m_trigCostSvcHandle.retrieve());

  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of IStateful::finalize
// =============================================================================
StatusCode TrigErrorMonTool::finalize() {
  ATH_CHECK(m_monTool.release());
  ATH_CHECK(m_algToChainTool.release());
  ATH_CHECK(m_aess.release());
  return StatusCode::SUCCESS;
}

// =============================================================================
// The main method of the tool, ITrigErrorMonTool::algExecErrors
// =============================================================================
std::unordered_map<std::string_view, StatusCode> TrigErrorMonTool::algExecErrors(const EventContext& eventContext) const {
  std::unordered_map<std::string_view, StatusCode> algErrors;
  bool wasTimeout = false;
  for (const auto& [key, state] : m_aess->algExecStates(eventContext)) {
    if (!state.execStatus().isSuccess()) {
      ATH_MSG_DEBUG("Algorithm " << key << " returned StatusCode " << state.execStatus().message()
                    << " in event " << eventContext.eventID());
      algErrors[key.str()] = state.execStatus();
      auto monErrorAlgName = Monitored::Scalar<std::string>("ErrorAlgName", key.str());
      auto monErrorCode = Monitored::Scalar<std::string>("ErrorCode", state.execStatus().message());
      auto mon = Monitored::Group(m_monTool, monErrorAlgName, monErrorCode);

      if (m_algToChainTool.isEnabled()) {
        std::set<std::string> chainNames = m_algToChainTool->getActiveChainsForAlg(key.str(), eventContext);
        // Monitored::Collection requires operator[]
        std::vector<std::string> chainNamesVec(chainNames.begin(), chainNames.end());

        auto monErrorChainNames= Monitored::Collection<std::vector<std::string>>("ErrorChainName", chainNamesVec);
        Monitored::Group(m_monTool, monErrorChainNames, monErrorCode);
      }

      if (state.execStatus() == Athena::Status::TIMEOUT) {
        // Print report of most time consuming algorithms
        wasTimeout = true;
      }
    }
  }

  if (wasTimeout && !m_trigCostSvcHandle.empty()){
    std::string timeoutReport;
    if (m_trigCostSvcHandle->generateTimeoutReport(eventContext, timeoutReport) == StatusCode::FAILURE){
      ATH_MSG_ERROR("Generating timeout algorithm report failed.");
    }
    else if (timeoutReport.empty()){
      ATH_MSG_INFO("CostMonitoring not active in this event. A list of slow algorithms is not available.");
    }
    else {
      ATH_MSG_ERROR(timeoutReport);
    }
  }

  return algErrors;
}
