/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ILArBadFebMasker_H
#define ILArBadFebMasker_H

#include "GaudiKernel/IAlgTool.h"
#include "CaloIdentifier/CaloGain.h"

class HWIdentifier;
class LArBadFeb;

class ILArBadFebMasker : public virtual IAlgTool
{
public:
  DeclareInterfaceID (ILArBadFebMasker, 1, 0);

   virtual ~ILArBadFebMasker()
   {}

   virtual bool febMissing(const HWIdentifier& febId)  const = 0;
   virtual bool shouldbeMasked(const HWIdentifier& febId) const = 0; 
   virtual  LArBadFeb febStatus(const HWIdentifier& febId) const =0;

   virtual bool isMaskingOn() const = 0;
};

#endif
