/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUCOMMONCALCVARS_H
#define TAURECTOOLS_TAUCOMMONCALCVARS_H

#include "tauRecTools/TauRecToolBase.h"


/**
 * @brief Calculate variables which rely on tracks and precalculated cell/cluster information.
 * 
 *  All variables here can be recalculated using AODs.
 * 
 * @author Stan Lai
 * @author Felix Friedrich
 */

class TauCommonCalcVars : public TauRecToolBase {
public:
    //-----------------------------------------------------------------
    // Constructor and destructor
    //-----------------------------------------------------------------
    TauCommonCalcVars(const std::string& name="TauCommonCalcVars");
    ASG_TOOL_CLASS2(TauCommonCalcVars, TauRecToolBase, ITauToolBase)
    ~TauCommonCalcVars();
    
    virtual StatusCode execute(xAOD::TauJet& pTau) const override;

private:
    int m_isolationTrackType;
};

#endif // TAURECTOOLS_TAUCOMMONCALCVARS_H
