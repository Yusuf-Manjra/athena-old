/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// CorrectPFOTool.cxx

#include "JetRecTools/CorrectPFOTool.h"
#include "PFlowUtils/FEHelpers.h"
#include "AsgDataHandles/ReadHandle.h"

#include <cmath>

CorrectPFOTool::CorrectPFOTool(const std::string &name):
  JetConstituentModifierBase(name), 
  m_weightPFOTool("",this) {

  // Configuration
  declareProperty("WeightPFOTool",   m_weightPFOTool,     "Name of tool that extracts the cPFO weights.");
  declareProperty("InputIsEM",       m_inputIsEM = true,  "True if neutral PFOs are EM scale clusters.");
  declareProperty("CalibratePFO",    m_calibrate = false, "True if LC calibration should be applied to EM PFOs.");
  declareProperty("CorrectNeutral",      m_correctneutral = true, "True to use the neutral component of PFlow.");
  declareProperty("CorrectCharged",      m_correctcharged = true, "True if use the charged component of PFlow.");
  declareProperty("UseChargedWeights",m_useChargedWeights = true, "True if we make use of weighting scheme for charged PFO");

  // Input properties
  declareProperty("VertexContainerKey", 
                  m_vertexContainer_key="PrimaryVertices",
                  "Datahandle key for the primary vertex container");
}

