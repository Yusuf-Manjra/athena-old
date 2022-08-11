/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRecConditions/LArCalibLineMapping.h"
#include "LArIdentifier/LArOnlineID_Base.h"


LArCalibLineMapping::LArCalibLineMapping(const LArOnlineID_Base* onlineId) :
  m_onlineId(onlineId)

{}


const std::vector<HWIdentifier>& LArCalibLineMapping::calibSlotLine(const HWIdentifier id) const  {
  const IdentifierHash hid=m_onlineId->channel_Hash(id);
  return calibSlotLineFromHash(hid);
}
