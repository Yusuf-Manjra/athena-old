/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * MinDeltaPhiIncl2.cpp
 * Created by Veronica Sorin  on 14/8/14.
 *
 * @brief algorithm calculates the min phi-distance between  two lists and applies delta-phi criteria
 *
 * @param NumberLeading
**********************************/

#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "L1TopoAlgorithms/MinDeltaPhiIncl2.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"

REGISTER_ALG_TCS(MinDeltaPhiIncl2)

// not the best solution but we will move to athena where this comes for free
#define LOG std::cout << "TCS::MinDeltaPhiIncl2:     "

TCS::MinDeltaPhiIncl2::MinDeltaPhiIncl2(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth1", 9);
   defineParameter("InputWidth2", 9);
   defineParameter("MaxTob1", 0); 
   defineParameter("MaxTob2", 0); 
   defineParameter("NumResultBits", 3);
   defineParameter("DeltaPhiMin",  0, 0);
   defineParameter("DeltaPhiMin",  0, 1);
   defineParameter("DeltaPhiMin",  0, 2);
   defineParameter("MinET1",1);
   defineParameter("MinET2",1);
   setNumberOutputBits(3);
}

TCS::MinDeltaPhiIncl2::~MinDeltaPhiIncl2(){}


TCS::StatusCode
TCS::MinDeltaPhiIncl2::initialize() {
   p_NumberLeading1 = parameter("InputWidth1").value();
   p_NumberLeading2 = parameter("InputWidth2").value();
   if(parameter("MaxTob1").value() > 0) p_NumberLeading1 = parameter("MaxTob1").value();
   if(parameter("MaxTob2").value() > 0) p_NumberLeading2 = parameter("MaxTob2").value();

   for(unsigned int i=0; i< numberOutputBits(); ++i) {
      p_DeltaPhiMin[i] = parameter("DeltaPhiMin", i).value();
   }
   p_MinET1 = parameter("MinET1").value();
   p_MinET2 = parameter("MinET2").value();
   TRG_MSG_INFO("NumberLeading1 : " << p_NumberLeading1);
   TRG_MSG_INFO("NumberLeading2 : " << p_NumberLeading2);
   for(unsigned int i=0; i< numberOutputBits(); ++i) {
      TRG_MSG_INFO("DeltaPhiMin"<<i<<"   : " << p_DeltaPhiMin[i]);
   }
   TRG_MSG_INFO("MinET1         : " << p_MinET1);
   TRG_MSG_INFO("MinET2         : " << p_MinET2);
   TRG_MSG_INFO("nummber output : " << numberOutputBits());

   // book histograms
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
       std::string hname_accept = "hMinDeltaPhiIncl2_accept_bit"+std::to_string((int)i);
       std::string hname_reject = "hMinDeltaPhiIncl2_reject_bit"+std::to_string((int)i);
       // mass
       bookHist(m_histAccept, hname_accept, "DPHI", 100, p_DeltaPhiMin[i], 70);
       bookHist(m_histReject, hname_reject, "DPHI", 100, p_DeltaPhiMin[i], 70);
   }
   
   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::MinDeltaPhiIncl2::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                            const std::vector<TCS::TOBArray *> & output,
                            Decision & decision )
{

   // mindphi 
   unsigned int mindphi = *std::min_element(std::begin(p_DeltaPhiMin),std::end(p_DeltaPhiMin));
   bool firstphi = true;

   // declare iterator for the tob with min dphi
   TCS::TOBArray::const_iterator tobmin1,tobmin2;  
      
   if (input.size() == 2) {
   
      for( TOBArray::const_iterator tob1 = input[0]->begin(); 
           tob1 != input[0]->end() && distance(input[0]->begin(), tob1) < p_NumberLeading1;
           ++tob1)
         {

            if( parType_t((*tob1)->Et()) <= p_MinET1) continue; // ET cut

            for( TCS::TOBArray::const_iterator tob2 = input[1]->begin(); 
                 tob2 != input[1]->end() && distance(input[1]->begin(), tob2) < p_NumberLeading2;
                 ++tob2) {

               if( parType_t((*tob2)->Et()) <= p_MinET2) continue; // ET cut

               // test DeltaPhiMin, DeltaPhiMax
               unsigned int deltaPhi = calcDeltaPhiBW( *tob1, *tob2 );

               if (firstphi) {
                  mindphi = deltaPhi;
                  tobmin1=tob1;
                  tobmin2=tob2;
                  firstphi = false;
               }

               if ( deltaPhi < mindphi ) {
                  
                  mindphi = deltaPhi;
                  tobmin1=tob1;
                  tobmin2=tob2;

               }

            }
         }

      for(unsigned int i=0; i<numberOutputBits(); ++i) {
          bool accept = mindphi > p_DeltaPhiMin[i] ;
          const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
          const bool fillReject = fillHistos() and not fillAccept;
          const bool alreadyFilled = decision.bit(i);
          if( accept ) {
              decision.setBit(i, true);
              output[i]->push_back(TCS::CompositeTOB(*tobmin1, *tobmin2));
          }
          if(fillAccept and not alreadyFilled){
              fillHist1D(m_histAccept[i],(float)mindphi);
          } else if(fillReject){
              fillHist1D(m_histReject[i],(float)mindphi);
          }
          TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail"));
      } // for(i)
   } else {
      TCS_EXCEPTION("MinDeltaPhiIncl2 alg must have 2 inputs, but got " << input.size());
   }

   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::MinDeltaPhiIncl2::process( const std::vector<TCS::TOBArray const *> & input,
                            const std::vector<TCS::TOBArray *> & output,
                            Decision & decision )
{

   // mindphi 
   unsigned int mindphi = *std::min_element(std::begin(p_DeltaPhiMin),std::end(p_DeltaPhiMin));
   bool firstphi = true;

   // declare iterator for the tob with min dphi
   TCS::TOBArray::const_iterator tobmin1,tobmin2;  
      
   if (input.size() == 2) {
   
      for( TOBArray::const_iterator tob1 = input[0]->begin(); 
           tob1 != input[0]->end() && distance(input[0]->begin(), tob1) < p_NumberLeading1;
           ++tob1)
         {

            if( parType_t((*tob1)->Et()) <= p_MinET1) continue; // ET cut

            for( TCS::TOBArray::const_iterator tob2 = input[1]->begin(); 
                 tob2 != input[1]->end() && distance(input[1]->begin(), tob2) < p_NumberLeading2;
                 ++tob2) {

               if( parType_t((*tob2)->Et()) <= p_MinET2) continue; // ET cut

               // test DeltaPhiMin, DeltaPhiMax
               unsigned int deltaPhi = calcDeltaPhi( *tob1, *tob2 );

               if (firstphi) {
                  mindphi = deltaPhi;
                  tobmin1=tob1;
                  tobmin2=tob2;
                  firstphi = false;
               }

               if ( deltaPhi < mindphi ) {
                  
                  mindphi = deltaPhi;
                  tobmin1=tob1;
                  tobmin2=tob2;

               }

            }
         }

      for(unsigned int i=0; i<numberOutputBits(); ++i) {
          bool accept = mindphi > p_DeltaPhiMin[i] ;
          const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
          const bool fillReject = fillHistos() and not fillAccept;
          const bool alreadyFilled = decision.bit(i);
          if( accept) {
              decision.setBit(i, true);
              output[i]->push_back(TCS::CompositeTOB(*tobmin1, *tobmin2));
          }
          if(fillAccept and not alreadyFilled){
              fillHist1D(m_histAccept[i],(float)mindphi);
          } else if(fillReject) {
              fillHist1D(m_histReject[i],(float)mindphi);
          }
          TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail"));
      }
   } else {
      TCS_EXCEPTION("MinDeltaPhiIncl2 alg must have 2 inputs, but got " << input.size());
   }

   return TCS::StatusCode::SUCCESS;
}
