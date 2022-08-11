/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigEgammaPrecisionCaloHypoAlg.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "AthViews/ViewHelper.h"

namespace TCU = TrigCompositeUtils;

TrigEgammaPrecisionCaloHypoAlg::TrigEgammaPrecisionCaloHypoAlg( const std::string& name, 
					  ISvcLocator* pSvcLocator ) :
  ::HypoBase( name, pSvcLocator ) {}


StatusCode TrigEgammaPrecisionCaloHypoAlg::initialize() {
  ATH_CHECK( m_hypoTools.retrieve() );
  
  ATH_CHECK( m_clustersKey.initialize() );
  renounce( m_clustersKey );// clusters are made in views, so they are not in the EvtStore: hide them

  return StatusCode::SUCCESS;
}

StatusCode TrigEgammaPrecisionCaloHypoAlg::execute( const EventContext& context ) const {
  ATH_MSG_DEBUG ( "Executing " << name() << "..." );
  auto previousDecisionsHandle = SG::makeHandle( decisionInput(), context );
  ATH_CHECK( previousDecisionsHandle.isValid() );
  ATH_MSG_DEBUG( "Running with "<< previousDecisionsHandle->size() <<" previous decisions");


  // new decisions

  // new output decisions
  SG::WriteHandle<TCU::DecisionContainer> outputHandle = TCU::createAndStore(decisionOutput(), context );
  auto decisions = outputHandle.ptr();

  // input for decision
  std::vector<ITrigEgammaPrecisionCaloHypoTool::ClusterInfo> toolInput;

  // loop over previous decisions
  size_t counter=0;
  for ( auto previousDecision: *previousDecisionsHandle ) {

    //get updated RoI  
    auto roiELInfo = TCU::findLink<TrigRoiDescriptorCollection>( previousDecision, TCU::roiString() );
      
    ATH_CHECK( roiELInfo.isValid() );
    const TrigRoiDescriptor* roi = *(roiELInfo.link);

    // not using View so commenting out following lines
    const auto viewEL = previousDecision->objectLink<ViewContainer>( TCU::viewString() );
    ATH_CHECK( viewEL.isValid() );
    auto clusterHandle = ViewHelper::makeHandle( *(viewEL), m_clustersKey, context);
    ATH_CHECK( clusterHandle.isValid() );
    ATH_MSG_DEBUG ( "Cluster handle size: " << clusterHandle->size() << "..." );
    // Loop over the clusterHandles
    size_t validclusters=0;
    for (size_t cl=0; cl< clusterHandle->size(); cl++){
	{
	    auto el = ViewHelper::makeLink( *(viewEL), clusterHandle, cl );
	    ATH_MSG_DEBUG ( "Checking el.isValid()...");
	    if( !el.isValid() ) {
		ATH_MSG_DEBUG ( "ClusterHandle in position " << cl << " -> invalid ElemntLink!. Skipping...");
	    }
	    ATH_CHECK(el.isValid());

	    ATH_MSG_DEBUG ( "ClusterHandle in position " << cl << " processing...");
	    auto d = TCU::newDecisionIn( decisions, TCU::hypoAlgNodeName() );
	    d->setObjectLink( TCU::featureString(),  el );
	    TCU::linkToPrevious( d, decisionInput().key(), counter );
	    toolInput.emplace_back( d, roi, clusterHandle.cptr()->at(cl), previousDecision );
	    validclusters++;


	}
    }
    ATH_MSG_DEBUG( "Clusters with valid links: " << validclusters );
    
    ATH_MSG_DEBUG( "roi, cluster, previous decision to new decision " << counter << " for roi " );
    counter++;

  }

  ATH_MSG_DEBUG( "Found "<<toolInput.size()<<" inputs to tools");

   
  for ( auto& tool: m_hypoTools ) {
    ATH_CHECK( tool->decide( toolInput ) );
  }
 
  ATH_CHECK( hypoBaseOutputProcessing(outputHandle) );
  return StatusCode::SUCCESS;
}
