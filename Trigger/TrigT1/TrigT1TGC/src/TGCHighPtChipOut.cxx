/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCHighPtChipOut.h"
#include "TrigT1TGC/TGCHighPtBoard.h"

namespace LVL1TGCTrigger {

TGCHighPtChipOut::TGCHighPtChipOut(TGCHighPtBoard* hpt, int bidIn)
  :m_origin(hpt), m_bid(bidIn)
{
  clear();
}

TGCHighPtChipOut::TGCHighPtChipOut()
  :m_origin(0), m_bid(-1)
{
  clear();
}

TGCHighPtChipOut::TGCHighPtChipOut(const TGCHighPtChipOut& right )
  :m_origin(right.m_origin), m_bid(right.m_bid)
{
  for(int i=0; i<NumberOfChip ; i+=1) {
    for(int j=0; j<s_NHitInTrackSelector; j+=1){
      m_pt[i][j]     = right.m_pt[i][j];
      m_pos[i][j]    = right.m_pos[i][j];
      m_hitID[i][j]  = right.m_hitID[i][j];
      m_dev[i][j]    = right.m_dev[i][j];
      m_select[i][j] = right.m_select[i][j];
    }
  }
}

TGCHighPtChipOut& TGCHighPtChipOut::operator=(const TGCHighPtChipOut& right )
{
  if (this != &right){
    m_origin = right.m_origin;
    m_bid    = right.m_bid;
    for(int i=0; i<NumberOfChip ; i+=1) {
      for(int j=0; j<s_NHitInTrackSelector; j+=1){
        m_pt[i][j]     = right.m_pt[i][j];
        m_pos[i][j]    = right.m_pos[i][j];
        m_hitID[i][j]  = right.m_hitID[i][j];
        m_dev[i][j]    = right.m_dev[i][j];
        m_select[i][j] = right.m_select[i][j];
      }
    }
  }
  return *this;
}
TGCHighPtChipOut::~TGCHighPtChipOut()
{
  m_origin=0;
  m_bid = -1;
  clear();
}


void TGCHighPtChipOut::clear()
{
  for(int i=0; i<NumberOfChip ; i+=1) {
    for(int j=0; j<s_NHitInTrackSelector; j+=1){
      m_pt[i][j]     = 0;
      m_pos[i][j]    = 0;
      m_hitID[i][j]  = 0;
      m_dev[i][j]    = 0;
      m_select[i][j] = 0;
    }
  }
}

} //end of namespace bracket
