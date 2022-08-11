/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGTRTHTHHYPOTOOL_H
#define TRIGTRTHTHHYPOTOOL_H 1

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "ITrigTRTHTHhypoTool.h"

/**
 * @class Implementation of the TRT High Threshold Hit selection
 * @brief 
 **/

class TrigTRTHTHhypoTool : public extends<AthAlgTool, ITrigTRTHTHhypoTool> { 
 public: 
  TrigTRTHTHhypoTool( const std::string& type, 
			 const std::string& name, 
			 const IInterface* parent );

  virtual StatusCode initialize() override;

  virtual StatusCode decide( std::vector<ITrigTRTHTHhypoTool::RNNOutputInfo>& input )  const override;

  virtual bool decide( const ITrigTRTHTHhypoTool::RNNOutputInfo& i ) const override;

 private:

  HLT::Identifier m_decisionId;

  Gaudi::Property< bool > m_acceptAll { this, "AcceptAll", false , "Bins of eta" };
  Gaudi::Property< unsigned int > m_minTRTHTHitsRoad { this, "MinTRTHTHitsRoad", 20 , "Bins of eta" };
  Gaudi::Property< float > m_minHTratioRoad { this, "MinHTRatioRoad", 0.4 , "Bins of eta" };
  Gaudi::Property< unsigned int > m_minTRTHTHitsWedge { this, "MinTRTHTHitsWedge", 30 , "Bins of eta" };
  Gaudi::Property< float > m_minHTratioWedge { this, "MinHTRatioWedge", 0.5 , "Bins of eta" };
  Gaudi::Property< bool > m_doWedge { this, "DoWedge", true , "Bins of eta" };
  Gaudi::Property< bool > m_doRoad { this, "DoRoad", false , "Bins of eta" };

  ToolHandle< GenericMonitoringTool >   m_monTool{ this, "MonTool", "", "Monitoring tool" };

};
#endif

