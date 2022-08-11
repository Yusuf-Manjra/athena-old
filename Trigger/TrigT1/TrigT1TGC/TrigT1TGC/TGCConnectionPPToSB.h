/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

//  Table of connection between Patch Panel and Slave Board.
#ifndef TGCConnectionPPToSB_hh
#define TGCConnectionPPToSB_hh

#include "TrigT1TGC/TGCBoardConnection.h"
#include "TrigT1TGC/TGCSector.h"

namespace LVL1TGCTrigger {

class TGCConnectionPPToSB : public TGCBoardConnection
{
 public:
  int getSBIdToPP(int type, int port, int index) const;
  void setSBIdToPP(int type, int port, int index, int idIn);
  int getNumberOfPort() const { return NumberOfPPPort; };

  TGCConnectionPPToSB();
  ~TGCConnectionPPToSB();
  TGCConnectionPPToSB(const TGCConnectionPPToSB& right);
  TGCConnectionPPToSB& operator=(const TGCConnectionPPToSB& right);

 private:
  enum{ NumberOfPPPort = 2 };
  int* m_SBIdToPP[NumberOfPPPort][TGCSector::NumberOfPatchPanelType];
};

inline
int TGCConnectionPPToSB::getSBIdToPP(int type, int port, int index) const
{
  return m_SBIdToPP[port][type][index];
}

inline
void TGCConnectionPPToSB::setSBIdToPP(int type, int port, int index, int idIn)
{
  if(m_SBIdToPP[port][type]==0)
    m_SBIdToPP[port][type] = new int [m_numberOfBoard[type]];
  m_SBIdToPP[port][type][index] = idIn;
}

inline TGCPatchPanelType operator++(TGCPatchPanelType &rs, int)
{
    return rs = (TGCPatchPanelType)(rs + 1);
}


} //end of namespace bracket

#endif // TGCConnectionPPToSB_hh
