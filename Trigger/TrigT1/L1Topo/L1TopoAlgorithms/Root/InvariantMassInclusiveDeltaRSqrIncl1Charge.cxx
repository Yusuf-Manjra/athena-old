/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * Created by Paula Martinez based on V Sorin and Joerg Stelzer.
 * 
 * @brief algorithm calculates the sqr of the INVMASS and DeltaR between one list and applies invmass and deltaR criteria
 * Events containing a pair of TGC muons with same charge are rejected
 *
 * @param NumberLeading
 * 
 * For questions contact atlas-trig-l1topo-algcom@cern.ch. 
**********************************/

#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"

#include "L1TopoAlgorithms/InvariantMassInclusiveDeltaRSqrIncl1Charge.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"
// Bitwise implementation utils
#include "L1TopoSimulationUtils/L1TopoDataTypes.h"
#include "L1TopoSimulationUtils/Trigo.h"
#include "L1TopoSimulationUtils/Hyperbolic.h"
#include "L1TopoSimulationUtils/Kinematics.h"

//

REGISTER_ALG_TCS(InvariantMassInclusiveDeltaRSqrIncl1Charge)


TCS::InvariantMassInclusiveDeltaRSqrIncl1Charge::InvariantMassInclusiveDeltaRSqrIncl1Charge(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth", 3);
   defineParameter("MaxTob", 0); 
   defineParameter("NumResultBits", 6);
   defineParameter("RequireOneBarrel", 0);
   defineParameter("MinMSqr",  0, 0);
   defineParameter("MaxMSqr", 999, 0);
   defineParameter("MinMSqr",  0, 1);
   defineParameter("MaxMSqr",  999, 1);
   defineParameter("MinMSqr", 0, 2);
   defineParameter("MaxMSqr", 999, 2);
   defineParameter("MinMSqr", 0, 3);
   defineParameter("MaxMSqr", 999, 3);
   defineParameter("MinMSqr", 0, 4);
   defineParameter("MaxMSqr", 999, 4);
   defineParameter("MinMSqr", 0, 5);
   defineParameter("MaxMSqr", 999, 5);
   defineParameter("MinET1",0,0);
   defineParameter("MinET2",0,0);
   defineParameter("MinET1",0,1);
   defineParameter("MinET2",0,1);
   defineParameter("MinET1",0,2);
   defineParameter("MinET2",0,2);
   defineParameter("MinET1",0,3);
   defineParameter("MinET2",0,3);
   defineParameter("MinET1",0,4);
   defineParameter("MinET2",0,4);
   defineParameter("MinET1",0,5);
   defineParameter("MinET2",0,5);  
   defineParameter("DeltaRMin", 0, 0);
   defineParameter("DeltaRMax", 0, 0);
   defineParameter("DeltaRMin", 0, 1);
   defineParameter("DeltaRMax", 0, 1);
   defineParameter("DeltaRMin", 0, 2);
   defineParameter("DeltaRMax", 0, 2);
   defineParameter("DeltaRMin", 0, 3);
   defineParameter("DeltaRMax", 0, 3);
   defineParameter("DeltaRMin", 0, 4);
   defineParameter("DeltaRMax", 0, 4);
   defineParameter("DeltaRMin", 0, 5);
   defineParameter("DeltaRMax", 0, 5);

 
   setNumberOutputBits(6);
}

TCS::InvariantMassInclusiveDeltaRSqrIncl1Charge::~InvariantMassInclusiveDeltaRSqrIncl1Charge(){}


