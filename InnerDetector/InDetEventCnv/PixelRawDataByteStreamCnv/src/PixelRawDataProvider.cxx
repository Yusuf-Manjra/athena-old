/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelRawDataProvider.h"
#include <memory>

#include "InDetIdentifier/PixelID.h"
#include "EventContainers/IdentifiableContTemp.h"

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

// --------------------------------------------------------------------
// Constructor

PixelRawDataProvider::PixelRawDataProvider(const std::string& name,
				       ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator) {
  declareProperty("RDOCacheKey", m_rdoCacheKey);
  declareProperty("BSErrorsCacheKey", m_bsErrorsCacheKey);
}

// Destructor

PixelRawDataProvider::~PixelRawDataProvider() {
}

// --------------------------------------------------------------------
// Initialize

StatusCode PixelRawDataProvider::initialize() {
  ATH_MSG_INFO("PixelRawDataProvider::initialize");

  // Get ROBDataProviderSvc
  if (m_robDataProvider.retrieve().isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve service " << m_robDataProvider);
    return StatusCode::FAILURE;
  } else
    ATH_MSG_INFO("Retrieved service " << m_robDataProvider);

  // Get PixelRawDataProviderTool
  if (m_rawDataTool.retrieve().isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve tool " << m_rawDataTool);
    return StatusCode::FAILURE;
  } else
    ATH_MSG_INFO("Retrieved tool " << m_rawDataTool);

  ATH_CHECK(detStore()->retrieve(m_pixel_id, "PixelID"));
  ATH_CHECK( m_rdoContainerKey.initialize() );

  ATH_CHECK( m_rdoCacheKey.initialize( SG::AllowEmpty ) );

  ATH_CHECK( m_bsErrorsKey.initialize() );
  ATH_CHECK( m_bsErrorsCacheKey.initialize( SG::AllowEmpty ) );

  if (m_roiSeeded) {
    ATH_CHECK( m_roiCollectionKey.initialize() );
    ATH_CHECK(m_regionSelector.retrieve());
  }else{
    ATH_CHECK( m_roiCollectionKey.initialize(false) ); //clear if unneeded
    m_regionSelector.disable();
  }

  ATH_CHECK(m_condCablingKey.initialize());

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------
// Execute

using DummyPixelRDO = EventContainers::IdentifiableContTemp<InDetRawDataCollection<PixelRDORawData>>;

StatusCode PixelRawDataProvider::execute(const EventContext& ctx) const {

#ifdef PIXEL_DEBUG
    ATH_MSG_DEBUG("Create Pixel RDO Container");
#endif

  // now create the container and register the collections


  // write into StoreGate

  SG::WriteHandle<PixelRDO_Container> rdoContainer(m_rdoContainerKey, ctx);
  if( m_rdoCacheKey.empty() ) rdoContainer = std::make_unique<PixelRDO_Container>(m_pixel_id->wafer_hash_max(), EventContainers::Mode::OfflineFast);
  else{
    SG::UpdateHandle<PixelRDO_Cache> updateh(m_rdoCacheKey, ctx);
    if( ! updateh.isValid() ) {
      ATH_MSG_FATAL("Failure to retrieve cache " << m_rdoCacheKey.key());
      return StatusCode::FAILURE;
    }
    rdoContainer = std::make_unique<PixelRDO_Container>(updateh.ptr());
    ATH_MSG_DEBUG("Created container " << m_rdoContainerKey.key() << " using external cache " << m_rdoCacheKey.key());
  }
  ATH_CHECK(rdoContainer.isValid());

  // Print ROB map in m_robDataProvider
  //m_robDataProvider->print_robmap();

  // ask ROBDataProviderSvc for the vector of ROBFragment for all Pixel ROBIDs
  std::vector<uint32_t> listOfRobs;

  if (!m_roiSeeded) {
    ATH_MSG_DEBUG("No RoI seed, fetching all ROBs");
    listOfRobs = SG::ReadCondHandle<PixelCablingCondData>(m_condCablingKey, ctx)->get_allRobs();  // need ROB id (not ROD)
  }
  else {//Enter RoI-seeded mode
     ATH_MSG_DEBUG("RoI seed, fetching regions infromation");
     SG::ReadHandle<TrigRoiDescriptorCollection> roiCollection(m_roiCollectionKey, ctx);
     ATH_CHECK(roiCollection.isValid());

     TrigRoiDescriptorCollection::const_iterator roi = roiCollection->begin();
     TrigRoiDescriptorCollection::const_iterator roiE = roiCollection->end();
     TrigRoiDescriptor superRoI;//add all RoIs to a super-RoI
     superRoI.reserve(roiCollection->size());
     superRoI.setComposite(true);
     superRoI.manageConstituents(false);
     for (; roi!=roiE; ++roi) {
       superRoI.push_back(*roi);
     }
     m_regionSelector->ROBIDList( superRoI, listOfRobs );
  }
  std::vector<const ROBFragment*> listOfRobf;

  m_robDataProvider->getROBData(ctx, listOfRobs, listOfRobf);

#ifdef PIXEL_DEBUG
    ATH_MSG_DEBUG("Number of ROB fragments " << listOfRobf.size()
                  << " (out of=" << listOfRobs.size() << " expected)");
#endif

  std::unique_ptr<DummyPixelRDO> tempcont;
  if( not m_rdoCacheKey.empty() ) tempcont = std::make_unique<DummyPixelRDO> (rdoContainer.ptr());

  IPixelRDO_Container *containerInterface = tempcont ? static_cast< IPixelRDO_Container* >(tempcont.get()) :
         static_cast< IPixelRDO_Container* >(rdoContainer.ptr());

  std::unique_ptr<IDCInDetBSErrContainer> decodingErrors;
  if ( not m_bsErrorsCacheKey.empty() ) {
    SG::UpdateHandle<IDCInDetBSErrContainer_Cache> bsErrorsCacheHandle( m_bsErrorsCacheKey, ctx);
    decodingErrors = std::make_unique<IDCInDetBSErrContainer>( bsErrorsCacheHandle.ptr() );
  } else {
    decodingErrors = std::make_unique<IDCInDetBSErrContainer>( m_rawDataTool->SizeOfIDCInDetBSErrContainer(), std::numeric_limits<int>::min() );
  }

  // ask PixelRawDataProviderTool to decode it and to fill the IDC
  if (m_rawDataTool->convert(listOfRobf,  containerInterface, *decodingErrors).isFailure())
    ATH_MSG_ERROR("BS conversion into RDOs failed");

  if(tempcont) ATH_CHECK(tempcont->MergeToRealContainer(rdoContainer.ptr()));

  SG::WriteHandle<IDCInDetBSErrContainer> bsErrorsHandle(m_bsErrorsKey, ctx);
  ATH_CHECK( bsErrorsHandle.record( std::move( decodingErrors ) ) );

#ifdef PIXEL_DEBUG
    ATH_MSG_DEBUG("Number of Collections in IDC " << rdoContainer->numberOfCollections());
#endif

  return StatusCode::SUCCESS;
}
