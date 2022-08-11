/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigSORFromPtreeHelper.h"

#include "AthenaKernel/IIOVDbSvc.h"
#include "AthenaKernel/IAddressProvider.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "CxxUtils/checker_macros.h"

#include "owl/time.h"
#include <eformat/DetectorMask.h>

using namespace boost::property_tree;

namespace
{
  const std::string CLNAME{"TrigSORFromPtreeHelper"};
}

////////////////////////////////////////////////////////////////////////////////
TrigSORFromPtreeHelper::TrigSORFromPtreeHelper(IMessageSvc* msgSvc,
                                               const ServiceHandle<StoreGateSvc>& detStore,
                                               const std::string& sorpath,
                                               const ptree& rparams) :
  AthMessaging(msgSvc, CLNAME),
  m_detStore(detStore),
  m_sorpath(sorpath),
  m_rparams(rparams)
{
  // Set run number and timestamp from RunParams. Can be overwritten later.
  m_runNumber = rparams.get<unsigned int>("run_number");

  const auto t = OWLTime{(rparams.get_child("timeSOR").data()).c_str()};
  m_sorTime_ns = t.total_mksec_utc() * 1000;
}

////////////////////////////////////////////////////////////////////////////////
StatusCode TrigSORFromPtreeHelper::fillSOR(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Setup SOR in DetectorStore");

  // get handle to the IOVDbSvc
  ServiceHandle<IIOVDbSvc> iovdbsvc("IOVDbSvc", CLNAME);
  if ((iovdbsvc.retrieve()).isFailure()) {
    ATH_MSG_ERROR("Could not find IOVDbSvc. Time dependent conditions data may be not properly handled.");
  } else {
    IOVTime currentIOVTime(ctx.eventID());
    // Signal BeginRun directly to IOVDbSvc to set complete IOV start time
    if ( iovdbsvc->signalBeginRun(currentIOVTime, ctx).isFailure() ) {
      ATH_MSG_ERROR("Unable to signal begin run IOVTime to IOVDbSvc. IOVTime = " << currentIOVTime);
    } else {
      ATH_MSG_DEBUG("Set start of run time to IOVTime = " << currentIOVTime);
    }
  }

  // createSOR marked unsafe due to copying/deletion of coral::AttributeList
  // objects.  There's no sharing of specificiations, though, so it's ok.
  StatusCode sc ATLAS_THREAD_SAFE = createSOR();
  ATH_CHECK( sc );

  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
EventIDBase TrigSORFromPtreeHelper::eventID() const
{
  EventIDBase eid;

  // Set run and timestamp
  eid.set_run_number( m_runNumber );
  eid.set_time_stamp( m_sorTime_ns / (1000*1000*1000) );
  eid.set_time_stamp_ns_offset( m_sorTime_ns % (1000*1000*1000) );

  eid.set_lumi_block(0);  // our best guess as this is not part of RunParams

  return eid;
}

////////////////////////////////////////////////////////////////////////////////
StatusCode TrigSORFromPtreeHelper::createSOR ATLAS_NOT_THREAD_SAFE () const
{
  // obtain SOR contents from ptree
  auto attrList = getAttrList();

  // Validity
  IOVTime iovTimeStart(attrList["RunNumber"].data<unsigned int>(),0);
  IOVTime iovTimeStop(attrList["RunNumber"].data<unsigned int>()+1,0);
  IOVRange iovRange(iovTimeStart, iovTimeStop);

  auto sor = new SOR(/*hasRunLumiBlockTime*/true);
  sor->add(SOR::ChanNum{0}, attrList);
  sor->add(SOR::ChanNum{0}, iovRange);
  sor->resetMinRange();
  sor->addNewStart(iovTimeStart);
  sor->addNewStop(iovTimeStop);

  // Record or overwrite existing SOR
  if ( m_detStore->transientContains<SOR>(m_sorpath) ) {
    const SOR * oldsor = m_detStore->retrieve<const SOR>(m_sorpath);
    ATH_MSG_INFO("Overwriting SOR contents (a dump of the old one follows):");
    oldsor->dump();
    ATH_CHECK( m_detStore->overwrite(sor, m_sorpath, true) );
  }
  else {
    ATH_MSG_DEBUG("Recording new SOR");
    ATH_CHECK( m_detStore->record(sor, m_sorpath, true) );
  }

  ATH_CHECK( setIOVRange(iovRange) );
  ATH_CHECK( updateProxy(sor) );

  ATH_MSG_INFO("Successfully setup SOR:");
  sor->dump();

  return StatusCode::SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
coral::AttributeList TrigSORFromPtreeHelper::getAttrList ATLAS_NOT_THREAD_SAFE () const
{
  // First create attribute specification
  // ugly new needed:
  // dtor is protected, have to use ptr and release it explicitly... go figure
  auto attrSpec = new coral::AttributeListSpecification{};
  attrSpec->extend("RunNumber", "unsigned int");
  attrSpec->extend("SORTime", "unsigned long long");
  attrSpec->extend("RunType", "string");
  attrSpec->extend("DetectorMaskFst", "unsigned long long");
  attrSpec->extend("DetectorMaskSnd", "unsigned long long");
  attrSpec->extend("RecordingEnabled", "bool");

  // now create the attribute list and fill it in
  coral::AttributeList attrList(*attrSpec);

  attrList["RunNumber"].data<unsigned int>() = m_runNumber;
  attrList["RunType"].data<std::string>()    = m_rparams.get<std::string>("run_type");
  attrList["RecordingEnabled"].data<bool>()  = m_rparams.get<bool>("recording_enabled");

  attrList["SORTime"].data<unsigned long long>() = m_sorTime_ns;

  std::pair<uint64_t, uint64_t> dm = eformat::helper::DetectorMask(m_rparams.get_child("det_mask").data()).serialize();
  attrList["DetectorMaskFst"].data<unsigned long long>() = dm.first;
  attrList["DetectorMaskSnd"].data<unsigned long long>() = dm.second;

  // coral wants us to have to do this explicitly...
  attrSpec->release(); // we don't delete because the dtor is private
                       // we have to hope the CORAL framework will

  return attrList;
}

////////////////////////////////////////////////////////////////////////////////
StatusCode TrigSORFromPtreeHelper::setIOVRange(IOVRange & iovRange) const
{
  // set IOVRange on the IOVSvc
  ServiceHandle<IIOVSvc> iovsvc("IOVSvc", CLNAME);
  ATH_CHECK( iovsvc.retrieve() );

  auto clid = ClassID_traits<SOR>::ID();
  ATH_CHECK( iovsvc->setRange(clid, m_sorpath, iovRange, "StoreGateSvc") );

  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
StatusCode TrigSORFromPtreeHelper::updateProxy(SOR * sor) const
{
  // check the SOR_Params proxy and add if necessary an IAddressProvider (typically for MC)
  auto proxy = m_detStore->proxy(sor);
  if (!proxy) {
    ATH_MSG_ERROR("Could not find proxy for SOR_Params folder.");
    return StatusCode::FAILURE;
  }

  // check if the transient address has an IAddressProvider, if not set IOVDbSvc as provider
  if (!proxy->provider()) {
    // get handle to the IOVDbSvc
    ServiceHandle<IIOVDbSvc> iovdbsvc("IOVDbSvc", CLNAME);
    ATH_CHECK( iovdbsvc.retrieve() );

    IAddressProvider* provider = dynamic_cast<IAddressProvider*>(&*iovdbsvc);
    if (!provider) {
      ATH_MSG_ERROR("Could not cast to IAddressProvider interface and set the provider for SOR_Params.");
      return StatusCode::FAILURE;
    }
    proxy->setProvider(provider, proxy->storeID());
  }

  return StatusCode::SUCCESS;
}

