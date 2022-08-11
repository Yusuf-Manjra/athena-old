/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGEGAMMAHYPO_TRIGPRECISIONCALOHYPOALG_PRECISION_H
#define TRIGEGAMMAHYPO_TRIGPRECISIONCALOHYPOALG_PRECISION_H 1

#include <string>

#include "AthViews/View.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "DecisionHandling/HypoBase.h"

#include "ITrigEgammaPrecisionCaloHypoTool.h"

/**
 * @class TrigEgammaPrecisionCaloHypoAlg
 * @brief Implements egamma calo selection for the new HLT framework
 **/
class TrigEgammaPrecisionCaloHypoAlg : public ::HypoBase {
 public: 

  TrigEgammaPrecisionCaloHypoAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute( const EventContext& context ) const override;

 private: 
  ToolHandleArray< ITrigEgammaPrecisionCaloHypoTool > m_hypoTools { this, "HypoTools", {}, "Hypo tools" };
     
  SG::ReadHandleKey< xAOD::CaloClusterContainer > m_clustersKey { this, "CaloClusters", "CaloClusters", "CaloClusters in roi" };  
  

}; 

#endif //> !TRIGEGAMMAHYPO_TESTTRIGPRECISIONCALOHYPOALG_H
