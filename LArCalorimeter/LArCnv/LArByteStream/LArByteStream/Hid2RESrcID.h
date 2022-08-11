/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARBYTESTREAM_HID2RESRCID_H
#define LARBYTESTREAM_HID2RESRCID_H

#include "LArByteStream/RodRobIdMap.h" 
#include "LArIdentifier/LArOnlineID.h" 


#include <stdint.h> 
#include <map>

#include "LArIdentifier/LArReadoutModuleService.h"
#include "CxxUtils/checker_macros.h"

class LArFebRodMapping;



// this class provides conversion between HWIdentifier and RESrcID
// HWIdentifier is used in LArRawChannel to identify each channel 
// RESrcID is used for identifying each ROD. 
// 
// It also provides conversion from ROC RESrcID to a LArFEB_ID. 
//  LArFEB_ID is currently used as ID for LArRawChannelCollection. 
//  each Collection corresponds to a ROC. 
//

// Revision: July 11, 2002
//  Modified for the new RE source ID, to be used with eformat package.

// Revision: Aug 11, 2006
//  move code from constructor to initialize() method.
//  only use the object after initialize() is called.

class Hid2RESrcID {

public:

  typedef HWIdentifier COLLECTION_ID; 

  /** constructor 
  */ 
  Hid2RESrcID(); 

  StatusCode initialize (const LArFebRodMapping& rodMapping);

  bool isInitialized() const { return m_initialized;}

  /** make a ROD SrcID for a HWIdentifier 
  */
  uint32_t getRodID (const LArFebRodMapping& rodMapping,
                     const HWIdentifier& hid) const;

  /** make a ROD SrcID for a COLLECTION_ID
  */ 
  uint32_t getRodIDFromROM  (const COLLECTION_ID& hid) const;

  /** Make a ROB Source ID from a ROD source ID
  */ 
  uint32_t getRobID  ( uint32_t rod_id) const;

  /** Make a ROS Source ID from a ROB source ID
  */ 
  uint32_t getRosID  ( uint32_t rob_id) const;

  /** Make a SubDetector ID from ROS source ID 
  */
  uint32_t getDetID  ( uint32_t ros_id) const;

private:
  StatusCode initialize (const std::vector<HWIdentifier>& roms);

  bool m_initialized;
  const LArOnlineID* m_onlineHelper; 
  typedef std::map<HWIdentifier, uint32_t> COLL_MAP ; 
  COLL_MAP m_coll2ROD ; 
  LArReadoutModuleService m_readoutModuleSvc;
  RodRobIdMap m_rodRobIdMap;
};

#endif 
