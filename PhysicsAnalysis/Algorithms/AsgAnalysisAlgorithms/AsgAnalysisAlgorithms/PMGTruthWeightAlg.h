/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


#ifndef ASG_ANALYSIS_ALGORITHMS__PMG_TRUTH_WEIGHT_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__PMG_TRUTH_WEIGHT_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <PMGAnalysisInterfaces/IPMGTruthWeightTool.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODEventInfo/EventInfo.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IPMGTruthWeightTool
  class PMGTruthWeightAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    PMGTruthWeightAlg (const std::string& name, 
                       ISvcLocator* pSvcLocator);


  public:
    StatusCode initialize () override;

  public:
    StatusCode execute () override;

    /// \brief the tool
  private:
    ToolHandle<PMGTools::IPMGTruthWeightTool> m_truthWeightTool;

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the event info we decorate
  private:
    SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
      this, "eventInfo", "EventInfo", "the event info object to run on"};

    /// \brief the decoration for the truth weights
  private:
    SysWriteDecorHandle<float> m_decoration {
      this, "decoration", "", "the decoration for the truth weights"};
  };
}

#endif
