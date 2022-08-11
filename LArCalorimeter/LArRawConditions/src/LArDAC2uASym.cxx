/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawConditions/LArDAC2uASym.h"
#include "LArRawConditions/LArDAC2uAMC.h"
#include "LArRawConditions/LArMCSym.h"


LArDAC2uASym::LArDAC2uASym(const LArMCSym* mcSym,  const LArDAC2uAMC* DAC2uAComplete) :
  m_mcSym(mcSym),
  m_DAC2uAComplete(DAC2uAComplete) //Implicit cast to base-class
{}

LArDAC2uASym::~LArDAC2uASym() {}
 
const float&  LArDAC2uASym::DAC2UA(const HWIdentifier&  hwid) const {
  HWIdentifier symhwid=m_mcSym->ZPhiSymOnl(hwid);
  return m_DAC2uAComplete->get(symhwid).m_data;
}
  

