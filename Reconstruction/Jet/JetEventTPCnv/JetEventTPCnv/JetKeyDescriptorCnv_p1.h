/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETEVENT_JETKEYDESCRIPTORCNV_P1_H
#define JETEVENT_JETKEYDESCRIPTORCNV_P1_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include <string>
#include <vector>

#include "JetEvent/JetKeyDescriptor.h"
#include "JetEventTPCnv/JetKeyDescriptor_p1.h"

class JetKeyDescriptorCnv_p1 : public T_AthenaPoolTPCnvConstBase<JetKeyDescriptor, JetKeyDescriptor_p1>
{ 
 public: 
  using base_class::transToPers;
  using base_class::persToTrans;


  JetKeyDescriptorCnv_p1() { };
  
  /** Method creating the transient representation of @c JetKeyDescriptor
   *  from its persistent representation @c JetKeyDescriptor_p1
   */
  virtual void persToTrans( const JetKeyDescriptor_p1* persObj, 
                            JetKeyDescriptor* transObj, 
                            MsgStream& msg ) const override;
      


  /** Method creating the persistent representation @c JetKeyDescriptor_p1
   *  from its transient representation @c JetKeyDescriptor
   */
  virtual void transToPers( const JetKeyDescriptor* transObj,
                            JetKeyDescriptor_p1* persObj, 
                            MsgStream& msg ) const override;
  
  /* JetKeyDescriptor *createTransient(const JetKeyDescriptor_p1* pers, MsgStream &msg ){ this-> } */
}; 

#endif
