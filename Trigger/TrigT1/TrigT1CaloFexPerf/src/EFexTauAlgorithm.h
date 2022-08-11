// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

/**
 * Name    : EFexTauAlgorithm.h
 * PACKAGE : Trigger/TrigT1/TrigT1CaloFexPerf
 * AUTHOR  : Denis Oliveira Damazio
 * PURPOSE : emulate the eFex tau algorithm for phase 1 L1Calo
 */

#ifndef LVL1__TRIGT1CALORUN3TAUFEX
#define LVL1__TRIGT1CALORUN3TAUFEX

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "CaloEvent/CaloConstCellContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODTrigger/EmTauRoIContainer.h"

#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"

namespace LVL1
{
   class EFexTauAlgorithm : public AthReentrantAlgorithm
   {
   public:
      EFexTauAlgorithm(const std::string &name, ISvcLocator *pSvcLocator);

      virtual ~EFexTauAlgorithm();

      StatusCode initialize() override;
      StatusCode execute(const EventContext& ctx) const override;

   private:
      /**
       * member functions
       */

      /** calculate the ET of an input cell */
      float CaloCellET(const CaloCell *const &inputCell, float digitScale, float digitThreshold) const;

      /**
       * input / output
       */
      SG::ReadHandleKey<CaloCellContainer> m_inputCellContainerKey;                   ///< LAr SuperCell input container
      SG::ReadHandleKey<CaloCellContainer> m_inputTileCellContainerKey;               ///< Tile cell input container
      SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_inputTriggerTowerContainerKey; ///< TriggerTowers (if needed)
      SG::WriteHandleKey<xAOD::EmTauRoIContainer> m_outputClusterName;

      /**
       * properties
       */
      bool m_use_tileCells; ///< boolean for using Tile cells instead of Tile TT

      bool m_useProvenanceSkim; ///< clear up container from bad BC by making a new container (Denis, old way)
      bool m_useProvenance;     ///<  clear up container from bad BC by skipping scells
      int m_qualBitMask;        ///< Configurable quality bitmask

      float m_nominalDigitization; ///< value of nominal digitisation
      float m_nominalNoise_thresh; ///< noise threshold
      float m_timeThr;
   };
} // namespace LVL1
#endif
