/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __TRIG_IN_DET_TRIPLET_H__
#define __TRIG_IN_DET_TRIPLET_H__

#include "TrigInDetEvent/TrigSiSpacePointBase.h"




class TrigInDetTriplet {

 public:
   TrigInDetTriplet() = delete; //to prevent creation w/o initialization

 static_assert(std::is_nothrow_move_constructible<TrigSiSpacePointBase>::value);
 TrigInDetTriplet(TrigSiSpacePointBase s1, TrigSiSpacePointBase s2, TrigSiSpacePointBase s3, float Q) :
    m_s1(std::move(s1)), m_s2(std::move(s2)), m_s3(std::move(s3)), m_Q(Q) {};

 TrigInDetTriplet(TrigInDetTriplet* t) :
    m_s1(t->m_s1), m_s2(t->m_s2), m_s3(t->m_s3), m_Q(t->m_Q) {};

  const TrigSiSpacePointBase& s1() const {return m_s1;}
  const TrigSiSpacePointBase& s2() const {return m_s2;}
  const TrigSiSpacePointBase& s3() const {return m_s3;}
  float Q() const {return m_Q;}
  void Q(double newQ) {m_Q = newQ;}

 protected:

  TrigSiSpacePointBase m_s1;
  TrigSiSpacePointBase m_s2;
  TrigSiSpacePointBase m_s3;
  float m_Q;//Quality
};

#endif // __TRIG_IN_DET_TRIPLET__
