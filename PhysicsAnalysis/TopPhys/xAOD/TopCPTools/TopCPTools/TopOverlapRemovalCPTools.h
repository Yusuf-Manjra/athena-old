/*
   Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TOPCPTOOLS_TOPOVERLAPREMOVALCPTOOLS_H_
#define TOPCPTOOLS_TOPOVERLAPREMOVALCPTOOLS_H_

// Include what you use
#include <vector>
#include <string>

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/ToolHandleArray.h"
#include "AsgTools/AnaToolHandle.h"

// Overlap removal includes:
#include "AssociationUtils/IOverlapRemovalTool.h"
#include "AssociationUtils/ToolBox.h"

namespace top {
  class TopConfig;

  class OverlapRemovalCPTools final: public asg::AsgTool {
  public:
    explicit OverlapRemovalCPTools(const std::string& name);
    virtual ~OverlapRemovalCPTools() {}

    StatusCode initialize();
  private:
    std::shared_ptr<top::TopConfig> m_config;

    ORUtils::ToolBox m_ORtoolBox;
    ORUtils::ToolBox m_ORtoolBox_Loose;
    ORUtils::ToolBox m_ORtoolBox_softMuons_PFjets;
    ORUtils::ToolBox m_ORtoolBox_softMuons_Alljets;
    asg::AnaToolHandle<ORUtils::IOverlapRemovalTool> m_overlapRemovalTool;
    asg::AnaToolHandle<ORUtils::IOverlapRemovalTool> m_overlapRemovalTool_Loose;
    asg::AnaToolHandle<ORUtils::IOverlapRemovalTool> m_overlapRemovalTool_softMuons_PFjets;
    asg::AnaToolHandle<ORUtils::IOverlapRemovalTool> m_overlapRemovalTool_softMuons_Alljets;

    StatusCode setupOverlapRemoval();
  };
}  // namespace top

#endif  // TOPCPTOOLS_TOPOVERLAPREMOVALCPTOOLS_H_
