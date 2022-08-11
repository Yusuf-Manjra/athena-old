/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/** 
 *  @brief The LumiBlockMetaDataTool reads luminosity metadata from input files and transfers it to output files
 *  @author Marjorie Shapiro <mdshapiro@lbl.gov> based on work from Peter van Gemmeren <gemmeren@anl.gov> 
 **/

#include "LumiBlockMetaDataTool.h"
#include "GoodRunsLists/IGoodRunsListSelectorTool.h"
#include "GoodRunsLists/TGoodRunsListReader.h"
#include "xAODLuminosity/SortLumiBlockRangeByStart.h"

#include "GoodRunsLists/TGoodRunsList.h"

#include "TROOT.h"

#include <algorithm>

//___________________________________________________________________________
LumiBlockMetaDataTool::LumiBlockMetaDataTool(const std::string& type
					     , const std::string& name
					     , const IInterface* parent)
  : AthAlgTool(type, name, parent)
  , m_pMetaDataStore ("StoreGateSvc/MetaDataStore", name)
  , m_pInputStore    ("StoreGateSvc/InputMetaDataStore", name)
  , m_fileCurrentlyOpened(false)
  , m_CurrentFileName("none")
{
  declareInterface<IMetaDataTool>(this);

  declareProperty("LBCollName",m_LBColl_name = "LumiBlocks");
  declareProperty("unfinishedLBCollName",m_unfinishedLBColl_name = "IncompleteLumiBlocks");
  declareProperty("suspectLBCollName", m_suspectLBColl_name="SuspectLumiBlocks");

  // Here is where we create the LumiBlockRange objects.  When we open a 
  // file, we fill the m_cacheInputRangeContainer from the input metadata store.  
  // When the file closes, we move the LumiBlockRange objects to the
  // m_cacheOutputRangeContainer.  Objects in the m_cacheOutputRangeContainer
  // will be sorted and merged at the end of the job
  // Note: the Suspect Range objects are kept in separate containers because
  // once an object is suspect, it remains suspect forever
  // If a file is only partially read, the objects in the m_cacheInputRangeContainer
  // will be moved to the m_suspectOutputRangeContainer 
  
  m_cacheInputRangeContainer = new xAOD::LumiBlockRangeContainer();
  m_cacheInputRangeAuxContainer = new xAOD::LumiBlockRangeAuxContainer();
  m_cacheInputRangeContainer->setStore( m_cacheInputRangeAuxContainer );
   
  m_cacheSuspectInputRangeContainer = new xAOD::LumiBlockRangeContainer();
  m_cacheSuspectInputRangeAuxContainer = new xAOD::LumiBlockRangeAuxContainer();
  m_cacheSuspectInputRangeContainer->setStore( m_cacheSuspectInputRangeAuxContainer );

  m_cacheOutputRangeContainer = new xAOD::LumiBlockRangeContainer();
  m_cacheOutputRangeAuxContainer = new xAOD::LumiBlockRangeAuxContainer();
  m_cacheOutputRangeContainer->setStore( m_cacheOutputRangeAuxContainer );
  
  m_cacheSuspectOutputRangeContainer = new xAOD::LumiBlockRangeContainer();
  m_cacheSuspectOutputRangeAuxContainer = new xAOD::LumiBlockRangeAuxContainer();
  m_cacheSuspectOutputRangeContainer->setStore( m_cacheSuspectOutputRangeAuxContainer );
}
  
//___________________________________________________________________________
LumiBlockMetaDataTool::~LumiBlockMetaDataTool() {
  delete m_cacheInputRangeContainer;
  delete m_cacheInputRangeAuxContainer;
  delete m_cacheSuspectInputRangeContainer;
  delete m_cacheSuspectInputRangeAuxContainer;

  delete m_cacheOutputRangeContainer;
  delete m_cacheOutputRangeAuxContainer;
  delete m_cacheSuspectOutputRangeContainer;
  delete m_cacheSuspectOutputRangeAuxContainer;
}

//___________________________________________________________________________
StatusCode LumiBlockMetaDataTool::initialize() {
  ATH_MSG_INFO( "in initialize()" );

  ATH_CHECK( m_pMetaDataStore.retrieve() );
  ATH_CHECK( m_pInputStore.retrieve() );

  return(StatusCode::SUCCESS);
}
//___________________________________________________________________________
StatusCode LumiBlockMetaDataTool::finalize() {
  return(StatusCode::SUCCESS);
}

