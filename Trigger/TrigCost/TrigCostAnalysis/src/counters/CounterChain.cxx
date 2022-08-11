/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTrigger/TrigCompositeContainer.h"
#include "TrigDataAccessMonitoring/ROBDataMonitor.h"

#include "CounterChain.h"


CounterChain::CounterChain(const std::string& name, const MonitorBase* parent) 
  : CounterBase(name, parent)
{
  regHistogram("Group_perCall", "Chain group/Call;Group;Calls", VariableType::kPerCall, kLinear, -0.5, 9.5, 10);
  regHistogram("Chain_perEvent", "Chain calls/Event;Chain call;Events", VariableType::kPerEvent, kLinear, -0.5, 49.5);
  regHistogram("AlgCalls_perEvent", "Algorithm Calls/Event;Calls;Events", VariableType::kPerEvent, kLinear, -0.5, 999.5, 100);
  regHistogram("Time_perCall", "CPU Time/Call;Time [ms];Calls", VariableType::kPerCall, kLog, 0.01, 100000);
  regHistogram("Time_perEvent", "CPU Time/Event;Time [ms];Events", VariableType::kPerEvent);
  regHistogram("UniqueTime_perCall", "Unique CPU Time/Call;Time [ms];Calls", VariableType::kPerCall, kLog, 0.01, 100000);
  regHistogram("ChainPassed_perEvent", "Passed chain/Event;Passsed;Events", VariableType::kPerEvent, kLinear, -0.5, 1.5, 2);
  regHistogram("Request_perEvent", "Number of requests/Event;Number of requests;Events", VariableType::kPerEvent, LogType::kLinear, -0.5, 299.5, 300);
  regHistogram("NetworkRequest_perEvent", "Number of network requests/Event;Number of requests;Events", VariableType::kPerEvent, LogType::kLinear, -0.5, 149.5, 150);
  regHistogram("CachedROBSize_perEvent", "Cached ROB Size/Event;ROB size;Events", VariableType::kPerEvent, LogType::kLinear, 0, 1024, 50);
  regHistogram("NetworkROBSize_perEvent", "Network ROB Size/Event;ROB size;Events", VariableType::kPerEvent, LogType::kLinear, 0, 1024, 50);
  regHistogram("RequestTime_perEvent", "ROB Elapsed Time/Event;Elapsed Time [ms];Events", VariableType::kPerEvent);
}


StatusCode CounterChain::newEvent(const CostData& data, size_t index, const float weight) {

  ATH_CHECK( increment("Chain_perEvent", weight) );

  // Fill the bins with groups and add the labels
  int bin = 1;
  for (const std::string& group : data.seededChains()[index].groups){
    ATH_CHECK( getVariable("Group_perCall").setBinLabel(bin, group) );
    ATH_CHECK( getVariable("Group_perCall").fill(group, weight) );
    ++bin;
  }

  if (data.seededChains()[index].isPassRaw){
    ATH_CHECK( increment("ChainPassed_perEvent", weight) );
  }

  // Monitor algorithms associated with chain name
  if (!data.chainToAlgMap().count(getName())) return StatusCode::SUCCESS;

  for (const size_t algIndex : data.chainToAlgMap().at(getName())){
    const xAOD::TrigComposite* alg = data.costCollection().at(algIndex);
    const uint32_t slot = alg->getDetail<uint32_t>("slot");
    if (slot != data.onlineSlot()) {
      continue; // When monitoring the master slot, this Monitor ignores algs running in different slots 
    }

    ATH_CHECK( increment("AlgCalls_perEvent", weight) );

    const uint64_t start = alg->getDetail<uint64_t>("start"); // in mus
    const uint64_t stop  = alg->getDetail<uint64_t>("stop"); // in mus
    const float cpuTime = timeToMilliSec(start, stop);
    ATH_CHECK( fill("Time_perEvent", cpuTime, weight) );
    ATH_CHECK( fill("Time_perCall", cpuTime, weight) );

    // Monitor data requests
    if (!data.algToRequestMap().count(algIndex)) continue;

    for (size_t requestIdx : data.algToRequestMap().at(algIndex)) {
      const xAOD::TrigComposite* request = data.rosCollection().at(requestIdx);
      const std::vector<unsigned> robs_history = request->getDetail<std::vector<unsigned>>("robs_history");
      const std::vector<uint32_t> robs_size = request->getDetail<std::vector<uint32_t>>("robs_size");

      bool networkRequestIncremented = false;
      for (size_t i = 0; i < robs_size.size(); ++i) {
        // ROB request was fetched over the network
        if (robs_history[i] == robmonitor::RETRIEVED) {
          // size is stored in words, should be in kilobytes
          ATH_CHECK( fill("NetworkROBSize_perEvent", robs_size[i] / 500., weight) );
          networkRequestIncremented = true;
        }
        // ROB request was cached
        else if (robs_history[i] == robmonitor::HLT_CACHED || robs_history[i] == robmonitor::DCM_CACHED) {
          ATH_CHECK( fill("CachedROBSize_perEvent", robs_size[i] / 500., weight) );
        }
      }

      ATH_CHECK( increment("Request_perEvent", weight) );

      if (networkRequestIncremented) {
        ATH_CHECK( increment("NetworkRequest_perEvent", weight) );
      }

      const float rosTime = timeToMilliSec(request->getDetail<uint64_t>("start"), request->getDetail<uint64_t>("stop"));
      ATH_CHECK( fill("RequestTime_perEvent", rosTime, weight) );
    }
  }

  // Monitor unique algorithms associated with chain name
  if (!data.chainToUniqAlgMap().count(getName())) return StatusCode::SUCCESS;

  for (const size_t algIndex : data.chainToUniqAlgMap().at(getName())){
    const xAOD::TrigComposite* alg = data.costCollection().at(algIndex);
    const uint32_t slot = alg->getDetail<uint32_t>("slot");
    if (slot != data.onlineSlot()) {
      continue;
    }
    const uint64_t start = alg->getDetail<uint64_t>("start"); // in mus
    const uint64_t stop  = alg->getDetail<uint64_t>("stop"); // in mus
    const float cpuTime = timeToMilliSec(start, stop);

    ATH_CHECK( fill("UniqueTime_perCall", cpuTime, weight) );
  }

  return StatusCode::SUCCESS;
}
