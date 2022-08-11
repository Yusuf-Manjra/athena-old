/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1CALOFEXPERF_GTOWERSFROMGCALOTOWERS_H
#define TRIGT1CALOFEXPERF_GTOWERSFROMGCALOTOWERS_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrigT1CaloFexPerf/JGTowerMappingData.h"
#include "xAODTrigL1Calo/JGTowerContainer.h"
#include "CaloIdentifier/GTower_ID.h"

namespace LVL1
{
  class GTowersFromGCaloTowers : public AthReentrantAlgorithm
  {
  public:
    GTowersFromGCaloTowers(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~GTowersFromGCaloTowers() override;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::JGTowerContainer> m_inputKey;
    SG::WriteHandleKey<xAOD::JGTowerContainer> m_outputKey;
    SG::ReadCondHandleKey<JGTowerMappingData> m_mappingKey{
        this, "MappingData", "GTowerMappingData"};
    bool m_mergeEdgeTowers;

    const GTower_ID *m_gTowerID{nullptr};
  }; //> end class GTowersFromGCaloTowers
} // namespace LVL1

#endif //> !TRIGT1CALOFEXPERF_GTOWERSFROMGCALOTOWERS_H