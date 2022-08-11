/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigEgammaForwardPrecisionElectronHypoAlg.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "AthViews/ViewHelper.h"
#include "AthenaMonitoringKernel/Monitored.h"

namespace TCU = TrigCompositeUtils;

TrigEgammaForwardPrecisionElectronHypoAlg::TrigEgammaForwardPrecisionElectronHypoAlg( const std::string& name, ISvcLocator* pSvcLocator ) :
  ::HypoBase( name, pSvcLocator ) {}


StatusCode TrigEgammaForwardPrecisionElectronHypoAlg::initialize() 
{

  ATH_MSG_DEBUG ( "Initializing " << name() << "..." );

  ATH_CHECK( m_hypoTools.retrieve() );

  // Now we try to retrieve the ElectronPhotonSelectorTools that we will use to apply the electron Identification. This is a *must*
  ATH_MSG_DEBUG( "Retrieving egammaElectronLHTool..."  );
  ATH_CHECK(m_egammaElectronLHTools.retrieve());

  // Retrieving Luminosity info
  ATH_MSG_DEBUG( "Retrieving luminosityCondData..."  );
  ATH_CHECK( m_avgMuKey.initialize() );
  ATH_CHECK( m_electronsKey.initialize() );
  renounce( m_electronsKey );// electrons are made in views, so they are not in the EvtStore: hide them
  if (! m_monTool.empty() ) ATH_CHECK( m_monTool.retrieve() );
  return StatusCode::SUCCESS;
}


StatusCode TrigEgammaForwardPrecisionElectronHypoAlg::execute( const EventContext& context ) const 
{  

  ATH_MSG_DEBUG ( "Executing " << name() << "..." );

  auto timer = Monitored::Timer("TIME_exec");
  auto timer_lh = Monitored::Timer("TIME_LH_exec");
  auto monitoring = Monitored::Group( m_monTool, timer, timer_lh);


  timer.start();


  auto previousDecisionsHandle = SG::makeHandle( decisionInput(), context );
  ATH_CHECK( previousDecisionsHandle.isValid() );
  ATH_MSG_DEBUG( "Running with "<< previousDecisionsHandle->size() <<" previous decisions");


  // new decisions
  // new output decisions
  SG::WriteHandle<TCU::DecisionContainer> outputHandle = TCU::createAndStore(decisionOutput(), context );
  auto decisions = outputHandle.ptr();

  // input for decision
  std::vector<ITrigEgammaForwardPrecisionElectronHypoTool::ElectronInfo> toolInput;

  // loop over previous decisions
  size_t counter=0;
  for ( auto previousDecision: *previousDecisionsHandle ) {

    //get updated RoI  
    auto roiELInfo = TCU::findLink<TrigRoiDescriptorCollection>( previousDecision, TCU::roiString() );

    ATH_CHECK( roiELInfo.isValid() );
    const TrigRoiDescriptor* roi = *(roiELInfo.link);
    const auto viewEL = previousDecision->objectLink<ViewContainer>( TCU::viewString() );
    ATH_CHECK( viewEL.isValid() );
    auto electronHandle = ViewHelper::makeHandle( *viewEL, m_electronsKey, context);
    ATH_CHECK( electronHandle.isValid() );
    ATH_MSG_DEBUG ( "Electron handle size: " << electronHandle->size() << "..." );
    // Loop over the electronHandles
    size_t validelectrons=0;
    for (size_t cl=0; cl< electronHandle->size(); cl++){
      
      {
        auto el = ViewHelper::makeLink( *viewEL, electronHandle, cl );
        ATH_MSG_DEBUG ( "Checking ph.isValid()...");
        if( !el.isValid() ) {
          ATH_MSG_DEBUG ( "ElectronHandle in position " << cl << " -> invalid ElemntLink!. Skipping...");
        }

        ATH_CHECK(el.isValid());
        ATH_MSG_DEBUG ( "ElectronHandle in position " << cl << " processing...");
        auto d = TCU::newDecisionIn( decisions, TCU::hypoAlgNodeName() );
        d->setObjectLink( TCU::featureString(),  el );
        TCU::linkToPrevious( d, decisionInput().key(), counter );

        // create the info
        ITrigEgammaForwardPrecisionElectronHypoTool::ElectronInfo info(d, roi, electronHandle.cptr()->at(cl), previousDecision);

        // Retrieve avgmu value from event info
        SG::ReadDecorHandle<xAOD::EventInfo,float> eventInfoDecor(m_avgMuKey, context);

        if(eventInfoDecor.isPresent()){
          float avg_mu = eventInfoDecor(0);
          ATH_MSG_DEBUG("Average mu " << avg_mu);
          info.valueDecorator["avgmu"] = avg_mu;
        }

        // Decorate the info with all LH decisions
        int idx=0;
        for ( auto &pidname : m_lhNames ){
          timer_lh.start();
          if(eventInfoDecor.isPresent()) {
             float avg_mu = eventInfoDecor(0);
             float lhvalue = m_egammaElectronLHTools[idx]->calculate(context, electronHandle.cptr()->at(cl),avg_mu);
             info.valueDecorator[pidname+"LHValue"] = lhvalue;
             info.pidDecorator[pidname] = (bool)m_egammaElectronLHTools[idx]->accept(context, electronHandle.cptr()->at(cl),avg_mu);
          }else{
            float lhvalue = m_egammaElectronLHTools[idx]->calculate(context, electronHandle.cptr()->at(cl));
            info.valueDecorator[pidname+"LHValue"] = lhvalue;
            ATH_MSG_WARNING("EventInfo decoration not available!");
            info.pidDecorator[pidname] = (bool)m_egammaElectronLHTools[idx]->accept(context, electronHandle.cptr()->at(cl));
          }
          timer_lh.stop();
          idx++;
        }

        toolInput.push_back( info );
        validelectrons++;
      }
    }

    ATH_MSG_DEBUG( "Electrons with valid links: " << validelectrons );
    ATH_MSG_DEBUG( "roi, electron, previous decision to new decision " << counter << " for roi " );
    counter++;
  }


  ATH_MSG_DEBUG( "Found "<<toolInput.size()<<" inputs to tools");
  for ( auto& tool: m_hypoTools ) {
    ATH_CHECK( tool->decide( toolInput ) );
  }
 
  ATH_CHECK( hypoBaseOutputProcessing(outputHandle) );
  return StatusCode::SUCCESS;
}
