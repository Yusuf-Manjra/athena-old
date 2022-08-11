/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIGLOBAL_HIEVENTSHAPEFILLERTOOL_H
#define HIGLOBAL_HIEVENTSHAPEFILLERTOOL_H

#include "AsgTools/AsgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "HIGlobal/IHIEventShapeFiller.h"
#include "xAODHIEvent/HIEventShapeContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "HIEventUtils/HIEventShapeIndex.h"
#include "HIEventUtils/HITowerWeightTool.h"
#include "HIEventUtils/HIEventShapeMapTool.h"
#include <NavFourMom/INavigable4MomentumCollection.h>

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

class CaloCellContainer;

class HIEventShapeFillerTool : public asg::AsgTool, virtual public IHIEventShapeFiller
{
  ASG_TOOL_CLASS(HIEventShapeFillerTool, IHIEventShapeFiller)

public:
  HIEventShapeFillerTool(const std::string& myname);

  virtual StatusCode initializeCollection(xAOD::HIEventShapeContainer* evtShape_) override;
  virtual StatusCode fillCollectionFromTowers(const SG::ReadHandleKey<xAOD::CaloClusterContainer>& tower_container_key,
    const SG::ReadHandleKey<INavigable4MomentumCollection>& navi_container_key) override;
  virtual StatusCode fillCollectionFromCells(const SG::ReadHandleKey<CaloCellContainer>& cell_container_key) override;

  virtual StatusCode fillCollectionFromTowerContainer(const INavigable4MomentumCollection* navInColl);
  virtual StatusCode fillCollectionFromCellContainer(const CaloCellContainer* CellContainer);
  virtual StatusCode fillCollectionFromClusterContainer(const xAOD::CaloClusterContainer* theClusters);
  virtual const xAOD::HIEventShapeContainer* getHIEventShapeContainer() const override { return m_evtShape; }

private:
  xAOD::HIEventShapeContainer* m_evtShape;


  const HIEventShapeIndex* m_index;

  void updateShape(xAOD::HIEventShapeContainer* shape, const HIEventShapeIndex* index, const CaloCell* theCell, float geoWeight, float eta0, float phi0, bool isNeg = false) const;

  ToolHandle<IHITowerWeightTool>   m_towerWeightTool{ this, "TowerWeightTool", "HITowerWeightTool", "Handle to Tower Weight Tool" };
  ToolHandle<IHIEventShapeMapTool> m_eventShapeMapTool{ this, "EventShapeMapTool", "HIEventShapeMapTool", "Handle to Event Shape Map Tool" };
  Gaudi::Property< bool >          m_useClusters{ this, "UseClusters", false, "use Clusters boolean switch" };
  Gaudi::Property< int >           m_numOrders{ this, "OrderOfFlowHarmonics", 7, "The number of Orders of harmonic flow to store in the EventShape" };
};

#endif