TCS::StatusCode
TCS::InvariantMassInclusiveDeltaRSqrIncl1Charge::initialize() {
   if(parameter("MaxTob").value() > 0) {
      p_NumberLeading1 = parameter("MaxTob").value();
      p_NumberLeading2 = parameter("MaxTob").value();
   } else {
      p_NumberLeading1 = parameter("InputWidth").value();
      p_NumberLeading2 = parameter("InputWidth").value();
   }

   p_OneBarrel = parameter("RequireOneBarrel").value();

   TRG_MSG_INFO("NumberLeading1 : " << p_NumberLeading1);
   TRG_MSG_INFO("NumberLeading2 : " << p_NumberLeading2);
   TRG_MSG_INFO("RequireOneBarrel : " << p_OneBarrel);

   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      p_InvMassMin[i] = parameter("MinMSqr", i).value();
      p_InvMassMax[i] = parameter("MaxMSqr", i).value();
      p_DeltaRMin[i] = parameter("DeltaRMin", i).value();
      p_DeltaRMax[i] = parameter("DeltaRMax", i).value();
      p_MinET1[i] = parameter("MinET1",i).value();
      p_MinET2[i] = parameter("MinET2",i).value();

      TRG_MSG_INFO("InvMassMin "<< i << "  : " << p_InvMassMin[i]);
      TRG_MSG_INFO("InvMassMax "<< i << "  : " << p_InvMassMax[i]);
      TRG_MSG_INFO("MinET1     "<< i << "  : " << p_MinET1[i]);
      TRG_MSG_INFO("MinET2     "<< i << "  : " << p_MinET2[i]);
      TRG_MSG_INFO("DeltaRMin  "<< i << "  : " << p_DeltaRMin[i]);
      TRG_MSG_INFO("DeltaRMax  "<< i << "  : " << p_DeltaRMax[i]);
   }

   TRG_MSG_INFO("number output : " << numberOutputBits());

   // book histograms
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
       std::string hname_accept = "hInvariantMassInclusiveDeltaRSqrIncl1Charge_accept_bit"+std::to_string(static_cast<int>(i));
       std::string hname_reject = "hInvariantMassInclusiveDeltaRSqrIncl1Charge_reject_bit"+std::to_string(static_cast<int>(i));
       // mass
       bookHist(m_histAcceptM, hname_accept, "INVM vs DR", 100, std::sqrt(p_InvMassMin[i]), std::sqrt(p_InvMassMax[i]), 100, std::sqrt(p_DeltaRMin[i]), std::sqrt(p_DeltaRMax[i]));
       bookHist(m_histRejectM, hname_reject, "INVM vs DR", 100, std::sqrt(p_InvMassMin[i]), std::sqrt(p_InvMassMax[i]), 100, std::sqrt(p_DeltaRMin[i]), std::sqrt(p_DeltaRMax[i]));
       // eta2 vs. eta1
       bookHist(m_histAcceptEta1Eta2, hname_accept, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
       bookHist(m_histRejectEta1Eta2, hname_reject, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
       
   }

   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::InvariantMassInclusiveDeltaRSqrIncl1Charge::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{

   if(input.size() == 1) {     
     
      for( TOBArray::const_iterator tob1 = input[0]->begin(); 
           tob1 != input[0]->end() && distance( input[0]->begin(), tob1) < p_NumberLeading1;
           ++tob1) 
         {
            

            TCS::TOBArray::const_iterator tob2 = tob1; ++tob2;      
            for( ;
                 tob2 != input[0]->end() && distance( input[0]->begin(), tob2) < p_NumberLeading2;
                 ++tob2) {


               // OneBarrel
               if (p_OneBarrel && parType_t(std::abs((*tob1)->eta())) > 10 && parType_t(std::abs((*tob2)->eta())) > 10 ) continue;
               
               // Inv Mass calculation
               unsigned int invmass2 = calcInvMassBW( *tob1, *tob2 );
	       // test DeltaR2Min, DeltaR2Max                                                                                                
	       unsigned int deltaR2 = calcDeltaR2BW( *tob1, *tob2 );
	       TRG_MSG_DEBUG("Jet1 = " << **tob1 << ", Jet2 = " << **tob2 << ", invmass2 = " << invmass2 << ", deltaR2 = " << deltaR2);
                // Charge cut ( 1 = positive, -1 = negative, 0 = undefined (RPC) )
               int charge1 = (*tob1)->charge();
               int charge2 = (*tob2)->charge();
               int totalCharge = charge1 + charge2;
               bool acceptCharge = true;
               if ( std::abs(totalCharge) == 2 ) { acceptCharge = false; }
               for(unsigned int i=0; i<numberOutputBits(); ++i) {
                   bool accept = false;
                   if( parType_t((*tob1)->Et()) <= std::min(p_MinET1[i],p_MinET2[i])) continue; // ET cut
                   if( parType_t((*tob2)->Et()) <= std::min(p_MinET1[i],p_MinET2[i])) continue; // ET cut
                   if( (parType_t((*tob1)->Et()) <= std::max(p_MinET1[i],p_MinET2[i])) && (parType_t((*tob2)->Et()) <= std::max(p_MinET1[i],p_MinET2[i]))) continue;
                   accept = invmass2 >= p_InvMassMin[i] && invmass2 <= p_InvMassMax[i] && deltaR2 >= p_DeltaRMin[i] && deltaR2 <= p_DeltaRMax[i] && acceptCharge;
                   const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
                   const bool fillReject = fillHistos() and not fillAccept;
                   const bool alreadyFilled = decision.bit(i);
                   if( accept ) {
                       decision.setBit(i, true);
                       output[i]->push_back( TCS::CompositeTOB(*tob1, *tob2) );
                   }
                   if(fillAccept and not alreadyFilled) {
		     fillHist2D(m_histAcceptM[i],std::sqrt(static_cast<float>(invmass2)),std::sqrt(static_cast<float>(deltaR2)));
                     fillHist2D(m_histAcceptEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                   } else if(fillReject) {
		     fillHist2D(m_histRejectM[i],std::sqrt(static_cast<float>(invmass2)),std::sqrt(static_cast<float>(deltaR2)));
                     fillHist2D(m_histRejectEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                   }
                   TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " invmass2 = " << invmass2 << " deltaR2 = " << deltaR2 );
               }
            }
         }
   } else {

      TCS_EXCEPTION("InvariantMassInclusiveDeltaRSqrIncl1Charge alg must have either 1  inputs, but got " << input.size());

   }

   return TCS::StatusCode::SUCCESS;

}

TCS::StatusCode
TCS::InvariantMassInclusiveDeltaRSqrIncl1Charge::process( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{

   if(input.size() == 1) {     
      for( TOBArray::const_iterator tob1 = input[0]->begin(); 
           tob1 != input[0]->end() && distance( input[0]->begin(), tob1) < p_NumberLeading1;
           ++tob1) 
         {
            

            TCS::TOBArray::const_iterator tob2 = tob1; ++tob2;      
            for( ;
                 tob2 != input[0]->end() && distance( input[0]->begin(), tob2) < p_NumberLeading2;
                 ++tob2) {


               // OneBarrel
               if (p_OneBarrel && parType_t(std::abs((*tob1)->eta())) > 10 && parType_t(std::abs((*tob2)->eta())) > 10 ) continue;
               
               // Inv Mass calculation
	       unsigned int invmass2 = calcInvMass( *tob1, *tob2 );
	       // test DeltaR2Min, DeltaR2Max                                                                                                  
	       unsigned int deltaR2 = calcDeltaR2( *tob1, *tob2 );
	       TRG_MSG_DEBUG("Jet1 = " << **tob1 << ", Jet2 = " << **tob2 << ", invmass2 = " << invmass2 << ", deltaR2 = " << deltaR2);
               // Charge cut ( 1 = positive, -1 = negative, 0 = undefined (RPC) )
               int charge1 = (*tob1)->charge();
               int charge2 = (*tob2)->charge();
               int totalCharge = charge1 + charge2;
               bool acceptCharge = true;
               if ( std::abs(totalCharge) == 2 ) { acceptCharge = false; }	       
               for(unsigned int i=0; i<numberOutputBits(); ++i) {
                   bool accept = false;
                  if( parType_t((*tob1)->Et()) <= std::min(p_MinET1[i],p_MinET2[i])) continue; // ET cut
                  if( parType_t((*tob2)->Et()) <= std::min(p_MinET1[i],p_MinET2[i])) continue; // ET cut
                  if( (parType_t((*tob1)->Et()) <= std::max(p_MinET1[i],p_MinET2[i])) && (parType_t((*tob2)->Et()) <= std::max(p_MinET1[i],p_MinET2[i]))) continue;
                  accept = invmass2 >= p_InvMassMin[i] && invmass2 <= p_InvMassMax[i] && deltaR2 >= p_DeltaRMin[i] && deltaR2 <= p_DeltaRMax[i] && acceptCharge; 
                  const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
                  const bool fillReject = fillHistos() and not fillAccept;
                  const bool alreadyFilled = decision.bit(i);
                  if( accept ) {
                      decision.setBit(i, true);
                      output[i]->push_back( TCS::CompositeTOB(*tob1, *tob2) );
                  }
                   if(fillAccept and not alreadyFilled) {
		     fillHist2D(m_histAcceptM[i],std::sqrt(static_cast<float>(invmass2)),std::sqrt(static_cast<float>(deltaR2)));
                     fillHist2D(m_histAcceptEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                   } else if(fillReject) {
		     fillHist2D(m_histRejectM[i],std::sqrt(static_cast<float>(invmass2)),std::sqrt(static_cast<float>(deltaR2)));
                     fillHist2D(m_histRejectEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                   }
                  TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " invmass2 = " << invmass2 << " deltaR2 = " << deltaR2 );
               }
            }
         }
   } else {

      TCS_EXCEPTION("InvariantMassInclusiveDeltaRSqrIncl1Charge alg must have either 1  inputs, but got " << input.size());

   }

   return TCS::StatusCode::SUCCESS;
}
