/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HLTSEEDING_CTPUNPACKINGTOOLBASE_H
#define HLTSEEDING_CTPUNPACKINGTOOLBASE_H

#include "HLTSeeding/ICTPUnpackingTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"


namespace ROIB {
  class RoIBResult;
}

/**
 * Base class for CTP unpacking tools
 *
 * Provides common features needed by all CPT unpacker.
 */
class CTPUnpackingToolBase : public extends<AthAlgTool, ICTPUnpackingTool> {
public:

  CTPUnpackingToolBase(const std::string& type,
                       const std::string& name,
                       const IInterface* parent);

  virtual StatusCode decode(const ROIB::RoIBResult& /*roib*/,
                            HLT::IDVec& /*enabledChains*/) const override;

  virtual StatusCode initialize() override;

  virtual StatusCode passBeforePrescaleSelection(const ROIB::RoIBResult* /*roib*/,
                            const std::vector<std::string>& /*l1ItemNames*/, bool& /*pass*/) const override;

  // Derived class may override this to return true if its decode() method doesn't depend on RoIBResult
  virtual bool isEmulated() const override {return false;}

protected:

  static constexpr int s_CTPIDForUnseededChains = -1;
  std::unordered_map<int, HLT::IDVec> m_ctpToChain;

  Gaudi::Property<bool> m_forceEnable{
    this, "ForceEnableAllChains", false, "Enables all chains in each event, testing mode"};

  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};
};

#endif