StatusCode LumiBlockMetaDataTool::beginInputFile(const SG::SourceID&)
{
  std::string fileName = "Undefined ";
  bool alreadyRecorded=false;
  if(m_CurrentFileName==fileName) {
    alreadyRecorded=true;
  }
  if(m_fileCurrentlyOpened) {
    alreadyRecorded=true;
  }
  m_CurrentFileName = fileName;
  if(alreadyRecorded) return StatusCode::SUCCESS;
  m_fileCurrentlyOpened=true;
  //
  // Look for LB information on input store and transfer it to temporay cache
  // ===========================================================================
  
  if (m_pInputStore->contains<xAOD::LumiBlockRangeContainer>(m_LBColl_name)) {
    ATH_MSG_INFO(" Contains xAOD::LumiBlockRangeContainer " << m_LBColl_name);
    const xAOD::LumiBlockRangeContainer* lbrange =nullptr;
    StatusCode sc = m_pInputStore->retrieve(lbrange,m_LBColl_name);
    if (!sc.isSuccess()) {
      ATH_MSG_INFO( "Could not find unfinished xAOD::LumiBlockRangeContainer in input metatdata store" );
      return StatusCode::SUCCESS;
    }
    ATH_MSG_INFO( "xAOD::LumiBlockRangeContainer size" << lbrange->size() );
    for ( const auto* lb : *lbrange ) {
      xAOD::LumiBlockRange* iovr = new xAOD::LumiBlockRange(*lb);
      m_cacheInputRangeContainer->push_back(iovr);
    }
  }
  if (m_pInputStore->contains<xAOD::LumiBlockRangeContainer>(m_unfinishedLBColl_name)) {
    ATH_MSG_INFO(" Contains xAOD::LumiBlockRangeContainer " << m_unfinishedLBColl_name);
    const xAOD::LumiBlockRangeContainer* lbrange =nullptr;
    StatusCode sc = m_pInputStore->retrieve(lbrange,m_unfinishedLBColl_name);
    if (!sc.isSuccess()) {
      ATH_MSG_INFO( "Could not find unfinished xAOD::LumiBlockRangeContainer in input metatdata store" );
      return StatusCode::SUCCESS;
    }
    ATH_MSG_INFO( "xAOD::LumiBlockRangeContainer size" << lbrange->size() );
    for ( const auto* lb : *lbrange ) {
      xAOD::LumiBlockRange* iovr = new xAOD::LumiBlockRange(*lb);
      m_cacheInputRangeContainer->push_back(iovr);
    }
  }
  if (m_pInputStore->contains<xAOD::LumiBlockRangeContainer>(m_suspectLBColl_name)) {
    ATH_MSG_INFO(" Contains xAOD::LumiBlockRangeContainer " << m_suspectLBColl_name);
    const xAOD::LumiBlockRangeContainer* lbrange =nullptr;
    StatusCode sc = m_pInputStore->retrieve(lbrange,m_suspectLBColl_name);
    if (!sc.isSuccess()) {
      ATH_MSG_INFO( "Could not find suspect xAOD::LumiBlockRangeContainer in input metatdata store" );
      return StatusCode::SUCCESS;
    }
    ATH_MSG_INFO( "xAOD::LumiBlockRangeContainer size" << lbrange->size() );
    for ( const auto* lb : *lbrange ) {
      xAOD::LumiBlockRange* iovr = new xAOD::LumiBlockRange(*lb);
      m_cacheSuspectInputRangeContainer->push_back(iovr);
    }
  }
  return(StatusCode::SUCCESS);
}

StatusCode LumiBlockMetaDataTool::endInputFile(const SG::SourceID&)
{
  m_fileCurrentlyOpened=false;
  for (const auto range : *m_cacheInputRangeContainer) {
    auto iovr = std::make_unique<xAOD::LumiBlockRange>(*range);
    m_cacheOutputRangeContainer->push_back(std::move(iovr));
  }
  m_cacheInputRangeContainer->clear();
  
  for (const auto range : *m_cacheSuspectInputRangeContainer) {
    auto iovr = std::make_unique<xAOD::LumiBlockRange>(*range);
    m_cacheSuspectOutputRangeContainer->push_back(std::move(iovr));
  }
  m_cacheSuspectInputRangeContainer->clear();
  return(StatusCode::SUCCESS);
}

