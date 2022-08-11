/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARTPCNV_LARDIGITCONTAINERCNV_P1_H
#define LARTPCNV_LARDIGITCONTAINERCNV_P1_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "LArTPCnv/LArRawSCContainer_p1.h"

class MsgStream;
class LArRawSCContainer;

class LArRawSCContainerCnv_p1  : public T_AthenaPoolTPCnvConstBase<LArRawSCContainer, LArRawSCContainer_p1>
{
public:

  LArRawSCContainerCnv_p1() {}
  using base_class::persToTrans;
  using base_class::transToPers;

  virtual void          persToTrans(const LArRawSCContainer_p1* pers, LArRawSCContainer* trans, MsgStream &log) const override;
  virtual void          transToPers(const LArRawSCContainer* trans, LArRawSCContainer_p1* pers, MsgStream &log) const override;
};

#endif
