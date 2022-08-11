//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARBYTESTREAM_LARRAWSCCONTRAWEVENTCNV_H
#define LARBYTESTREAM_LARRAWSCCONTRAWEVENTCNV_H

#include <stdint.h>
#include <map>
#include <string>
#include "GaudiKernel/Converter.h"
#include "ByteStreamData/RawEvent.h" 
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h" 

class DataObject;
class StatusCode;
class IAddressCreator;
class IByteStreamEventAccess;
class StoreGateSvc; 
class MsgStream; 
class LArDigitContainer; 
class IROBDataProviderSvc; 
class ByteStreamCnvSvc;
class LArABBADecoder;
class LArLATOMEDecoder;

// Abstract factory to create the converter
template <class TYPE> class CnvFactory;

class LArRawScContByteStreamCnv: public Converter {

public: 
  LArRawScContByteStreamCnv(ISvcLocator* svcloc);
  virtual ~LArRawScContByteStreamCnv();

  virtual StatusCode initialize();
  virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj); 
  virtual StatusCode createRep(DataObject* pObj, IOpaqueAddress*& pAddr);

  /// Storage type and class ID
  virtual long repSvcType() const { return i_repSvcType(); }
  static long storageType()     { return ByteStreamAddress::storageType(); }
  static const CLID& classID();

private: 

   MsgStream* m_log;
   LArLATOMEDecoder* m_scTool = nullptr;
   ByteStreamCnvSvc* m_ByteStreamEventAccess; 
   IROBDataProviderSvc *m_rdpSvc;
   StoreGateSvc* m_storeGate; 
};
#endif



