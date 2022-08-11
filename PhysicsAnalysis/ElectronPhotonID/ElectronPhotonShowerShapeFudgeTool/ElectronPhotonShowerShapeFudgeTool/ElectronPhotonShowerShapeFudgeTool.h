/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _ELECTRONPHOTONSHOWERSHAPEFUDGETOOL_H
#define _ELECTRONPHOTONSHOWERSHAPEFUDGETOOL_H

/**
   @class ElectronPhotonShowerShapeFudgeTool
   @brief Tool to fudge photon MC shower shapes.

   @author Rob Roy Fletcher (University of Pennsylvania) rob.fletcher@cern.ch
   @date   August 2014


*/
// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "ElectronPhotonShowerShapeFudgeTool/TPhotonMCShifterTool.h"
#include "ElectronPhotonShowerShapeFudgeTool/TElectronMCShifterTool.h"
#include "EgammaAnalysisInterfaces/IElectronPhotonShowerShapeFudgeTool.h"
#include "TEnv.h"

class ElectronPhotonShowerShapeFudgeTool : public asg::AsgTool, virtual public  IElectronPhotonShowerShapeFudgeTool
{
   /// Declare the interface that the class provides
   ASG_TOOL_CLASS(ElectronPhotonShowerShapeFudgeTool, IElectronPhotonShowerShapeFudgeTool)

public:
  /** Standard constructor */
  ElectronPhotonShowerShapeFudgeTool( const std::string& myname);


  /** Standard destructor */
  virtual ~ElectronPhotonShowerShapeFudgeTool();

public:
  /** Gaudi Service Interface method implementations */
  virtual StatusCode initialize() override final;


public:
  virtual const CP::CorrectionCode applyCorrection(
    xAOD::Photon& ph) const override final;

  virtual const CP::CorrectionCode applyCorrection(
    xAOD::Electron& el) const override final;

  virtual const CP::CorrectionCode correctedCopy(
    const xAOD::Photon& ph,
    xAOD::Photon*& output) const override final;

  virtual const CP::CorrectionCode correctedCopy(
    const xAOD::Electron& el,
    xAOD::Electron*& output) const override final;

private:

   TPhotonMCShifterTool* m_ph_rootTool;
   TElectronMCShifterTool* m_el_rootTool;

   int m_preselection;

   std::string m_configFile;
   std::string m_ffFile;

   /** Copied over from the configuration helper so that the selector tools do not need to be included */
   std::vector<float> GetFloatVector(const std::string& input,  TEnv& env);

   bool strtof(const std::string& input, float& f);
}; // class ElectronPhotonShowerShapeFudgeTool




#endif // _IFUDGEMCTOOL_H
