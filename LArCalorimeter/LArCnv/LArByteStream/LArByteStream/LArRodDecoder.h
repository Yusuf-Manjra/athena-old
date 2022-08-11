//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARBYTESTREAM_LARRODDECODER_H
#define LARBYTESTREAM_LARRODDECODER_H

#include <stdint.h>

//#include "GaudiKernel/AlgTool.h"
//#include "GaudiKernel/MsgStream.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "LArRawEvent/LArRawChannelContainer.h"
#include "LArRawEvent/LArDigit.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArCalibDigitContainer.h"
#include "LArRawEvent/LArAccumulatedCalibDigit.h"
#include "LArRawEvent/LArAccumulatedCalibDigitContainer.h"
#include "LArRawEvent/LArAccumulatedDigit.h"
#include "LArRawEvent/LArAccumulatedDigitContainer.h"
#include "LArRawEvent/LArFebHeaderContainer.h"
#include "LArRecEvent/LArCellCollection.h"


#include "LArByteStream/LArRodBlockStructure.h"
#include "CaloUtils/CaloCellCorrection.h"

#include "ByteStreamData/RawEvent.h" 
#include "LArRecEvent/LArFebEnergy.h" 
#include "LArRecEvent/LArFebEnergyCollection.h" 

#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "CxxUtils/checker_macros.h"

#include "eformat/Version.h"
#include "eformat/Issue.h"
#include <vector>
#include <string>

class LArCalibLineMapping;
class LArOnOffIdMapping;

/** This class provides conversion between ROD data and LArRawChannels
   * @author H. Ma
   * @version  0-0-1 , Oct 7, 2002

   * Modified, Dec 4, 2002
       Create either LArRawChannel or LArCell. 

   * Modified, Jan 02, 2003
       Moved Encoding part to LArROD_Encoder. 

   * Modified, Jul 23, 2003
       by R. Lafaye
       Added LArDigit support
       Build on the same structure as LArROD_Encoder
       No longer an AlgTool

   * Modified, Aug 5 2003
       by W. Lampl
       Have now functionality to decode:
       - LArDigitContainer
       - LArRawChannelCollection (as IdentfiableContainer)
       - LArCellCollection (as Identifiable Container)
       Is now again a AlgTool since I need jobOpts for the
       LArCell creation. 

   * Modified, Aug 7 2003
      by W. Lampl
      Made a template-function for LArRawChannelCollection
      and LArRawCellCollection

   * Modified, Aug 17 2003
      by W. Lampl
      Introduce ability to decode different ROD format versions
      by using a map of LArRodBlockStructures

   * Modified, Sept 9 2003
      by W. Lampl
      Change implementation of multiple-version feature.
      A ROD block type/version is now written as second word of the fragment. 
      Replace std::map by c-style array to increase performance

   * Modified, Sept 28 2003
     by W. Lampl
     Adapt for new constructor of LArRawChannels. 
     (Needs also gain as parameter)

   * Modified, November 10 2005
     by D.O. Damazio
     Inclusion of new RodBlock format (Bertrand Laforge).
     Inclusion of methods for fast decoding of data.

   * Modified,January 04 2006
     by I. Wingerter
     Inclusion of Accumulated Calib Digit

   * Modified, May 03 2006
     by I. Aracena
     Inclusion of fillCollectionHLTFeb

   * Modified, Dec 03 2006
     by G. Rosenbaum
     Loop over all possible pusled calib lines and make isPulsed int
     which stores which lines 1-4 have been pulsed. (needed for HEC)


   * Modified Aug 16 2009 
     by W. Lampl
     Remove internal accumlation of LArAccumulatedCalibDigit.
     They are now put in the event store for each substep and 
     contain only data of this substep.

   */


//using namespace OFFLINE_FRAGMENTS_NAMESPACE ; 

class LArRodDecoder : public AthAlgTool
{

public: 
//   typedef EventFormat::RODHeader RODHEADER; //Give default template explicitly to work around apparent compiler bug
//  typedef eformat::Header<const uint32_t *>  RODHeader ;

