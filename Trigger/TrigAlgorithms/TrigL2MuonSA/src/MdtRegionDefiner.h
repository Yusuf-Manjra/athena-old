/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_MDTREGIONDEFINER_H
#define  TRIGL2MUONSA_MDTREGIONDEFINER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include "TgcFit.h"
#include "TgcData.h"
#include "RpcData.h"
#include "TgcFitResult.h"
#include "RpcFitResult.h"
#include "MuonRoad.h"
#include "MdtRegion.h"

#include <string>

namespace MuonGM {
     class MdtReadoutElement;
     class MuonStation;
}

namespace TrigL2MuonSA {


// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

  class MdtRegionDefiner: public AthAlgTool
  {
  public:

    MdtRegionDefiner(const std::string& type, 
		     const std::string& name,
		     const IInterface*  parent);
    
    virtual StatusCode initialize() override;

    void setRpcGeometry(bool use_rpc){m_use_rpc = use_rpc;};
    
  public:
    StatusCode getMdtRegions(const TrigRoiDescriptor*          p_roids,
			     const TrigL2MuonSA::RpcFitResult& rpcFitResult,
			     TrigL2MuonSA::MuonRoad&           muonRoad,
			     TrigL2MuonSA::MdtRegion&          mdtRegion) const;
    
    StatusCode getMdtRegions(const TrigRoiDescriptor*          p_roids,
			     const TrigL2MuonSA::TgcFitResult& tgcFitResult,
			     TrigL2MuonSA::MuonRoad&           muonRoad,
			     TrigL2MuonSA::MdtRegion&          mdtRegion) const;
    
  private:
    StatusCode prepareTgcPoints(const TrigL2MuonSA::TgcHits& tgcHits);
    void find_barrel_road_dim(float max_road, float aw, float bw,
			      float rMmin,float rMax,float *zMin,float *zMax) const;
    void find_endcap_road_dim(float road,float aw, float bw, float zMin,
			      float zMax,float *rMin,float *rMax) const;
    void find_eta_min_max(float zMin, float rMin,
			  float zMax, float rMax,
			  float& etaMin, float& etaMax) const;
    void find_phi_min_max(float phiMiddle, float& phiMin, float& phiMax) const;
        
    void find_station_sector(std::string name, int phi, bool& endcap, int& chamber, int& sector) const;

    StatusCode computePhi(const TrigRoiDescriptor*          p_roids,
			  const TrigL2MuonSA::RpcFitResult& rpcFitResult,
			  const TrigL2MuonSA::MdtRegion&    mdtRegion,
			  TrigL2MuonSA::MuonRoad&           muonRoad) const;
      
    StatusCode computePhi(const TrigRoiDescriptor*          p_roids,
			  const TrigL2MuonSA::TgcFitResult& tgcFitResult,
			  const TrigL2MuonSA::MdtRegion&    mdtRegion,
			  TrigL2MuonSA::MuonRoad&           muonRoad) const;

  private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey {this, "DetectorManagerKey", "MuonDetectorManager", "Key of input MuonDetectorManager condition data"}; 

    bool m_use_rpc {true};

    TrigL2MuonSA::TgcFit::PointArray m_tgcStripMidPoints;  // List of TGC strip middle station points.
    TrigL2MuonSA::TgcFit::PointArray m_tgcWireMidPoints;   // List of TGC wire middle station points.
    TrigL2MuonSA::TgcFit::PointArray m_tgcStripInnPoints;  // List of TGC strip inner station points.
    TrigL2MuonSA::TgcFit::PointArray m_tgcWireInnPoints;   // List of TGC wire inner station points.

  };
  
  // --------------------------------------------------------------------------------
  // --------------------------------------------------------------------------------
}

#endif // TRIGL2MUONSA_TESTCLASS_H
