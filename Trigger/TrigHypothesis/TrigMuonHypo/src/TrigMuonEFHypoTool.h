/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONHYPO_TRIGMUONEFHYPOTOOL_H 
#define TRIGMUONHYPO_TRIGMUONEFHYPOTOOL_H 1
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h" 
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "xAODMuon/MuonContainer.h"
#include "GaudiKernel/SystemOfUnits.h"

// include Muon SelectionTool for quality criteria 
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

class StoreGateSvc;
class TrigMuonEFHypoTool: public ::AthAlgTool {
  enum { MaxNumberTools = 20 };  
 public:
  TrigMuonEFHypoTool(const std::string& type, const std::string & name, const IInterface* parent);
  ~TrigMuonEFHypoTool();
   
  struct MuonEFInfo {
  MuonEFInfo( TrigCompositeUtils::Decision* d, 
              const xAOD::Muon* m,
	      const TrigCompositeUtils::Decision* previousDecision )
    : decision( d ), 
      muon( m ),
      previousDecisionIDs(TrigCompositeUtils::decisionIDs( previousDecision ).begin(), 
			  TrigCompositeUtils::decisionIDs( previousDecision ).end() )
    {}
      
    TrigCompositeUtils::Decision* decision;
    const xAOD::Muon* muon;
    const TrigCompositeUtils::DecisionIDContainer previousDecisionIDs;
  };
  virtual StatusCode initialize() override;    
  StatusCode decide(std::vector<TrigMuonEFHypoTool::MuonEFInfo>& toolInput) const ;
  float getdphi(float phi1, float phi2) const;
 private:
  bool passedQualityCuts(const xAOD::Muon* muon) const;
  bool decideOnSingleObject(TrigMuonEFHypoTool::MuonEFInfo& input, size_t cutIndex) const;
  StatusCode inclusiveSelection(std::vector<TrigMuonEFHypoTool::MuonEFInfo>& toolInput) const;
  StatusCode multiplicitySelection(std::vector<TrigMuonEFHypoTool::MuonEFInfo>& toolInput) const;

  HLT::Identifier m_decisionId;
  // Properties:
  Gaudi::Property< bool > m_nscan {
    this, "NarrowScan", false, "Apply narrow scan" };
  Gaudi::Property< float > m_conesize {
    this, "ConeSize", 0.5, "Narrow scan cone size" };
  Gaudi::Property< bool > m_muonqualityCut {
    this, "MuonQualityCut", false, "Ignore selection" };
  Gaudi::Property< std::vector<std::vector<double>> > m_ptBins {
    this, "PtBins", { {0, 2.5} }, "Bins range of each pT threshold" };
  Gaudi::Property< std::vector<std::vector<double>> > m_ptThresholds {
    this, "PtThresholds", { {5.49*Gaudi::Units::GeV} }, "Track pT requirement ( separate threshold for each muon )" };
  Gaudi::Property< bool > m_acceptAll {
    this, "AcceptAll", false, "Ignore selection" };
  Gaudi::Property<bool>  m_decisionPerRoI { 
    this, "DecisionPerRoI", true, "Is multiplicity requirement refering to muons ( false ) or RoIs with muons ( true ), relevant only in when multiplicity > 1" };
  Gaudi::Property<bool> m_threeStationCut{
    this, "RequireThreeStations", false, "Apply cut on N GoodPrecisionLayers in endcaps"};
  Gaudi::Property<bool> m_doSA{
    this, "RequireSAMuons", false, "Apply cut on SA muons (otherwise require combined muons)"};
   Gaudi::Property< float > m_d0min {
        this, "MinimumD0", 0., "lower d0 cut (mm)"};
   Gaudi::Property< bool > m_checkOvlp {
        this, "RemoveOverlaps", false, "do overlap removal"};

  // Other members:   
  std::vector<size_t> m_bins={0};
  ToolHandle< GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };
  ToolHandle<CP::IMuonSelectionTool> m_muonSelTool{this, "MuonSelectionTool", "CP::MuonSelectionTool/MuonSelectionTool", "Tool for muon quality selection"};
  xAOD::Muon::TrackParticleType m_type;


};
#endif
