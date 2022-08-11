//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  LArRawDataContByteStreamTool.h
 * @author Remi Lafaye, Walter Lampl 
 * @date July, 2004
 * @brief Helper tool for conversion of raw data classes to/from LArByteStream 
 *        
 */
#ifndef LARBYTESTREAM_LARRAWDATACONTBYTESTREAMTOOL_H
#define LARBYTESTREAM_LARRAWDATACONTBYTESTREAMTOOL_H

#include <stdint.h>
#include "AthenaBaseComps/AthAlgTool.h"
#include "LArByteStream/Hid2RESrcID.h"
#include "LArByteStream/RodRobIdMap.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArFebRodMapping.h"
#include "ByteStreamCnvSvcBase/FullEventAssembler.h" 
#include "ByteStreamData/RawEvent.h" 
#include "LArByteStream/LArRodDecoder.h"

#include "CaloIdentifier/CaloGain.h"
#include "CaloConditions/CaloNoise.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CxxUtils/CachedUniquePtr.h"

#include "LArRawEvent/LArFebHeaderContainer.h"
// Map of ROBs need this
#include "eformat/index.h"

class LArRawChannelContainer; 
class LArDigitContainer;
class IByteStreamEventAccess;

/** 
 *@class LArRawDataContByteStreamTool
 *@brief An AlgTool class to help converting LAr raw data objects to/from ByteStream
 *
 * This tool is used for reading as well as writing. It's main prupose is to 
 * find all LAr-Fragment in the full byte-stream event
 */
      
class LArRawDataContByteStreamTool: public AthAlgTool {
public:
  using FEA_t = FullEventAssembler<RodRobIdMap>;

  /** Constructor
      Standard AlgTool constructor
  */
   LArRawDataContByteStreamTool( const std::string& type, const std::string& name,
                                 const IInterface* parent ) ;

  /** Destructor 
  */ 
  virtual ~LArRawDataContByteStreamTool() ;

  /** AlgTool InterfaceID
  */
  static const InterfaceID& interfaceID( ) ;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /** 
   * @brief Templated conversion class form Raw Event to a container
   * @param RawEvent: Full event in ByteStream format as defined in 
   * @c ByteStreamData/RawEvent.h
   * @param COLLECTION: Template for the raw data container. Can be the 
   * containers of LArRawChannel, LArDigit, LArCalibDigit, LArFebHeader 
   * or LArAccumulatedCalibDigit or LArAccumulatedDigit
   * @param gain: For fixed-gain mode (e.g. calibration digits), indicates 
   * which gain should be read
   * @return Gaudi StatusCode 
   * 
   * Iterates though all levels of the Atlas ByteStream format (aka eformat) 
   * to get all ROD fragments belonging to LAr. Methods from @c LArRodDecoder 
   * are used to deal with the individual ROD fragments. 
  */
  template <class COLLECTION, typename ...ARGS >
  StatusCode convert(const RawEvent* re,
                     COLLECTION* digit_cont,
                     CaloGain::CaloGain gain,
                     ARGS&&... args) const;

  /** 
   * @brief Fill channels from LArDigitContainer to a FullEvent
   * @param digit_cont Pointer to LArDigitContainer (input)
   * @param fea Pointer to FullEventAssember (output)
   * @return Gaudi StatusCode 
   */
  StatusCode WriteLArDigits(const LArDigitContainer* digit_cont,
                            FEA_t& fea) const;

 /** 
  * @brief Fill channels from LArCalibDigitContainer to a FullEvent
  * @param digit_cont Pointer to LArCalibDigitContainer (input)
  * @param fea Pointer to FullEventAssember (output)
  * @return Gaudi StatusCode 
  */
  StatusCode WriteLArCalibDigits(const LArCalibDigitContainer* digit_cont,
                                 FEA_t& fea) const;

  /** 
   * @brief Fill channels from LArRawChannelContainer to a FullEvent
   * @param digit_cont Pointer to LArRawChannelContainer (input)
   * @param fea Pointer to FullEventAssember (output)
   * @return Gaudi StatusCode 
   */
  StatusCode WriteLArRawChannels(const LArRawChannelContainer* CannelCont,
                                 FEA_t& fea) const;
 
private: 
  using RobIndex_t = std::map<eformat::SubDetectorGroup, std::vector<const uint32_t*> >;

  /** Prepare ROB index before conversion */
  StatusCode prepareRobIndex (const RawEvent* event, RobIndex_t& robIndex) const;
 
  /** Construct a RodBlockStructure instance of the proper concrete type. */
  std::unique_ptr<LArRodBlockStructure> makeRodBlockStructure() const;

  //StatusCode prepareWriting();
  /** 
   * @brief Check that all elements in a container have the same gain
   * @param coll Pointer to input container (template)
   * @return false if an incosistency is detected
   */
  template <class COLLECTION >
    bool checkGainConsistency(const COLLECTION* coll) const;
 
  const Hid2RESrcID& getHid2RESrcID (const LArFebRodMapping& rodMapping) const;

  CxxUtils::CachedUniquePtr<Hid2RESrcID> m_hid2re;       //!< Contains the mapping from channel to ROD (writing only)
  PublicToolHandle<LArRodDecoder> m_decoder
    { this, "LArRodDecoder", "LArRodDecoder", "LArRodDecoder" };

  /** Indicates which version of DSP code should be used for writing.
      This is equivalent to the DetectorEventType word in the ROD block header.
      Only for writing. 
   */
  unsigned m_DSPRunMode;

  /** Minor ROD block version
      This is equivalent to the lower 16 bits of the version word in the
      ROD block header. Only for writing.
   */
  unsigned short m_RodBlockVersion;

 /** JobOption to intitialize services needed only for writing
  */
  bool m_initializeForWriting;
  uint16_t m_subDetId;
  double m_nfebsigma;
  // want to process digits together with RawChannel
  bool m_includeDigits;
  // Name of Digit container to retrieve
  std::string m_DigitContName;

  const LArOnlineID*    m_onlineHelper = nullptr;

  SG::ReadCondHandleKey<CaloNoise> m_caloNoiseKey
  { this, "CaloNoiseKey", "totalNoise", "" };

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_onOffIdMappingKey
  { this, "OnOffIdMappingKey", "LArOnOffIdMap", "LArOnOffIdMap" };

  SG::ReadCondHandleKey<LArFebRodMapping> m_febRodMappingKey
  { this, "FebRodMappingKey", "LArFebRodMap", "LArFebRodMap" };

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey
  { this, "CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store" };
};


#include "LArByteStream/LArRawDataContByteStreamTool.icc" 

#endif
