/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
//
// TrkToLeptonPVTool.h - Description
//
/*
   Tool to match a track to a vertex assumed to be a Primary Vertex.
   This PV can be taken elsewhere or obtained with the tool itsels.
   Example:  check compatibility of a track to PV obtained with 2 energetic leptons from Z decay.
   See README fro more details

    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch
*/
#ifndef TrkToLeptonPVTool_H
#define TrkToLeptonPVTool_H

#include <vector>
#include <utility>
#include <memory>
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TrkToLeptonPVTool/ITrkToLeptonPV.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
//


  class TrkToLeptonPVTool : public AthAlgTool, virtual public ITrkToLeptonPV
  {

   public:
      TrkToLeptonPVTool(const std::string& type, const std::string& name, const IInterface* parent);
      virtual ~TrkToLeptonPVTool();


      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;


      virtual std::unique_ptr<xAOD::Vertex> matchTrkToPV(const xAOD::TrackParticle *trk, const xAOD::Vertex * PV, const xAOD::EventInfo * =0) const override final;
      virtual std::unique_ptr<xAOD::Vertex> npartVertex(const std::vector<const xAOD::TrackParticle*>&, const xAOD::EventInfo * =0) const override final;
//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

   private:

    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };
    ToolHandle< Trk::IVertexFitter >    m_fitterSvc{this,"VertexFitter","Trk::TrkVKalVrtFitter/VertexFitterTool","Vertex Fitter tool instance"};

 };


#endif
