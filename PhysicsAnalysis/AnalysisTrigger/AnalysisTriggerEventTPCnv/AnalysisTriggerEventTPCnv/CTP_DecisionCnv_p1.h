/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ANALYSISTRIGGEREVENTTPCNV_CTP_DECISIONCNV_P1_H
#define ANALYSISTRIGGEREVENTTPCNV_CTP_DECISIONCNV_P1_H

// Gaudi/Athena include(s):
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// Trigger include(s):
#include "AnalysisTriggerEvent/CTP_Decision.h"

// Local include(s):
#include "AnalysisTriggerEventTPCnv/CTP_Decision_p1.h"

/**
 *   @short T/P converter for CTP_Decision and CTP_Decision_p1
 *
 *          Simple converter that can create CTP_Decision objects from
 *          CTP_Decision_p1 ones, and vice versa. It is used in CTP_DecisionCnv
 *          to do the T/P conversion before/after writing/reading the CTP_Decision
 *          object.
 *
 * @version $Revision: 1.2 $
 *    @date $Date: 2007-07-30 03:48:54 $
 *  @author Attila Krasznahorkay Jr.
 */
class CTP_DecisionCnv_p1 : public T_AthenaPoolTPCnvBase< CTP_Decision, CTP_Decision_p1 > {

public:
  CTP_DecisionCnv_p1();

  virtual void persToTrans( const CTP_Decision_p1* persObj, CTP_Decision* transObj, MsgStream &log );
  virtual void transToPers( const CTP_Decision* transObj, CTP_Decision_p1* persObj, MsgStream &log );

}; // class CTP_DecisionCnv_p1

template<>
class T_TPCnv<CTP_Decision, CTP_Decision_p1>
  : public CTP_DecisionCnv_p1
{
public:
};

#endif // ANALYSISTRIGGEREVENTTPCNV_CTP_DECISIONCNV_P1_H
