/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GOODRUNSLISTS_GRLSELECTORALG_H
#define GOODRUNSLISTS_GRLSELECTORALG_H 1

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/ToolHandle.h>
#include <AsgAnalysisInterfaces/IGoodRunsListSelectionTool.h>
#include <EventBookkeeperTools/FilterReporterParams.h>

///Author: will buttinger
///To use this alg in your joboptions:
///masterseq = CfgMgr.AthSequencer("AthMasterSeq")
///ToolSvc += CfgMgr.GoodRunsListSelectionTool("myGRLTool",GoodRunsListVec=["my.grl.xml"])
///masterseq += CfgMgr.GRLSelectorAlg(Tool=ToolSvc.myGRLTool)


class GRLSelectorAlg: public EL::AnaAlgorithm { 
 public: 
  GRLSelectorAlg( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~GRLSelectorAlg(); 

  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();

 private: 
  ToolHandle<IGoodRunsListSelectionTool> m_grlTool;
  FilterReporterParams m_filterParams {this, "GoodRunsList", "Good Runs Lists selection"};
}; 

#endif //> !GOODRUNSLISTS_GRLSELECTORALG_H
