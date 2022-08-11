/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "LArEventTest/DumpLArRawChannels.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "xAODEventInfo/EventInfo.h"
//#include "testpack/compdigit.h"
#include <stdlib.h>

#include <vector>

DumpLArRawChannels::DumpLArRawChannels(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator),
    m_chan(0),
    m_onlineHelper(0),
    m_emId(0),
    m_thistSvc ("THistSvc",     name) 
{m_count=0;
 declareProperty("LArRawChannelContainerName",m_key=""); 
 declareProperty("OutputFileName",m_FileName="LArRawChannels.txt");
 declareProperty("NtupStream",m_ntup);
 declareProperty("ToLog",m_toLog=true);
}

DumpLArRawChannels::~DumpLArRawChannels() 
{//empty
}

StatusCode DumpLArRawChannels::initialize()
{
  ATH_MSG_INFO ( "======== initialize DumpLArRawChanels ========" );

  ATH_CHECK( detStore()->retrieve(m_onlineHelper, "LArOnlineID") );
  ATH_MSG_DEBUG ( " Found the LArOnlineID helper. " );

  const CaloCell_ID* idHelper = nullptr;
  ATH_CHECK( detStore()->retrieve (idHelper, "CaloCell_ID") );
  m_emId=idHelper->em_idHelper();
  if (!m_emId) {
    ATH_MSG_ERROR ( "Unable to get EM_ID" );
    return StatusCode::FAILURE;
  }

  ATH_CHECK( m_cablingKey.initialize() );

  if (!m_FileName.empty()) {
    m_outfile.open(m_FileName.c_str(),std::ios::out);
    m_toFile=true;
  }
  else 
    m_toFile=false;

  if (m_ntup.size()) {
    m_tree= new TTree("RC","LArRawChannels");
    const std::string ntupStream="/"+m_ntup+"/LArRawChanenls";
    ATH_CHECK(m_thistSvc->regTree(ntupStream,m_tree));

    m_tree->Branch("evt",&m_evt,"evt/I");
    m_tree->Branch("e",&m_e,"e/F");
    m_tree->Branch("t",&m_t,"t/F");
    m_tree->Branch("Q",&m_Q,"q/F");
    m_tree->Branch("gain",&m_gain,"gain/I");
    m_tree->Branch("HWID",&m_id,"hwid/I");
      
  }


  ATH_MSG_INFO ( "======== test-stuff initialize successfully ========" );
  return StatusCode::SUCCESS;
}


