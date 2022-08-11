/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1CALOBYTESTREAM_JEPREADBYTESTREAMV1V2CNV_H
#define TRIGT1CALOBYTESTREAM_JEPREADBYTESTREAMV1V2CNV_H

#include <string>

#include "AthenaBaseComps/AthConstConverter.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

class DataObject;
class IByteStreamEventAccess;
class IOpaqueAddress;
class IROBDataProviderSvc;
class ISvcLocator;
class StatusCode;

template <typename> class CnvFactory;


namespace LVL1BS {

class JepByteStreamV1Tool;
class JepByteStreamV2Tool;

/** ByteStream converter for JEP component containers which are unchanged
 *  post-LS1.  Allows for data containing pre- or post-LS1 format sub-blocks.
 *  For reading only.
 *
 *  @author Peter Faulkner
 */

template <typename Container>
class JepReadByteStreamV1V2Cnv: public AthConstConverter {

public:
  JepReadByteStreamV1V2Cnv(ISvcLocator* svcloc);

  virtual ~JepReadByteStreamV1V2Cnv();

  virtual StatusCode initialize() override;
  /// Create Container from ByteStream
  virtual StatusCode createObjConst (IOpaqueAddress* pAddr, DataObject*& pObj) const override;

  //  Storage type and class ID
  virtual long repSvcType() const override { return i_repSvcType(); }
  static  long storageType();
  static const CLID& classID();

private:
  /// Tool that does the actual work pre-LS1
  ToolHandle<LVL1BS::JepByteStreamV1Tool> m_tool1;
  /// Tool that does the actual work post-LS1
  ToolHandle<LVL1BS::JepByteStreamV2Tool> m_tool2;

  /// Service for reading bytestream
  ServiceHandle<IROBDataProviderSvc> m_robDataProvider;
};

} // end namespace

#include "JepReadByteStreamV1V2Cnv.icc"

#endif
