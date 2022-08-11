/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
#
#include "./EtCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJet.h"

#include <sstream>
#include <cmath>
#include <TLorentzVector.h>

EtCondition::EtCondition(double threshold) : m_min(threshold) {
}


bool EtCondition::isSatisfied(const pHypoJet& ip,
				const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const {

  auto et = ip->et();
  bool pass = m_min <= et;

  
  if(collector){
    const void* address = static_cast<const void*>(this);

    std::stringstream ss0;
    ss0 << "EtCondition: (" << address << ") " 
        << " et thresh " << m_min
        << " pass: "  << std::boolalpha << pass << '\n';

    auto j_addr = static_cast<const void*>(ip.get());
    std::stringstream ss1;
    ss1 <<  "     jet : ("<< j_addr << ")"
        " et " << et << '\n';
    
    collector->collect(ss0.str(), ss1.str());

  }
  return pass;
}


bool 
EtCondition::isSatisfied(const HypoJetVector& ips,
			   const std::unique_ptr<ITrigJetHypoInfoCollector>& c) const {
  auto result =  isSatisfied(ips[0], c);
  return result;
}


std::string EtCondition::toString() const {
  std::stringstream ss;
  ss << "EtCondition (" << this << ") "
     << " Et threshold: " 
     << m_min
     <<'\n';

  return ss.str();
}
