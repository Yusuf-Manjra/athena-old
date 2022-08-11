/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//  gJetSelect.h
//  TopoCore

#ifndef __TopoCore__gJetSelect__
#define __TopoCore__gJetSelect__

#include "L1TopoInterfaces/SortingAlg.h"
#include "L1TopoEvent/TOBArray.h"

#include <iostream>
#include <vector>

namespace TCS {
   
   class gJetSelect : public SortingAlg {
   public:
      
      // constructor
      gJetSelect(const std::string & name);

      // destructor
      virtual ~gJetSelect();
      virtual TCS::StatusCode initialize() override;
      virtual TCS::StatusCode sort(const InputTOBArray & input, TOBArray & output) override final;    
    
   private:
   
      parType_t      m_numberOfJets = { 0 };
      parType_t      m_minEta = { 0 };
      parType_t      m_maxEta = { 0 };
      parType_t      m_et = { 0 };
    
   };

} // end of namespace TCS

#endif /* defined(__TopoCore__SortingAlg__) */
