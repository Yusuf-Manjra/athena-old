/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-

#ifndef __ASGELECTRONISEMSELECTOR__
#define __ASGELECTRONISEMSELECTOR__

/**
   @class AsgElectronIsEMSelector
   @brief Tool to select objects.

   @author Jovan Mitrevski (UCSC) Karsten Koeneke (CERN)
   @date   Dec 2011 - Fab 2012

   Based on egammaElectronCutIDTool, by F. Derue.

   11-MAR-2014, convert to ASGTool (Jovan Mitrevski)

*/

// Atlas includes
#include "AsgTools/AsgTool.h"
// Include the interfaces
#include "EgammaAnalysisInterfaces/IAsgElectronIsEMSelector.h"

#include <string>

class EventContext;

namespace Root {
class TElectronIsEMSelector;
}

class AsgElectronIsEMSelector
  : public asg::AsgTool
  , virtual public IAsgElectronIsEMSelector
{

  ASG_TOOL_CLASS3(AsgElectronIsEMSelector,
                  IAsgElectronIsEMSelector,
                  IAsgEGammaIsEMSelector,
                  IAsgSelectionTool)

public:
  /** Standard constructor */
  AsgElectronIsEMSelector(const std::string& myname);

  /** Standard destructor */
  virtual ~AsgElectronIsEMSelector();

  /** Gaudi Service Interface method implementations */
  virtual StatusCode initialize() override;

  /// @name Methods from the  IAsgSelectionTool interface
  /// @{

  /** Method to get the plain AcceptInfo.
      This is needed so that one can already get the AcceptInfo
      and query what cuts are defined before the first object
      is passed to the tool. */
  virtual const asg::AcceptInfo& getAcceptInfo() const override final;

  /** Accept with generic interface */
  virtual asg::AcceptData accept(const xAOD::IParticle* part) const override final;
  virtual asg::AcceptData accept(const EventContext& ctx,
                                 const xAOD::IParticle* part) const override final;
  ///@}

  /** Accept with Egamma objects */
  virtual asg::AcceptData accept(const EventContext& ctx,
                                 const xAOD::Egamma* part) const override final;
  /** Method to get the operating point */
  virtual std::string getOperatingPointName() const override final;

  ///@}

  /// @name Methods from the IAsgElectronIsEMSelectorinterface
  /// @{
  /** Accept with Photon objects */
  virtual asg::AcceptData accept(const EventContext& ctx,
                                 const xAOD::Photon* part) const override final;

  /** Accept with Electron objects */
  virtual asg::AcceptData accept(const EventContext& ctx,
                                 const xAOD::Electron* part) const override final;
  // The main execute method
  virtual StatusCode execute(const EventContext& ctx,
                             const xAOD::Egamma* eg,
                             unsigned int& isEM) const override final;
  /// @}

  // Private member variables
private:
  unsigned int calocuts_electrons(const xAOD::Egamma* eg,
                                  float eta2,
                                  double et,
                                  double trigEtTh,
                                  unsigned int iflag) const;

  unsigned int TrackCut(const xAOD::Electron* eg,
                        float eta2,
                        double et,
                        double energy,
                        unsigned int iflag) const;

  /** Working Point */
  std::string m_WorkingPoint;

  /** Config File */
  std::string m_configFile;

  /** Pointer to the underlying ROOT based tool */
  Root::TElectronIsEMSelector* m_rootTool;

  /** @brief use f3core or f3 (default: use f3)*/
  bool m_useF3core;

  /// Flag for calo only cut-base
  bool m_caloOnly;
  float m_trigEtTh;

}; // End: class definition

#endif

