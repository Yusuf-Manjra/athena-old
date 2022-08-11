/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#ifndef EVGENPRODTOOLS_COPYEVENTWEIGHT_H
#define EVGENPRODTOOLS_COPYEVENTWEIGHT_H

#include "GeneratorModules/GenBase.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

/**
 * @class CopyEventWeight
 * @brief Copy MC truth event weights into the event info store
 *
 * @author Paolo Calafiura <pcalafiura@lbl.gov>
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 * @author Andy Buckley <andy.buckley@cern.ch>
 *
 * @note Only the weights for the first (signal, by convention)
 *       event are copied. Others are assumed to be pile-up events and
 *       are ignored by this algorithm.
 **/

class CopyEventWeight : public GenBase {
public:

  /// Constructor
  CopyEventWeight(const std::string& name, ISvcLocator* svcLoc);

  /// Copy the weight vector for each event
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

private:

  SG::WriteDecorHandleKey<xAOD::EventInfo> m_mcWeightsKey {this
      , "mcEventWeightsKey"
      , "TMPEvtInfo.mcEventWeights"
      , "Decoration for MC Event Weights"};
};


#endif

#endif
