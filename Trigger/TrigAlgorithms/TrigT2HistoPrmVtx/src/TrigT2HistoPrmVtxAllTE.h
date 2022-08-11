/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// ************************************************
//
// NAME:     TrigT2HistoPrmVtxAllTE.h
// PACKAGE:  Trigger/TrigAlgorithms/TrigT2HistoPrmVtx
//
// AUTHOR:   Viviana Cavaliere
// EMAIL:    viviana.cavaliere@cern.ch
// 
// ************************************************

#ifndef TRIGT2HISTOPRMVTX_TRIGT2HISTOPRMVTXALLTE_H
#define TRIGT2HISTOPRMVTX_TRIGT2HISTOPRMVTXALLTE_H

#include "TrigT2HistoPrmVtxBase.h"
#include "HistoVertexHelper.h"

#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "BeamSpotConditionsData/BeamSpotData.h"

/**
 * @brief Class for a fast primary vertex reconstruction in the HLT framework.
 * This is the base class for the primary vertex reconstruction at LVL2.
 */

class TrigT2HistoPrmVtxAllTE : public TrigT2HistoPrmVtxBase {
 public:
  TrigT2HistoPrmVtxAllTE(const std::string&, ISvcLocator*);

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

 private:
  unsigned int getTrackNumbers(const xAOD::TrackParticleContainer*);

 private:
  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };

  SG::ReadHandleKey< TrigRoiDescriptorCollection > m_inputRoIContainerKey {this,"InputRoIsKey","Undefined","Input RoIs Key"};
  SG::ReadHandleKey< xAOD::TrackParticleContainer > m_inputTracksContainerKey {this,"InputTracksKey","Undefined","Input Tracks Key"};

  SG::WriteHandleKey< xAOD::VertexContainer > m_outputVertexContainerKey {this,"OutputVertexKey","EFHistoPrmVtx","Output Vertex Container Key"}; 

 private:
  std::unique_ptr< HistoVertexHelper > m_hisVtx;
};
 
#endif 
