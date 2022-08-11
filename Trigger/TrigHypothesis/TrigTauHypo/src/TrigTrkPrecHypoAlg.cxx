/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "Gaudi/Property.h"
#include "TrigTrkPrecHypoAlg.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "AthViews/ViewHelper.h"

using namespace TrigCompositeUtils;

TrigTrkPrecHypoAlg::TrigTrkPrecHypoAlg( const std::string& name, 
				      ISvcLocator* pSvcLocator ) :
  ::HypoBase( name, pSvcLocator ) {}


StatusCode TrigTrkPrecHypoAlg::initialize() {
  ATH_CHECK( m_hypoTools.retrieve() );
  ATH_CHECK( m_fastTracksKey.initialize() );
  ATH_CHECK( m_roiForID2ReadKey.initialize(SG::AllowEmpty) );
  renounce( m_fastTracksKey );// tau candidates are made in views, so they are not in the EvtStore: hide them
  renounce( m_roiForID2ReadKey);

  return StatusCode::SUCCESS;
}

StatusCode TrigTrkPrecHypoAlg::execute( const EventContext& context ) const {
  ATH_MSG_DEBUG ( "Executing " << name() << "..." );
  auto previousDecisionsHandle = SG::makeHandle( decisionInput(), context );
  if( not previousDecisionsHandle.isValid() ) {//implicit
    ATH_MSG_DEBUG( "No implicit RH for previous decisions "<<  decisionInput().key()<<": is this expected?" );
    return StatusCode::SUCCESS;      
  }
  
  ATH_MSG_DEBUG( "Running with "<< previousDecisionsHandle->size() <<" previous decisions");

  // new decisions

  // new output decisions
  SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), context ); 
  auto decisions = outputHandle.ptr();

  // input for decision
  std::vector<ITrigTrkPrecHypoTool::TrackingInfo> toolInput;

  // loop over previous decisions
  int counter=-1;
  for ( auto previousDecision: *previousDecisionsHandle ) {
    counter++;

    //get View
    const auto viewEL = previousDecision->objectLink<ViewContainer>( viewString() );
    ATH_CHECK( viewEL.isValid() );
    auto tracksHandle = ViewHelper::makeHandle( *viewEL, m_fastTracksKey, context);
    ATH_CHECK( tracksHandle.isValid() );
    ATH_MSG_DEBUG ( "tracks handle size: " << tracksHandle->size() << "..." );

    const TrigRoiDescriptor *roi = nullptr;
    //get RoI
    if(m_roiForID2ReadKey.key().empty()) {
       auto roiELInfo = findLink<TrigRoiDescriptorCollection>( previousDecision, initialRoIString());
       ATH_CHECK( roiELInfo.isValid() );
       roi = *(roiELInfo.link);
    }else{
       auto roiHandle = ViewHelper::makeHandle( *viewEL, m_roiForID2ReadKey, context);
       ATH_CHECK( roiHandle.isValid() );    
       if( roiHandle ->size() != 1 ) {
         ATH_MSG_ERROR("Expect exactly one updated ROI");
         return StatusCode::FAILURE;
       }
       roi = roiHandle->at(0);
    }
    // create new decision
    auto d = newDecisionIn( decisions, hypoAlgNodeName() );
    TrigCompositeUtils::linkToPrevious( d, decisionInput().key(), counter );

    if (tracksHandle->size()) {
      auto el = ViewHelper::makeLink( *viewEL, tracksHandle, 0 );
      ATH_CHECK( el.isValid() );
      d->setObjectLink( featureString(),  el );
    } else {
      auto el = TrigCompositeUtils::decisionToElementLink(d, context);
      ATH_CHECK( el.isValid() );
      d->setObjectLink( featureString(),  el );
    }
    
    toolInput.emplace_back( d, roi, tracksHandle.cptr(), previousDecision );

    ATH_MSG_DEBUG( "Added view, roi, tracks, previous decision to new decision " << counter << " for view " << (*viewEL)->name()  );
  }

  ATH_MSG_DEBUG( "Found "<<toolInput.size()<<" inputs to tools");

  for ( auto& tool: m_hypoTools ) {
    ATH_CHECK( tool->decide( toolInput ) );
  }

  ATH_CHECK( hypoBaseOutputProcessing(outputHandle) );
  
  return StatusCode::SUCCESS;
}