  /** Constructor
      Standard AlgTool constructor
  */
   LArRodDecoder(const std::string& type, const std::string& name,
		 const IInterface* parent ) ;

  static const InterfaceID& interfaceID( ) ;
  /** Destructor
   */ 
  virtual ~LArRodDecoder(); 

  virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE () override;

  // Simple method to associate the second feb
  inline void setsecfeb (HWIdentifier feb) {m_febIdHLT = feb.get_identifier32().get_compact();};

  // fast decoding for trigger
  inline uint32_t fillCollectionHLT (const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                                     const uint32_t* p, uint32_t n, LArCellCollection& coll,
				     LArRodBlockStructure*& providedRodBlockStructure, 
				     uint16_t& rodMinorVersion, uint32_t& robBlockType) const;

  void fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                      const uint32_t* p, uint32_t n, LArRawChannelContainer& coll, const CaloGain::CaloGain gain) const; 

  //Specialized method to convert ROD Data words into LArDigit
  void fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                      const uint32_t* p, uint32_t n, LArDigitContainer& coll, const CaloGain::CaloGain gain) const;

  //Specialized method to convert ROD Accumulated Calib Digit words into LArAccumulatedCalibDigit
  void fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                      const uint32_t* p,
                      uint32_t n,
                      LArAccumulatedCalibDigitContainer& coll,
                      const CaloGain::CaloGain gain,
                      const LArCalibLineMapping& calibLineMapping) const;

  //Specialized method to convert ROD Accumulated Digit words into LArAccumulatedDigit
  void fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                      const uint32_t* p, uint32_t n, LArAccumulatedDigitContainer& coll, const CaloGain::CaloGain gain) const;

  //Specialized method to convert ROD Data words into LArCalibDigit
  void fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                      const uint32_t* p,
                      uint32_t n,
                      LArCalibDigitContainer& coll,
                      const CaloGain::CaloGain gain,
                      const LArCalibLineMapping& calibLineMapping,
                      const LArOnOffIdMapping& onOffIdMapping) const;

  //Specialized method to convert ROD Data words to read the headers of the Feb
  void fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                      const uint32_t* p, uint32_t n, LArFebHeaderContainer& coll, const CaloGain::CaloGain) const;

  //Send an error reported by the eformat package to a MsgStream.
  //inline void report_error (const ers::Issue& error, MsgStream& log);
  //Inputs: error: The eformat exception
  //        log:   The Gaudi MsgStream.

  //Check an eformat fragment for validity. Report an error if it's not valid.
  template <class T>
  inline bool check_valid (const T* frag, MsgStream& log) const;
  //Inputs: frag: The fragment to check
  //        log:  The Gaudi MsgStream.
	
    
    
