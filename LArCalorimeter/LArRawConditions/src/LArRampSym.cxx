/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRawConditions/LArRampSym.h"
#include "LArRawConditions/LArRampMC.h"
#include "LArRawConditions/LArMCSym.h"


LArRampSym::LArRampSym(const LArMCSym* mcSym,  const LArRampMC* rampComplete) :
  m_mcSym(mcSym),
  m_rampComplete(rampComplete) //Implicit cast to base-class
{}

LArRampSym::~LArRampSym() {}
 
ILArRamp::RampRef_t LArRampSym::ADC2DAC(const HWIdentifier&  hwid, int gain) const {
  HWIdentifier symhwid=m_mcSym->ZPhiSymOnl(hwid);
  return m_rampComplete->get(symhwid,gain).m_vRamp;
}
  
