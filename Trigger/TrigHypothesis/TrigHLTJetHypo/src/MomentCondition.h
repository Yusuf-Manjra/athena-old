/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_MOMENTCONDITION_H
#define TRIGHLTJETHYPO_MOMENTCONDITION_H

/********************************************************************
 *
 * NAME:     MomentCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR:   P. Sherwood
 *********************************************************************/

#include <string>
#include "./ICondition.h"

namespace HypoJet{
  class IJet;
}

class ITrigJetHypoInfoCollector;

class MomentCondition: public ICondition{
 public:
  MomentCondition(double mMin, double mMax, const std::string& moment);
  
  bool isSatisfied(const HypoJetVector&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;

  virtual unsigned int capacity() const override{return s_capacity;}


  std::string toString() const override;
 private:
  
  double m_min;
  double m_max;
  std::string m_moment;
  
  bool isSatisfied(const pHypoJet&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const;
  
  const static  unsigned int s_capacity{1};
  
};

#endif