StatusCode CorrectPFOTool::initialize() {
  if(!m_correctneutral && !m_correctcharged) {
    ATH_MSG_ERROR("This tool is configured to do nothing!");
    return StatusCode::FAILURE;
  }
  if(m_inputType!=xAOD::Type::ParticleFlow && m_inputType!=xAOD::Type::FlowElement) {
    ATH_MSG_ERROR("CorrectPFOTool requires PFO inputs. It cannot operate on objects of type "
                  << m_inputType);
    return StatusCode::FAILURE;
  }
  if(m_useChargedWeights) {
    ATH_CHECK( m_weightPFOTool.retrieve() );
  }
  ATH_CHECK( m_vertexContainer_key.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::process_impl(xAOD::IParticleContainer* cont) const {
  // Type-checking happens in the JetConstituentModifierBase class
  // so it is safe just to static_cast
  if(m_inputType == xAOD::Type::FlowElement){
    xAOD::FlowElementContainer* feCont = static_cast<xAOD::FlowElementContainer*>(cont);
    if(!feCont->empty() && !(feCont->front()->signalType() & xAOD::FlowElement::PFlow)){
      ATH_MSG_ERROR("CorrectPFOTool received FlowElements that aren't PFOs");
      return StatusCode::FAILURE;
    }
    return correctPFO(*feCont);
  }
  xAOD::PFOContainer* pfoCont = static_cast<xAOD::PFOContainer*> (cont);
  return correctPFO(*pfoCont);
}

const xAOD::Vertex* CorrectPFOTool::getPrimaryVertex() const {
  // Retrieve Primary Vertices
  auto handle = SG::makeHandle(m_vertexContainer_key);
  if (!handle.isValid()){
      ATH_MSG_WARNING(" This event has no primary vertex container" );
      return nullptr;
  }
    
  const xAOD::VertexContainer* pvtxs = handle.cptr();
  if(pvtxs->empty()){
      ATH_MSG_WARNING(" Failed to retrieve valid primary vertex container" );
      return nullptr;
  } 

  //Usually the 0th vertex is the primary one, but this is not always 
  // the case. So we will choose the first vertex of type PriVtx
  for (auto theVertex : *pvtxs) {
    if (theVertex->vertexType()==xAOD::VxType::PriVtx) {
      return theVertex;
    }//If we have a vertex of type primary vertex
  }//iterate over the vertices and check their type

  // If we failed to find an appropriate vertex, return the dummy vertex
  ATH_MSG_DEBUG("Could not find a primary vertex in this event " );
  for (auto theVertex : *pvtxs) {
    if (theVertex->vertexType()==xAOD::VxType::NoVtx) {
      return theVertex;
    }
  }

  // If there is no primary vertex, then we cannot do PV matching.
  ATH_MSG_WARNING("Primary vertex container was empty");
  return nullptr;
}

StatusCode CorrectPFOTool::correctPFO(xAOD::PFOContainer& cont) const { 

  const xAOD::Vertex* vtx = nullptr;
  if(m_correctneutral) {
    vtx = getPrimaryVertex();
    if(vtx==nullptr) {
      ATH_MSG_ERROR("Primary vertex container was empty or no valid vertex found!");
      return StatusCode::FAILURE;
    } else if (vtx->vertexType()==xAOD::VxType::NoVtx) {
      ATH_MSG_VERBOSE("No genuine primary vertex found. Will not apply origin correction");
    }
  }

  for ( xAOD::PFO* ppfo : cont ) {

    if ( fabs(ppfo->charge())<FLT_MIN) { // Neutral PFOs
      if(m_correctneutral) {
        ATH_CHECK( applyNeutralCorrection(*ppfo, *vtx) );
      }
    } else { // Charged PFOs
      if(m_correctcharged) {
        ATH_CHECK( applyChargedCorrection(*ppfo) );
      }      
    }
  } // PFO loop

  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::correctPFO(xAOD::FlowElementContainer& cont) const { 

  const xAOD::Vertex* vtx = nullptr;
  if(m_correctneutral) {
    vtx = getPrimaryVertex();
    if(vtx==nullptr) {
      ATH_MSG_ERROR("Primary vertex container was empty or no valid vertex found!");
      return StatusCode::FAILURE;
    } else if (vtx->vertexType()==xAOD::VxType::NoVtx) {
      ATH_MSG_VERBOSE("No genuine primary vertex found. Will not apply origin correction");
    }
  }

  for ( xAOD::FlowElement* ppfo : cont ) {

    if ( !ppfo->isCharged()) { // Neutral PFOs
      if(m_correctneutral) {
        ATH_CHECK( applyNeutralCorrection(*ppfo, *vtx) );
      }
    } else { // Charged PFOs
      if(m_correctcharged) {
        ATH_CHECK( applyChargedCorrection(*ppfo) );
      }      
    }
  } // PFO loop

  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::applyNeutralCorrection(xAOD::PFO& pfo, const xAOD::Vertex& vtx) const {
  if (pfo.e() < FLT_MIN) { //This is necessary to avoid changing sign of pT for pT<0 PFO
    pfo.setP4(0,0,0,0);
  } else {
    if ( !m_inputIsEM || m_calibrate ) { // Use LC four-vector
      // Only correct if we really reconstructed a vertex
      if(vtx.vertexType()==xAOD::VxType::PriVtx) {
        pfo.setP4(pfo.GetVertexCorrectedFourVec(vtx));
      }
    } else { // Use EM four-vector
      // Only apply origin correction if we really reconstructed a vertex
      if(vtx.vertexType()==xAOD::VxType::PriVtx) {
        pfo.setP4(pfo.GetVertexCorrectedEMFourVec(vtx));
      } else {pfo.setP4(pfo.p4EM());} // Just set EM 4-vec
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::applyNeutralCorrection(xAOD::FlowElement& pfo, const xAOD::Vertex& vtx) const {
  if (pfo.e() < FLT_MIN) { //This is necessary to avoid changing sign of pT for pT<0 PFO
    pfo.setP4(0,0,0,0);
  }
  // Only apply origin correction if we really reconstructed a vertex
  else if(vtx.vertexType() == xAOD::VxType::PriVtx) {
    pfo.setP4(FEHelpers::getVertexCorrectedFourVec(pfo, vtx));
  }
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::applyChargedCorrection(xAOD::PFO& pfo) const {
  if (m_useChargedWeights) {
    float weight = 0.0;
    ATH_CHECK( m_weightPFOTool->fillWeight( pfo, weight ) );
    ATH_MSG_VERBOSE("Fill pseudojet for CPFO with weighted pt: " << pfo.pt()*weight);
    pfo.setP4(pfo.p4()*weight);
  }//if should use charged PFO weighting scheme
  return StatusCode::SUCCESS;
}

StatusCode CorrectPFOTool::applyChargedCorrection(xAOD::FlowElement& pfo) const {
  if (m_useChargedWeights) {
    float weight = 0.0;
    ATH_CHECK( m_weightPFOTool->fillWeight( pfo, weight ) );
    ATH_MSG_VERBOSE("Fill pseudojet for CPFO with weighted pt: " << pfo.pt()*weight);
    pfo.setP4(pfo.p4()*weight);
  }//if should use charged PFO weighting scheme
  return StatusCode::SUCCESS;
}
