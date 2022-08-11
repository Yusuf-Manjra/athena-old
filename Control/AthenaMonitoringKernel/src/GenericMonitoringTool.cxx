/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include <map>
#include <mutex>
#include <algorithm>

#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TProfile2D.h>

#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "AthenaMonitoringKernel/HistogramDef.h"
#include "AthenaMonitoringKernel/HistogramFiller.h"
#include "AthenaMonitoringKernel/IMonitoredVariable.h"

#include "HistogramFiller/HistogramFillerFactory.h"

using namespace Monitored;

GenericMonitoringTool::~GenericMonitoringTool() { }

StatusCode GenericMonitoringTool::initialize() {
  ATH_CHECK(m_histSvc.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode GenericMonitoringTool::start() {
  if ( not m_explicitBooking ) {
    ATH_MSG_DEBUG("Proceeding to histogram booking");
    return book();
  }
  return StatusCode::SUCCESS;
}

StatusCode GenericMonitoringTool::stop() {
  m_alwaysCreateFillers.clear();
  m_fillers.clear();
  if (m_registerHandler) {
    ATH_MSG_DEBUG("Deregistering incident handler");
    IIncidentSvc* incSvc{nullptr};
    ATH_CHECK(service("IncidentSvc", incSvc));
    incSvc->removeListener(this, IncidentType::BeginEvent);
  }
  return StatusCode::SUCCESS;
}

void GenericMonitoringTool::handle( const Incident& ) {
  for (const auto& filler : m_alwaysCreateFillers) {
    filler->touch();
  }
}

StatusCode GenericMonitoringTool::book() {

  // If no histogram path given use parent or our own name
  if (m_histoPath.empty()) {
    auto named = dynamic_cast<const INamedInterface*>(parent());
    m_histoPath = named ? named->name() : name();
  }

  // Replace dot (e.g. MyAlg.MyTool) with slash to create sub-directory
  std::replace( m_histoPath.begin(), m_histoPath.end(), '.', '/' );

  ATH_MSG_DEBUG("Booking histograms in path: " << m_histoPath.value());

  HistogramFillerFactory factory(this, m_histoPath);

  for (const std::string& item : m_histograms) {
    if (item.empty()) {
      ATH_MSG_DEBUG( "Skipping empty histogram definition" );
      continue;
    }
    ATH_MSG_DEBUG( "Configuring monitoring for: " << item );
    HistogramDef def = HistogramDef::parse(item);

    if (def.ok) {
      std::shared_ptr<HistogramFiller> filler(factory.create(def));

      if (filler) {
        if (def.kAlwaysCreate) {
          if (m_registerHandler) {
            m_alwaysCreateFillers.push_back(filler); // prepare list of fillers for handler
          } else {
            filler->touch(); // create now and be done with it
          }
        }
      	m_fillers.push_back(filler);
      } else {
        ATH_MSG_WARNING( "The histogram filler cannot be instantiated for: " << def.name );
      }
    } else {
      ATH_MSG_ERROR( "Unparsable histogram definition: " << item );
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG( "Monitoring for variable " << def.name << " prepared" );
  }

  if ( m_fillers.empty() && m_failOnEmpty ) {
    std::string hists;
    for (const auto &h : m_histograms) hists += (h+",");
    ATH_MSG_ERROR("No monitored variables created based on histogram definition: [" << hists <<
                  "] Remove this monitoring tool or check its configuration.");
    return StatusCode::FAILURE;
  }

  // are there some histograms that should always be made?
  // then register to be notified on every event
  if (! m_alwaysCreateFillers.empty() && m_registerHandler) {
    ATH_MSG_DEBUG("Registering incident handler");
    IIncidentSvc* incSvc{nullptr};
    ATH_CHECK(service("IncidentSvc", incSvc));
    incSvc->addListener(this, IncidentType::BeginEvent);
  }

  return StatusCode::SUCCESS;
}

namespace Monitored {
    std::ostream& operator<< ( std::ostream& os, const std::reference_wrapper<Monitored::IMonitoredVariable>& rmv ) {
        std::string s = rmv.get().name();
        return os << s;
    }
}

namespace std {
  // Next four functions are for speeding up lookups in the the caching of invokeFillers
  // They allow us to directly compare keys of the cache std::map
  // with vectors of IMonitoredVariables, avoiding memory allocations
  // these compare strings and IMonitoredVariables
  bool operator<(const std::string& a, const std::reference_wrapper<Monitored::IMonitoredVariable>& b)  {
    return a < b.get().name();
  }
  bool operator<(const std::reference_wrapper<Monitored::IMonitoredVariable>& a, const std::string& b)  {
    return a.get().name() < b;
  }

  // lexicographical comparison of cache map items and vector of IMonitoredVariables
  bool operator<(const std::vector<std::string>& lhs,
                 const std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
  }
  bool operator<(const std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>>& lhs,
                 const std::vector<std::string>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
  }
}

namespace {
  // this exists to avoid reallocating memory on every invokeFillers call
  thread_local Monitored::HistogramFiller::VariablesPack tl_vars ATLAS_THREAD_SAFE;

  // Ensure that TLS defined in this library actually gets used.
  // Avoids a potential slowdown in accessing TLS seen in simualation.
  // See ATLASSIM-4932.
  [[maybe_unused]]
  const Monitored::HistogramFiller::VariablesPack& varDum = tl_vars;
}

void GenericMonitoringTool::invokeFillers(const std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>>& monitoredVariables) const {
  // This is the list of fillers to consider in the invocation.
  // If we are using the cache then this may be a proper subset of m_fillers; otherwise will just be m_fillers
  const std::vector<std::shared_ptr<Monitored::HistogramFiller>>* fillerList{nullptr};
  // do we need to update the cache?
  bool makeCache = false;
  // pointer to list of matched fillers, if we need to update the cache (default doesn't create the vector)
  std::unique_ptr<std::vector<std::shared_ptr<Monitored::HistogramFiller>>> matchedFillerList;
  if (m_useCache) {
    // lock the cache during lookup
    std::scoped_lock cacheguard(m_cacheMutex);
    const auto match = m_fillerCacheMap.find(monitoredVariables);
    if (match != m_fillerCacheMap.end()) {
      fillerList = match->second.get();
    } else {
      fillerList = &m_fillers;
      matchedFillerList = std::make_unique<std::vector<std::shared_ptr<Monitored::HistogramFiller>>>();
      makeCache = true;
    }
  } else {
    fillerList = &m_fillers;
  }

  for ( auto filler: *fillerList ) {
    tl_vars.reset();
    const int fillerCardinality = filler->histogramVariablesNames().size() + (filler->histogramWeightName().empty() ? 0: 1) + (filler->histogramCutMaskName().empty() ? 0 : 1);

    if ( fillerCardinality == 1 ) { // simplest case, optimising this to be super fast
      for ( auto& var: monitoredVariables ) {
        if ( var.get().name().compare( filler->histogramVariablesNames()[0] ) == 0 )  {
          tl_vars.var[0] = &var.get();
          {
            auto guard{filler->getLock()};
            filler->fill( tl_vars );
          }
          if (makeCache) { 
            matchedFillerList->push_back(filler); 
          }
          break;
        }
      }
    } else { // a more complicated case, and cuts or weights
      int matchesCount = 0;
      for ( const auto& var: monitoredVariables ) {
        bool matched = false;
        for ( unsigned fillerVarIndex = 0; fillerVarIndex < filler->histogramVariablesNames().size(); ++fillerVarIndex ) {
          if ( var.get().name().compare( filler->histogramVariablesNames()[fillerVarIndex] ) == 0 ) {
            tl_vars.set(fillerVarIndex, &var.get());
            matched = true;
            matchesCount++;
            break;
          }
        }
        if ( matchesCount == fillerCardinality ) break;
        if ( not matched ) { // may be a weight or cut variable still
          if ( var.get().name().compare( filler->histogramWeightName() ) == 0 )  {
            tl_vars.weight = &var.get();
            matchesCount ++;
          } else if ( var.get().name().compare( filler->histogramCutMaskName() ) == 0 )  {
            tl_vars.cut = &var.get();
            matchesCount++;
         }
        }
        if ( matchesCount == fillerCardinality ) break;
      }
      if ( matchesCount == fillerCardinality ) {
        {
          auto guard{filler->getLock()};
          filler->fill( tl_vars );
        }
        if (makeCache) { 
          matchedFillerList->push_back(filler); 
        }
      } else if ( ATH_UNLIKELY( matchesCount != 0 ) ) { // something has matched, but not all, worth informing user
        invokeFillersDebug(filler, monitoredVariables);
      }
    }
  }

  if (makeCache) {
    // we may hit this multiple times. If another thread has updated the cache in the meanwhile, don't update
    // (or we might delete the fillerList under another thread)
    std::scoped_lock cacheguard(m_cacheMutex);
    const auto match = m_fillerCacheMap.find(monitoredVariables);
    if (match == m_fillerCacheMap.end()) {
      std::vector<std::string> key;
      key.reserve(monitoredVariables.size());
      for (const auto& mv : monitoredVariables) {
        key.push_back(mv.get().name());
      }
      m_fillerCacheMap[key].swap(matchedFillerList);
    }
  }
}

void GenericMonitoringTool::invokeFillersDebug(const std::shared_ptr<Monitored::HistogramFiller>& filler,
                                               const std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>>& monitoredVariables) const {
  bool reasonFound = false;
  if (ATH_UNLIKELY(!filler->histogramWeightName().empty() && !tl_vars.weight)) {
    reasonFound = true;
    ATH_MSG_DEBUG("Filler weight not found in monitoredVariables:"
      << "\n  Filler weight               : " << filler->histogramWeightName()
      << "\n  Asked to fill from mon. tl_vars: " << monitoredVariables);
  }
  if (ATH_UNLIKELY(!filler->histogramCutMaskName().empty() && !tl_vars.cut)) {
    reasonFound = true;
    ATH_MSG_DEBUG("Filler cut mask not found in monitoredVariables:"
      << "\n  Filler cut mask             : " << filler->histogramCutMaskName()
      << "\n  Asked to fill from mon. tl_vars: " << monitoredVariables);
  }
  if ( not reasonFound ) {
    ATH_MSG_DEBUG("Filler has different variables than monitoredVariables:"
      << "\n  Filler variables            : " << filler->histogramVariablesNames()
      << "\n  Asked to fill from mon. tl_vars: " << monitoredVariables
      << "\n  Selected monitored variables: " << tl_vars.names() );
  }
}

uint32_t GenericMonitoringTool::runNumber() {
  return Gaudi::Hive::currentContext().eventID().run_number();
}

uint32_t GenericMonitoringTool::lumiBlock() {
  return Gaudi::Hive::currentContext().eventID().lumi_block();
}