private:
  LArRodBlockStructure* prepareBlockStructure1(const uint16_t rodMinorVersion, const uint32_t robBlockType) const;
  LArRodBlockStructure* prepareBlockStructure(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                                              const uint32_t* p, uint32_t n, const CaloGain::CaloGain RequestedGain) const;

  std::unique_ptr<LArRodBlockStructure>
  makeBlockStructure (unsigned int rodBlockType,
                      unsigned int rodMinorVersion) const;

  inline void setCellEnergy(
   LArCell* element, int energy, int time,
   int quality, CaloGain::CaloGain gain) const ;

  inline void writeFebInfo(
     LArCellCollection& m_coll, LArFebEnergy& febene) const;

  //  inline void writeFebInfo(LArRawChannelContainer& m_coll, LArFebEnergy& febene);

  
  SG::ReadHandleKey<xAOD::EventInfo>    m_evt  {this, "EvtInfo", "EventInfo", "EventInfo name"};
  float	m_LArCellEthreshold ;
  bool m_readtdc; // set to True if the tdc phase is read from the upper byte of the nsamples word.
  bool m_febExchange; //HW cause, can not be fixed otherwise: Ugly hack for swapped FEB
  unsigned int m_febId1, m_febId2, m_febIdHLT = 0U;

  int m_firstSample; // FIXED but kept for backward compatibility
  // In Auto-Gain mode, the FEB does not send the samples in the right order, but the one first that 
  // was used for the gain selection. Unfortunatly, there is no way to extract the index if this sample 
  // form the raw data, one has to know it in advance (from the FEB configuration). 
  // For the time beeing, the jobOption ToolSvc.LArRodDecoder.FirstSample tells the LArRodDecoder what 
  // sample to put in front. 


  std::vector<int> m_vFTPreselection;
  std::vector<int> m_vBEPreselection;
  std::vector<int> m_vPosNegPreselection;
  std::vector<unsigned int> m_vFinalPreselection;
  uint32_t m_StatusMask, m_StatusNMask = 0U;
  const LArOnlineID*       m_onlineHelper; 
  std::vector<std::string> m_LArCellCorrNames;
  std::vector<unsigned int> m_IgnoreCheckFEBs;
  std::vector<const CaloCellCorrection*> m_LArCellCorrTools;
  
  double m_delayScale;
  mutable SG::SlotSpecificObj<std::vector<std::unique_ptr<LArRodBlockStructure> > >
    m_blstructs ATLAS_THREAD_SAFE;

  bool m_MultiDSPMode;
  bool m_CheckSum;
  unsigned short m_requiredPhysicsNSamples;
} ; 

template <class T>
inline bool LArRodDecoder::check_valid (const T* frag, MsgStream& /*log*/) const
{ 


/* FIXME,  exception?
  EventFormat::Error error;
  if (!frag->is_valid (&error)) {
    report_error (error, log);
    return false;
  }
  return true;
*/

  bool ret=false;
  try {
    ret=frag->check();
  } 
  //catch ( .... ) {
  catch (eformat::Issue& ex) {
    msg(MSG::WARNING) << "Exception while checking eformat fragment validity: " << ex.what() << endmsg; 
    ret=false;
  }
  return ret;

}

/*
inline void LArRodDecoder::report_error (const ers::Issue& error, MsgStream& log)
{ log << MSG::FATAL << "Error reading bytestream event: " << error.what() << endmsg;
}
*/

