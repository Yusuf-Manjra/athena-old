/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_ITRIGJETHYPOTOOLHELPER_H
#define TRIGHLTJETHYPO_ITRIGJETHYPOTOOLHELPER_H

#include "GaudiKernel/IAlgTool.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"

class ITrigJetHypoInfoCollector;
class xAODJetCollector;

class ITrigJetHypoToolHelper : virtual public ::IAlgTool {

public:
  DeclareInterfaceID(ITrigJetHypoToolHelper, 1, 0);
  virtual ~ITrigJetHypoToolHelper(){};
  // make a decision (yes/no) based on the jets in the jet container,
  // set the new decision object according to this decision, and previous
  // devicsion objects. Jets typically have only a decision object
  // generated at L1.

  virtual bool pass(HypoJetVector& jets,
		    xAODJetCollector&,
                    const std::unique_ptr<ITrigJetHypoInfoCollector>&) const = 0;
  
  virtual std::size_t requiresNJets() const = 0;
  virtual StatusCode getDescription(ITrigJetHypoInfoCollector&) const = 0;
  virtual std::string toString() const = 0;
   
};
#endif
