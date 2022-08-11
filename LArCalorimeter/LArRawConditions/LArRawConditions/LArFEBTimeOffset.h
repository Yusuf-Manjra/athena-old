/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARFEBTIMEOFFSET_H
#define LARFEBTIMEOFFSET_H

#include "LArElecCalib/ILArFEBTimeOffset.h"
#include "AthenaKernel/CLASS_DEF.h" 
#include "Identifier/HWIdentifier.h"
#include <map>

class LArFEBTimeOffset: public ILArFEBTimeOffset{
 public:
  LArFEBTimeOffset();                                              //Constructor
  virtual ~LArFEBTimeOffset() {};                                  //Destructor 
  inline void setTimeOffset(const HWIdentifier fId, const float offset) //Setter
    {m_febTimeMap[fId]=offset;}    
  inline float TimeOffset(const HWIdentifier fId) const;                  //Getter
  inline float TimeOffset(const HWIdentifier fId, float defVal) const;

  inline unsigned size() const
    { return m_febTimeMap.size();}
  
  inline void setDefaultReturnValue(const float value)
    { m_defaultReturnValue=value; }

 private:
  std::map <HWIdentifier,float> m_febTimeMap;
  float m_defaultReturnValue = 0;
};

inline float LArFEBTimeOffset::TimeOffset(const HWIdentifier fId) const
{
  const std::map <HWIdentifier,float>::const_iterator it=m_febTimeMap.find(fId);
  if (it==m_febTimeMap.end())
    return m_defaultReturnValue;
  else
    return it->second;
}

inline float LArFEBTimeOffset::TimeOffset(const HWIdentifier fId,
                                          float defVal) const
{
  const std::map <HWIdentifier,float>::const_iterator it=m_febTimeMap.find(fId);
  if (it==m_febTimeMap.end())
    return defVal;
  else
    return it->second;
}

CLASS_DEF( LArFEBTimeOffset, 83217154, 1) 
#endif 