// fillCollection for HLT without automatic BS Cnv
// default values
static uint16_t rMV=99;
static uint32_t rBT=99;
uint32_t LArRodDecoder::fillCollectionHLT(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
                                          const uint32_t* p, uint32_t n,
                                          LArCellCollection& coll, 
					  LArRodBlockStructure*& providedRodBlockStructure, uint16_t& rodMinorVersion =  rMV , uint32_t& robBlockType = rBT ) const
{
  LArCell *collElem=0; //Pointer to a new element to be added to the collection
  uint32_t error = 0;

#ifndef NDEBUG
  ATH_MSG_VERBOSE("Prepare LArRodBlockStructure. Got a fragment of size " << n);
#endif
/*  if (n<2) //Avoid segmentation fault
    {(*m_log) << MSG::WARNING << "Got empty Rod Fragment!" << endmsg;
     return;
    }
*/
  const uint32_t blocksize=p[0]; //First word contains block size
  if (blocksize>n) {
    msg(MSG::ERROR) << "Got truncated ROD Fragment!" << endmsg;
    // First Bit is truncated (see also below)
    error|= 0x1;
    return error;
  }

  //Get version and blocktype form header
  eformat::helper::Version ver(robFrag.rod_version());
  const uint16_t rMV_present=ver.minor_version();
  const uint32_t rBT_present=robFrag.rod_detev_type()&0xff;

  LArRodBlockStructure* BlStruct(nullptr);
  if ( !providedRodBlockStructure || (rodMinorVersion!=rMV_present) || (robBlockType!=rBT_present) ){
  BlStruct = prepareBlockStructure1 (rMV_present, rBT_present);
  if (!BlStruct) {
    // Second Bit is block empty or unknown
    error|= 0x2;
    return error;
  }
  providedRodBlockStructure = BlStruct;
  rodMinorVersion = rMV_present;
  robBlockType = rBT_present;
  } else BlStruct = providedRodBlockStructure;

  BlStruct->setFragment(p,n);
  for(LArCellCollection::iterator ii=coll.begin();ii!=coll.end();++ii)
       (*ii)->setEnergyFast(0.0);

  int32_t energy(0);
  int32_t time;
  int32_t quality;
  uint32_t gain;
  CaloGain::CaloGain calogain;
  int fcNb;
  int nfeb;
  calogain=CaloGain::LARNGAIN;
  if(!BlStruct->setGain(calogain)){
    ATH_MSG_DEBUG("Setting the Gain Problem");
  }

  int feb_number=0;
  do //Loop over FEB's
    {
      HWIdentifier fId(BlStruct->getFEBID());
      if (!(fId.get_identifier32().get_compact())) {
#ifndef NDEBUG
	ATH_MSG_DEBUG("Bad FebID=0x"<< std::hex << BlStruct->getFEBID() << std::dec << " found for this FEB, skipping it!");
#endif
     // Third Bit is FEB ID issue (disabled - use 0x20 instead)
     //error|= 0x4;
       continue;
    }
  // RL 05.11.2007 checksum
  if(m_CheckSum) {
    uint32_t onsum  = BlStruct->onlineCheckSum();
    uint32_t offsum = BlStruct->offlineCheckSum();
    if(onsum!=offsum) {
      msg(MSG::WARNING) << "Checksum error:" << endmsg;
      msg(MSG::WARNING) << " online checksum  = 0x" << MSG::hex << onsum  << endmsg;
      msg(MSG::WARNING) << " offline checksum = 0x" << MSG::hex << offsum << MSG::dec << endmsg;
     // Fourth Bit CheckSum issue (maybe disabled!)
     error|= 0x8;
      continue;
    }
  }
  feb_number++;

  if ( BlStruct->hasPhysicsBlock() ) {
      if ( fId == m_febIdHLT ) nfeb = 128; // This is the second feb
        else nfeb = 0;
      int NthisFebChannel=128 ; // m_onlineHelper->channelInSlotMax(fId);
     uint16_t iquality;
     uint16_t iprovenance;
     while (BlStruct->getNextEnergy(fcNb,energy,time,quality,gain)) {
        if (fcNb>=NthisFebChannel) continue;
        collElem = coll[fcNb+nfeb];
        iprovenance=0x1000; // data comes from DSP computation
        iquality=0;
        if ( quality>=0 ) { iprovenance|= 0x2000; iquality=(quality& 0xffff);}
	// time converted to ns
	collElem->set(energy, time*1e-3, iquality, iprovenance, (CaloGain::CaloGain)gain);
        //setCellEnergy(collElem,energy, time, quality, (CaloGain::CaloGain)gain);
       }
       continue;
    } 
  }
  while (BlStruct->nextFEB()); //Get NextFeb
  // Error meaning data corruption. Maybe in any FEB
  if ( BlStruct->report_error() ) error |= 0x1;
  // error of uncompatible number of cells is bit 5
  unsigned int collection_size = coll.size();
  if ( feb_number== 0 && collection_size>0 ) error |= 0x20;
  if ( feb_number== 1 && collection_size !=128 ) error |= 0x20;
  if ( feb_number== 2 && collection_size !=256 ) error |= 0x20;
  return error;
}

inline void LArRodDecoder:: setCellEnergy(
   LArCell* element, int energy, int time,
   int quality, CaloGain::CaloGain gain) const
{
  // Set energy, time, quality and hardware gain
  element->set((float)energy, (float)time, (double)quality, gain);
}

inline void LArRodDecoder:: writeFebInfo(
   LArCellCollection& coll, LArFebEnergy& febene) const
 {
   coll.addfebenergy(febene);
 }

#endif

