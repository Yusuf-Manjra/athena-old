/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#
#include "./PhiCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJet.h"

#include <sstream>
#include <cmath>
#include <TLorentzVector.h>

PhiCondition::PhiCondition(double phiMin,
			       double phiMax): m_min(phiMin), m_max(phiMax){
}


bool
PhiCondition::isSatisfied(const pHypoJet& ip,
			  const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const {
  
  auto phi = ip->phi();
  bool pass = m_min <= phi and m_max > phi;
  
  if(collector){
    const void* address = static_cast<const void*>(this);

    std::stringstream ss0;
    ss0 << "PhiCondition: (" << address << ") " 
        << " phi[" << m_min << ", " << m_max << "]" 
        << " pass: "  << std::boolalpha << pass << '\n';
    
    auto j_addr = static_cast<const void*>(ip.get());
    std::stringstream ss1;
    ss1 <<  "     jet : ("<< j_addr << ") phi " << phi << '\n';
    
    collector->collect(ss0.str(), ss1.str());
    
  }
  return pass;
}


bool 
PhiCondition::isSatisfied(const HypoJetVector& ips,
			  const std::unique_ptr<ITrigJetHypoInfoCollector>& c) const {
  auto result =  isSatisfied(ips[0], c);
  return result;
}


std::string PhiCondition::toString() const {
  std::stringstream ss;
  ss << "PhiCondition (" << this << ") phiMin "
     <<  m_min 
     << " phiMax " 
     << m_max 
     <<'\n';

  return ss.str();
}
