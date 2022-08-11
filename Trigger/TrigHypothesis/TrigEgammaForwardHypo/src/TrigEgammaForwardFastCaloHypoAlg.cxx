/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigEgammaForwardFastCaloHypoAlg.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "AthViews/ViewHelper.h"
#include "xAODTrigger/TrigCompositeContainer.h"

namespace TCU = TrigCompositeUtils;

TrigEgammaForwardFastCaloHypoAlg::TrigEgammaForwardFastCaloHypoAlg( const std::string& name, 
					  ISvcLocator* pSvcLocator ) :
  ::HypoBase( name, pSvcLocator ) {}


StatusCode TrigEgammaForwardFastCaloHypoAlg::initialize() {

  ATH_CHECK( m_hypoTools.retrieve() );
  
  ATH_CHECK( m_clustersKey.initialize() );
  ATH_CHECK( m_ringsKey.initialize(SG::AllowEmpty));
  
  if ( not m_monTool.name().empty() ) 
    ATH_CHECK( m_monTool.retrieve() );
    
  renounce( m_clustersKey );// clusters are made in views, so they are not in the EvtStore: hide them
  renounce( m_ringsKey );

  return StatusCode::SUCCESS;
}


StatusCode TrigEgammaForwardFastCaloHypoAlg::execute( const EventContext& context ) const 
{
  
    
  ATH_MSG_DEBUG ( "Executing " << name() << "..." );
  auto previousDecisionsHandle = SG::makeHandle( decisionInput(), context );
  ATH_CHECK( previousDecisionsHandle.isValid() );
  ATH_MSG_DEBUG( "Running with "<< previousDecisionsHandle->size() <<" previous decisions");


  // new decisions

  // new output decisions
  SG::WriteHandle<TCU::DecisionContainer> outputHandle = TCU::createAndStore(decisionOutput(), context );
  auto decisions = outputHandle.ptr();

  // input for decision
  std::vector<ITrigEgammaForwardFastCaloHypoTool::FastClusterInfo> toolInput;

  // loop over previous decisions
  size_t counter=0;
  for ( const auto previousDecision: *previousDecisionsHandle ) {
    //get RoI  
    auto roiELInfo = TCU::findLink<TrigRoiDescriptorCollection>( previousDecision, TCU::initialRoIString() );
    
    ATH_CHECK( roiELInfo.isValid() );
    const TrigRoiDescriptor* roi = *(roiELInfo.link);

    // get View
    const auto viewEL = previousDecision->objectLink<ViewContainer>( TCU::viewString() );
    ATH_CHECK( viewEL.isValid() );

    // get cluster
    auto clusterHandle = ViewHelper::makeHandle( *viewEL, m_clustersKey, context);
    ATH_CHECK( clusterHandle.isValid() );
    ATH_MSG_DEBUG ( "Cluster handle size: " << clusterHandle->size() << "..." );

    auto d = TCU::newDecisionIn( decisions, TCU::hypoAlgNodeName() );
    

    // get Rings
    const xAOD::TrigRingerRingsContainer* rings = nullptr;    
    /*
    if ( not m_ringsKey.empty() ) {      
      auto ringerShapeHandle = ViewHelper::makeHandle( *viewEL, m_ringsKey, context);      
      ATH_CHECK( ringerShapeHandle.isValid());
      rings = ringerShapeHandle.cptr();	
      ATH_MSG_DEBUG ( "Ringer handle size: " << ringerShapeHandle->size() << "..." );

      // link the rings      
      auto el = ViewHelper::makeLink( *viewEL, ringerShapeHandle, 0 );
      ATH_CHECK( el.isValid() );
      d->setObjectLink( "ringer",  el );
    }*/

    // create new decision
    toolInput.emplace_back( d, roi, clusterHandle.cptr()->at(0), (rings ? rings->at(0) : nullptr) , previousDecision );
    

    // link the cluster
    { 
      auto clus = ViewHelper::makeLink( *viewEL, clusterHandle, 0 );
      ATH_CHECK( clus.isValid() );
      d->setObjectLink( TCU::featureString(),  clus );
    }
    
    d->setObjectLink( TCU::roiString(), roiELInfo.link );
    
    TCU::linkToPrevious( d, previousDecision, context );
    ATH_MSG_DEBUG( "Added view, roi, cluster, previous decision to new decision " << counter << " for view " << (*viewEL)->name()  );
    counter++;

  }

  ATH_MSG_DEBUG( "Found "<<toolInput.size()<<" inputs to tools");

   
  for ( auto& tool: m_hypoTools ) {
    ATH_CHECK( tool->decide( toolInput ) );
  }
 
  ATH_CHECK( hypoBaseOutputProcessing(outputHandle) );

  return StatusCode::SUCCESS;
}
