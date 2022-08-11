/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//! This class implements a tool to calculate ID input variables and add them to the tau aux store
/*!
 * Tau ID input variable calculator tool
 *
 * Author: Lorenz Hauswald
 */

#ifndef TAURECTOOLS_TAUIDVARCALCULATOR_H
#define TAURECTOOLS_TAUIDVARCALCULATOR_H

#include "tauRecTools/TauRecToolBase.h"


class TauIDVarCalculator: public TauRecToolBase {

public:
  
  ASG_TOOL_CLASS2(TauIDVarCalculator, TauRecToolBase, ITauToolBase)
  
  TauIDVarCalculator(const std::string& name = "TauIDVarCalculator");
  
  virtual ~TauIDVarCalculator() = default;

  virtual StatusCode execute(xAOD::TauJet&) const override;

  static const float LOW_NUMBER;

private:

  bool m_doVertexCorrection;
};

#endif // TAURECTOOLS_TAUIDVARCALCULATOR_H