StatusCode LumiBlockMetaDataTool::metaDataStop()
{
  if(m_fileCurrentlyOpened) {
    ATH_MSG_INFO( "MetaDataStop called when input file open: LumiBlock is suspect" );
    for (const auto range : *m_cacheInputRangeContainer) {
      auto iovr = std::make_unique<xAOD::LumiBlockRange>(*range);
      m_cacheSuspectOutputRangeContainer->push_back(std::move(iovr));
    }
    m_cacheInputRangeContainer->clear();
  }
  
  ATH_CHECK( finishUp() );
  return(StatusCode::SUCCESS);
}

//__________________________________________________________________________
StatusCode   LumiBlockMetaDataTool::finishUp() {
  // 
  // stop() is called whenever the event loop is finished.
  // ======================================================
  ATH_MSG_VERBOSE(  " finishUp: write lumiblocks to meta data store " );
  
  xAOD::LumiBlockRangeContainer* piovComplete = new xAOD::LumiBlockRangeContainer();
  xAOD::LumiBlockRangeAuxContainer* piovCompleteAux = new xAOD::LumiBlockRangeAuxContainer();
  piovComplete->setStore( piovCompleteAux );
  
  xAOD::LumiBlockRangeContainer* piovUnfinished = new xAOD::LumiBlockRangeContainer();
  xAOD::LumiBlockRangeAuxContainer* piovUnfinishedAux = new xAOD::LumiBlockRangeAuxContainer();
  piovUnfinished->setStore( piovUnfinishedAux );
  
  xAOD::LumiBlockRangeContainer* piovSuspect = new xAOD::LumiBlockRangeContainer();
  xAOD::LumiBlockRangeAuxContainer* piovSuspectAux = new xAOD::LumiBlockRangeAuxContainer();
  piovSuspect->setStore( piovSuspectAux );
  
  if(!m_cacheSuspectOutputRangeContainer->empty()) {
    ATH_MSG_VERBOSE("Suspect OutputRangeCollection with size " << m_cacheSuspectOutputRangeContainer->size());
    for (const auto range : *m_cacheSuspectOutputRangeContainer) {
      auto iovr = std::make_unique<xAOD::LumiBlockRange>(*range);
      piovSuspect->push_back(std::move(iovr));
    }
  }
  
  if(!m_cacheOutputRangeContainer->empty()) {
    ATH_MSG_VERBOSE("OutputRangeCollection with size " << m_cacheOutputRangeContainer->size());
    m_cacheOutputRangeContainer->sort(xAOD::SortLumiBlockRangeByStart());
    
    //  Use tmp collection to do the merging
    xAOD::LumiBlockRangeContainer tempLBColl;
    xAOD::LumiBlockRangeAuxContainer* p_tempAuxLBColl = new xAOD::LumiBlockRangeAuxContainer();
    tempLBColl.setStore( p_tempAuxLBColl );
    
    // Sort and Merge LumiBlockRange objects if necessary
    // Merge LumiBlockRange objects for same run and lumiblock
    xAOD::LumiBlockRangeContainer::const_iterator i = m_cacheOutputRangeContainer->begin();
    xAOD::LumiBlockRangeContainer::const_iterator ie = m_cacheOutputRangeContainer->end();
    xAOD::LumiBlockRangeContainer::const_iterator ilast = m_cacheOutputRangeContainer->begin();
    xAOD::LumiBlockRange* iovr = new xAOD::LumiBlockRange(*(*i));
    tempLBColl.push_back(iovr);
    ATH_MSG_VERBOSE(  "Push_back tmpLBColl with run  " 
		      << (*i)->startRunNumber() << " LB " << (*i)->startLumiBlockNumber() << " events seen "     
		      << (*ilast)->eventsSeen() << " expected " << (*i)->eventsExpected());
    ++i;
    while (i != ie) {
      if( ((*i)->startRunNumber()==(*ilast)->startRunNumber()) &&
	  ((*i)->stopRunNumber()==(*ilast)->stopRunNumber()) &&
	  ((*i)->startLumiBlockNumber()==(*ilast)->startLumiBlockNumber()) &&
	  ((*i)->stopLumiBlockNumber()==(*ilast)->stopLumiBlockNumber()) ) {

	if((*ilast)->eventsExpected()!=(*i)->eventsExpected()) {
	  ATH_MSG_WARNING(  "Error: tmpLBColl with run  " << (*i)->startRunNumber() << " LB " << (*i)->startLumiBlockNumber() << " events expected "     
			    << (*ilast)->eventsExpected() << " and " << (*i)->eventsExpected() );
	}
	else {
	  ATH_MSG_VERBOSE(  "Merge Run " << (*i)->startRunNumber() << " LB " << (*i)->startLumiBlockNumber() 
			    << " events seen "  << iovr->eventsSeen() << "+" 
			    << (*i)->eventsSeen() << " and events expected "
			    << iovr->eventsExpected() );
	  
	  iovr->setEventsSeen((*i)->eventsSeen()+iovr->eventsSeen());
	}
      }
      else {
	iovr = new xAOD::LumiBlockRange(*(*i));
	
	ATH_MSG_VERBOSE(  "Push_back tmpLBColl with run  " 
			  << iovr->startRunNumber() << " LB " << iovr->startLumiBlockNumber() << " events seen "     
			  << iovr->eventsSeen() << " expected " << iovr->eventsExpected());
	
	tempLBColl.push_back(iovr);
	ilast = i;
      }
      ++i;
    }

    for (const auto range : tempLBColl) {
      auto iovr = std::make_unique<xAOD::LumiBlockRange>(*range);
      if(range->eventsSeen() == range->eventsExpected() ) {
	piovComplete->push_back(std::move(iovr));
      }
      else if(range->eventsSeen() > range->eventsExpected() ) {
	piovSuspect->push_back(std::move(iovr));
      }
      else {
	piovUnfinished->push_back(std::move(iovr));
      }
    }
  }


  if(!piovComplete->empty()) {
    ATH_MSG_DEBUG( "Number of Complete LumiBlocks:" << piovComplete->size() );
    for (const auto range : *piovComplete) {
      ATH_MSG_INFO("\t [ ("
		   << range->startRunNumber()  << "," << range->startLumiBlockNumber()
		   << "):("
		   << range->startRunNumber()  << "," << range->startLumiBlockNumber()
		   << ") eventsSeen = " << range->eventsSeen()
		   << ", eventsExpected = " << range->eventsExpected()
		   << " ]");
    }
  }

  if(!piovUnfinished->empty()) {
    ATH_MSG_DEBUG( "Number of Unfinished LumiBlocks:" << piovUnfinished->size() );
    for (const auto range : *piovUnfinished) {
      ATH_MSG_INFO("\t [ ("
		   << range->startRunNumber()  << "," << range->startLumiBlockNumber()
		   << "):("
		   << range->startRunNumber()  << "," << range->startLumiBlockNumber()
		   << ") eventsSeen = " << range->eventsSeen()
		   << ", eventsExpected = " << range->eventsExpected()
		   << " ]");
    }
  }
  if(!piovSuspect->empty()) {
    ATH_MSG_DEBUG( "Number of Suspect LumiBlocks:"  << piovSuspect->size() );
    for (const auto range : *piovSuspect) {
      ATH_MSG_INFO("\t [ ("
		   << range->startRunNumber()  << "," << range->startLumiBlockNumber()
		   << "):("
		   << range->startRunNumber()  << "," << range->startLumiBlockNumber()
		   << ") eventsSeen = " << range->eventsSeen()
		   << ", eventsExpected = " << range->eventsExpected()
		   << " ]");
    }
  }

  // Store the LumiBlockRangeContainer in the metadata store
  // =======================================================
  if(!piovComplete->empty()) {
    ATH_MSG_INFO(  "Write Complete LumiBlocks with size  " <<  piovComplete->size());
    ATH_CHECK( m_pMetaDataStore->record( piovComplete, m_LBColl_name ) );
    ATH_CHECK( m_pMetaDataStore->record( piovCompleteAux, m_LBColl_name + "Aux." ) );
  }
  
  if(!piovUnfinished->empty()) {
    ATH_MSG_INFO(  "Write Unfinished LumiBlocks with size  " <<  piovUnfinished->size());
    ATH_CHECK( m_pMetaDataStore->record( piovUnfinished, m_unfinishedLBColl_name ) );
    ATH_CHECK( m_pMetaDataStore->record( piovUnfinishedAux, m_unfinishedLBColl_name + "Aux." ) );
  }

  if(!piovSuspect->empty()) {
    ATH_MSG_INFO(  "Write Suspect LumiBlocks with size  " <<  piovSuspect->size());
    ATH_CHECK( m_pMetaDataStore->record( piovSuspect, m_suspectLBColl_name ) );
    ATH_CHECK( m_pMetaDataStore->record( piovSuspectAux, m_suspectLBColl_name + "Aux." ) );
  }
  
  return(StatusCode::SUCCESS);
}
