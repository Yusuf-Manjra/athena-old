/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
//  InvariantMassDeltaPhiInclusive2Charge.h
//  TopoCore
//  Based on InvariantMassInclusive2 and DeltaPhiIncl2 by Paula Martinez on 11/02/2021. For questions contact atlas-trig-l1topo-algcom@cern.ch. 
//  TO DO size of the input list to be possbly refined 

#ifndef L1TOPOALGORITHMS_INVARIANTMASSDELTAPHIINCLUSIVE2CHARGE_H
#define L1TOPOALGORITHMS_INVARIANTMASSDELTAPHIINCLUSIVE2CHARGE_H

#include "L1TopoInterfaces/DecisionAlg.h"

class TH2;

namespace TCS {
   
   class InvariantMassDeltaPhiInclusive2Charge : public DecisionAlg {
   public:
      InvariantMassDeltaPhiInclusive2Charge(const std::string & name);
      virtual ~InvariantMassDeltaPhiInclusive2Charge();

      virtual StatusCode initialize() override final;
 
      virtual StatusCode processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison ) override final;

      
      virtual StatusCode process( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison ) override final;
      

   private:

      parType_t      p_NumberLeading1 = { 0 };
      parType_t      p_NumberLeading2 = { 0 };
      parType_t      p_InvMassMin[6] = { 0,0,0,0,0,0 };
      parType_t      p_InvMassMax[6] = { 0,0,0,0,0,0 };
      parType_t      p_MinET1[6] = { 0,0,0,0,0,0 };
      parType_t      p_MinET2[6] = { 0,0,0,0,0,0 };
      parType_t      p_ApplyEtaCut = { 0 };
      parType_t      p_MinEta1 = { 0 };
      parType_t      p_MaxEta1 = { 0 };
      parType_t      p_MinEta2 = { 0 };
      parType_t      p_MaxEta2 = { 0 };
      parType_t      p_DeltaPhiMin[6] = { 0,0,0,0,0,0 };
      parType_t      p_DeltaPhiMax[6] = { 0,0,0,0,0,0 };

   };
   
}

#endif
