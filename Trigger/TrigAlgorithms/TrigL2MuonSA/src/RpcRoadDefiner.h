/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_RPCROADDEFINER_H
#define  TRIGL2MUONSA_RPCROADDEFINER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrigMuonToolInterfaces/ITrigMuonBackExtrapolator.h"
#include "RpcData.h"
#include "RpcPatFinder.h"
#include "MuonRoad.h"
#include "RpcFitResult.h"
#include "BarrelRoadData.h"
#include "TrigT1Interfaces/RecMuonRoI.h"
#include "xAODTrigger/MuonRoI.h"
#include "IRegionSelector/IRegSelTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include <string>

namespace TrigL2MuonSA {

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

class RpcRoadDefiner: public AthAlgTool
{
 public:

  RpcRoadDefiner(const std::string& type,
                 const std::string& name,
                 const IInterface*  parent);

  virtual StatusCode initialize() override;

 public:
  StatusCode defineRoad(const LVL1::RecMuonRoI*             p_roi,
			const bool                          insideOut,
			TrigL2MuonSA::MuonRoad&             muonRoad,
			TrigL2MuonSA::RpcHits&              rpcHits,
                        const TrigL2MuonSA::RpcLayerHits&   rpcLayerHits,
			const ToolHandle<RpcPatFinder>*     rpcPatFinder,
			TrigL2MuonSA::RpcFitResult&         rpcFitResult,
			double                              roiEtaMinLow,
			double                              roiEtaMaxLow,
			double                              roiEtaMinHigh,
			double                              roiEtaMaxHigh) const;

  StatusCode defineRoad(const xAOD::MuonRoI*                p_roi,
			const bool                          insideOut,
			TrigL2MuonSA::MuonRoad&             muonRoad,
			TrigL2MuonSA::RpcHits&              rpcHits,
                        const TrigL2MuonSA::RpcLayerHits&   rpcLayerHits,
			const ToolHandle<RpcPatFinder>*     rpcPatFinder,
			TrigL2MuonSA::RpcFitResult&         rpcFitResult,
			double                              roiEtaMinLow,
			double                              roiEtaMaxLow,
			double                              roiEtaMinHigh,
			double                              roiEtaMaxHigh) const;

  void setRoadWidthForFailure(double rWidth_RPC_Failed){ m_rWidth_RPC_Failed = rWidth_RPC_Failed; };
  void setRpcGeometry(bool use_rpc){ m_use_rpc = use_rpc; };

 protected:
  float f(float x, float c0, float c1, float c2, float c3) const;
  float fp(float x, float c33, float c22, float c1) const;

 private:
  double m_rWidth_RPC_Failed{0};
  bool m_use_rpc{true};

  ToolHandle<IRegSelTool> m_regionSelector{this, "RegionSelectionTool", "RegSelTool/RegSelTool_MDT", "MDT Region Selector Tool"};
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

 inline float TrigL2MuonSA::RpcRoadDefiner::f(float x, float c0, float c1, float c2, float c3) const
   {
     return c0 + x * (c1 + x * (c2 + x * c3)); // faster
   }

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
 inline float TrigL2MuonSA::RpcRoadDefiner::fp(float x, float c33, float c22, float c1) const
   {
     return c1 + x * (c22 + x * c33); // faster
   }

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
}

#endif // TRIGL2MUONSA_RPCROADDEFINER_H
