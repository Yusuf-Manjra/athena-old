/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_MVATESVARIABLEDECORATOR_H
#define TAURECTOOLS_MVATESVARIABLEDECORATOR_H

#include "tauRecTools/TauRecToolBase.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODEventShape/EventShape.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"

class MvaTESVariableDecorator : public TauRecToolBase {
 
public:
  
  ASG_TOOL_CLASS2( MvaTESVariableDecorator, TauRecToolBase, ITauToolBase )
    
  MvaTESVariableDecorator(const std::string& name="MvaTESVariableDecorator");
  
  virtual ~MvaTESVariableDecorator() = default;
    
  virtual StatusCode initialize() override;
  
  virtual StatusCode execute(xAOD::TauJet& xTau) const override;

private:

  bool m_doVertexCorrection;

  SG::ReadDecorHandleKey<xAOD::EventInfo> m_aveIntPerXKey {this, 
      "averageInteractionsPerCrossingKey", 
      "EventInfo.averageInteractionsPerCrossing",
      "Decoration for Average Interaction Per Crossing"};
  
  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey {this,
      "Key_vertexInputContainer",
      "PrimaryVertices",
      "input vertex container key"};

  SG::ReadHandleKey<xAOD::EventShape> m_eventShapeKey {this,
      "EventShapeKey",
      "Kt4LCTopoOriginEventShape",
      "EventShape key"};

};

#endif // TAURECTOOLS_MVATESVARIABLEDECORATOR_H