StatusCode DumpLArRawChannels::execute()
{
 m_count++; 
 ATH_MSG_INFO ( "======== executing event "<< m_count << " ========" );
 const DataHandle<xAOD::EventInfo> thisEventInfo;
 StatusCode sc=evtStore()->retrieve(thisEventInfo);
 if (sc!=StatusCode::SUCCESS)
   ATH_MSG_WARNING ( "No EventInfo object found!" );
 else
   {
     if (m_toLog) std::cout << "*** Event #" << std::dec << thisEventInfo->runNumber() << "/" << thisEventInfo->eventNumber() << std::endl;
     m_evt=thisEventInfo->eventNumber();
   }

 const DataHandle < LArRawChannelContainer > channel_cont;
 
 SG::ReadCondHandle<LArOnOffIdMapping> larCablingHdl(m_cablingKey);
 const LArOnOffIdMapping* cabling=*larCablingHdl;
 if(!cabling) {
     ATH_MSG_ERROR("Could not get LArOnOffIdMapping !!");
     return StatusCode::FAILURE;
 }

 if (m_key.size())
   ATH_CHECK( evtStore()->retrieve(channel_cont,m_key) );
 else
   ATH_CHECK( evtStore()->retrieve(channel_cont) );
 ATH_MSG_INFO ( "Retrieved LArRawChannelContainer from StoreGate! key=" << m_key );
 //int nColl=0;
 short layer,eta,phi;
 m_chan=0;
 //int Nchan=0;
 typedef std::vector< const LArRawChannel* > LArRawChannelVector;
 //std::vector< const LArRawChannel* > channelVector;
 LArRawChannelVector channelVector;
 LArRawChannelContainer::const_iterator it_chan_coll = channel_cont->begin(); 
 LArRawChannelContainer::const_iterator it_chan_coll_end = channel_cont->end(); 
 //log <<MSG::DEBUG<<" number of LArRawChannel collections "<< channel_cont->size() <<endmsg;
 for( ; it_chan_coll!=it_chan_coll_end;++it_chan_coll) {
   m_chan++;
   channelVector.push_back(&(*it_chan_coll));
 }
 if (m_toFile) m_outfile << "Event " << m_count << " contains " << m_chan << " channels\n";
 if (m_toLog) std::cout << "Event " << m_count << " contains " << m_chan << " channels\n";
 
 mySort aSort;
 std::sort(channelVector.begin(),channelVector.end(),aSort);
 for (const LArRawChannel* chan : channelVector)
   {
     //if (chan->energy()==0)
     //continue;
    const HWIdentifier chid=chan->channelID();//hardwareID();
    if (m_toLog) std::cout << "Channel: " << m_onlineHelper->channel_name(chid);
    if (m_toFile) m_outfile << "Channel: " << m_onlineHelper->channel_name(chid);

    if (!cabling->isOnlineConnected(chid)) {
      if (m_toLog) std::cout << " disconnected" << std::endl;
      if (m_toFile) m_outfile << " disconnected" << std::endl;
      continue;
    }
    const Identifier id=cabling->cnvToIdentifier(chid);

    if(!m_emId->is_lar_em(id)) {
      eta=m_emId->eta(id); 
      phi=m_emId->phi(id);
      layer=m_emId->sampling(id);      
      if (m_emId->is_em_endcap(id)) {
	if (m_toLog) std::cout << " Endcap l/e/p= " << layer << "/" << eta << "/" << phi << ":";
	if (m_toFile) m_outfile << " Endcap l/e/p= " << layer << "/" << eta << "/" << phi << ":";
      }
      else {
	if (m_toLog) std::cout << " Barrel l/e/p= " << layer << "/" << eta << "/" << phi << ":";
	if (m_toFile) m_outfile << " Barrel l/e/p= " << layer << "/" << eta << "/" << phi << ":";
      }
    }
    else {
      if (m_toLog) std::cout << " (is not EM)";
      if (m_toFile) m_outfile << " (is not EM)";
    }


    int Time=chan->time();
    /*
    if (Time>0x1fff)
      Time=0x1fff;
    if (Time<-0x1fff)
      Time=-0x1fff;
    */
    //if (abs(Time)>24000)
    //Time=0;
    if (m_toLog) std::cout << " E= " << chan->energy() << " t= " << Time << " Q= " << chan->quality() << " P=0x" 
	      << std::hex << chan->provenance() << std::dec << " G=" << chan->gain() << std::endl;
    
    if (m_toFile) m_outfile << " E= " << chan->energy() << " t= " << Time << " Q= " << chan->quality() << " P=0x"
	      << std::hex << chan->provenance() << std::dec << " G=" << chan->gain() << std::endl;

    if (m_tree) {
      m_e=chan->energy();
      m_t=chan->time();
      m_Q=chan->quality();
      m_gain=chan->gain();
      m_id=chid.get_identifier32().get_compact();
      m_tree->Fill();
    }
   }
 //std::cout << "Collection #" << ++nColl << " contains " << chan_coll->size() << " elementes." << std::endl;
 std::cout << "Event " << m_count << " contains " << m_chan << " (" <<channelVector.size() <<")  channels\n";
 return StatusCode::SUCCESS;
}

StatusCode DumpLArRawChannels::finalize()
{
  if (m_toFile) m_outfile.close(); 
  ATH_MSG_INFO ( "======== finalize DumpLArRawChannel ========" );
  return StatusCode::SUCCESS;
}

bool DumpLArRawChannels::mySort::operator()(const LArRawChannel* a, const LArRawChannel* b)
{return a->channelID() < b->channelID();}
