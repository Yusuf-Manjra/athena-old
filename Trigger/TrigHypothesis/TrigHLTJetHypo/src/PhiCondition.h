/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_PHICONDITIONSIGNED_H
#define TRIGHLTJETHYPO_PHICONDITIONSIGNED_H


/********************************************************************
 *
 * NAME:     PhiConditionSigned.h
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

class PhiCondition: public ICondition{
 public:
  PhiCondition(double phiMin,
		       double phiMax);

  bool isSatisfied(const HypoJetVector&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;

  virtual unsigned int capacity() const override{return s_capacity;}
  std::string toString() const override;
  
 private:

  double m_min;
  double m_max;
  bool isSatisfied(const pHypoJet&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const;
  
  const static  unsigned int s_capacity{1};
  
};

#endif
