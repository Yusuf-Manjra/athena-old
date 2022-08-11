/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * InvariantMassThreeTOBsIncl1Charge.cxx
 * Created by Paula Martinez based on InvariantMassInclusive1 by V Sorin
 * 
 * For questions contact atlas-trig-l1topo-algcom@cern.ch. 
 * @brief Algorithm calculates the sqr of the INVMASS of three TOBs from one  list and applies invmass criteria.
 * Events containing 3 TGC muons with same charge are rejected
 *
 * @param NumberLeading 
 *
 * First version assuming same cuts in all TOBs.
**********************************/

// TO DO size of the input list to be possibly refined 

#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "L1TopoAlgorithms/InvariantMassThreeTOBsIncl1Charge.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"


REGISTER_ALG_TCS(InvariantMassThreeTOBsIncl1Charge)


TCS::InvariantMassThreeTOBsIncl1Charge::InvariantMassThreeTOBsIncl1Charge(const std::string & name) : DecisionAlg(name)
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
   defineParameter("MinET1",0,1);
   defineParameter("MinET1",0,2);
   defineParameter("MinET1",0,3);
   defineParameter("MinET1",0,4);
   defineParameter("MinET1",0,5);
 
   setNumberOutputBits(6);
}

TCS::InvariantMassThreeTOBsIncl1Charge::~InvariantMassThreeTOBsIncl1Charge(){}


TCS::StatusCode
TCS::InvariantMassThreeTOBsIncl1Charge::initialize() {
  if(parameter("MaxTob").value() > 0) {
      p_NumberLeading1 = parameter("MaxTob").value();
   } else {
      p_NumberLeading1 = parameter("InputWidth").value();
   }

   p_OneBarrel = parameter("RequireOneBarrel").value();

   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      p_InvMassMin[i] = parameter("MinMSqr", i).value();
      p_InvMassMax[i] = parameter("MaxMSqr", i).value();
   
      p_MinET1[i] = parameter("MinET1",i).value();
   }
   TRG_MSG_INFO("NumberLeading1 : " << p_NumberLeading1);
   TRG_MSG_INFO("RequireOneBarrel : " << p_OneBarrel);
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      TRG_MSG_INFO("InvMassMin   : " << p_InvMassMin[i]);
      TRG_MSG_INFO("InvMassMax   : " << p_InvMassMax[i]);
   
      TRG_MSG_INFO("MinET1          : " << p_MinET1[i]);
   }
   TRG_MSG_INFO("number output : " << numberOutputBits());

   // book histograms
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
       std::string hname_accept = "hInvariantMassThreeTOBsIncl1Charge_accept_bit"+std::to_string(static_cast<int>(i));
       std::string hname_reject = "hInvariantMassThreeTOBsIncl1Charge_reject_bit"+std::to_string(static_cast<int>(i));
       std::string hname_acceptEta1Eta2 = "hInvariantMassThreeTOBsIncl1Charge_acceptEta1Eta2_bit"+std::to_string(static_cast<int>(i));
       std::string hname_rejectEta1Eta2 = "hInvariantMassThreeTOBsIncl1Charge_rejectEta1Eta2_bit"+std::to_string(static_cast<int>(i));
       std::string hname_acceptEta2Eta3 = "hInvariantMassThreeTOBsIncl1Charge_acceptEta2Eta3_bit"+std::to_string(static_cast<int>(i));
       std::string hname_rejectEta2Eta3 = "hInvariantMassThreeTOBsIncl1Charge_rejectEta2Eta3_bit"+std::to_string(static_cast<int>(i));
       std::string hname_acceptEta3Eta1 = "hInvariantMassThreeTOBsIncl1Charge_acceptEta3Eta1_bit"+std::to_string(static_cast<int>(i));
       std::string hname_rejectEta3Eta1 = "hInvariantMassThreeTOBsIncl1Charge_rejectEta3Eta1_bit"+std::to_string(static_cast<int>(i));
       // mass
       bookHist(m_histAccept, hname_accept, "INVM", 100, std::sqrt(p_InvMassMin[i]), std::sqrt(p_InvMassMax[i]));
       bookHist(m_histReject, hname_reject, "INVM", 100, std::sqrt(p_InvMassMin[i]), std::sqrt(p_InvMassMax[i]));
       // eta vs eta
       bookHist(m_histAcceptEta1Eta2, hname_acceptEta1Eta2, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
       bookHist(m_histRejectEta1Eta2, hname_rejectEta1Eta2, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
       bookHist(m_histAcceptEta2Eta3, hname_acceptEta2Eta3, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
       bookHist(m_histRejectEta2Eta3, hname_rejectEta2Eta3, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
       bookHist(m_histAcceptEta3Eta1, hname_acceptEta3Eta1, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
       bookHist(m_histRejectEta3Eta1, hname_rejectEta3Eta1, "ETA vs ETA", 100, -70, 70, 100, -70, 70);
   }
   

 
   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::InvariantMassThreeTOBsIncl1Charge::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
						 const std::vector<TCS::TOBArray *> & output,
						 Decision & decision )
{

   if(input.size() == 1) {     
     
      for( TCS::TOBArray::const_iterator tob1 = input[0]->begin(); 
           tob1 != input[0]->end() && distance( input[0]->begin(), tob1) < p_NumberLeading1;
           ++tob1) {
            
           TCS::TOBArray::const_iterator tob2 = tob1; ++tob2;      
           for( ;
                tob2 != input[0]->end() && distance( input[0]->begin(), tob2) < p_NumberLeading1;
                ++tob2) {

	        TCS::TOBArray::const_iterator tob3 = tob2; ++tob3;
	        for( ;
		     tob3 != input[0]->end() && distance( input[0]->begin(), tob3) < p_NumberLeading1;
		     ++tob3) {

		// OneBarrel
		if (p_OneBarrel && static_cast<parType_t>(std::abs((*tob1)->eta())) > 10 && static_cast<parType_t>(std::abs((*tob2)->eta())) > 10 && static_cast<parType_t>(std::abs((*tob3)->eta())) > 10 ) continue;
               
		// Inv Mass calculation
		unsigned int invmass2_12 = calcInvMassBW( *tob1, *tob2 );
		unsigned int invmass2_13 = calcInvMassBW( *tob1, *tob3 );
		unsigned int invmass2_23 = calcInvMassBW( *tob2, *tob3 );
		unsigned int invmass2 = invmass2_12 + invmass2_13 + invmass2_23;
                // Charge cut ( 1 = positive, -1 = negative, 0 = undefined (RPC) )
                int charge1 = (*tob1)->charge();
                int charge2 = (*tob2)->charge();
                int charge3 = (*tob3)->charge();
                int totalCharge = charge1 + charge2 + charge3;
                bool acceptCharge = true;
                if ( std::abs(totalCharge) == 3 ) { acceptCharge = false; }
		for(unsigned int i=0; i<numberOutputBits(); ++i) {
		  bool accept = false;
		  if( static_cast<parType_t>((*tob1)->Et()) <= p_MinET1[i]) continue; // ET cut
		  if( static_cast<parType_t>((*tob2)->Et()) <= p_MinET1[i]) continue; // ET cut
		  if( static_cast<parType_t>((*tob3)->Et()) <= p_MinET1[i]) continue; // ET cut

		  accept = invmass2 >= p_InvMassMin[i] && invmass2 <= p_InvMassMax[i] && acceptCharge;
		  const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
		  const bool fillReject = fillHistos() and not fillAccept;
		  const bool alreadyFilled = decision.bit(i);
		  if( accept ) {
		    std::vector<TCS::GenericTOB*> TOBvector;
		    TOBvector.push_back( *tob1 );
		    TOBvector.push_back( *tob2 );
		    TOBvector.push_back( *tob3 );
		    decision.setBit(i, true);
		    output[i]->push_back( TCS::CompositeTOB(TOBvector) );
		    TOBvector.clear();
		  }
		  if(fillAccept and not alreadyFilled) {
		    fillHist1D(m_histAccept[i],std::sqrt(invmass2));
                    fillHist2D(m_histAcceptEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                    fillHist2D(m_histAcceptEta2Eta3[i],(*tob2)->eta(),(*tob3)->eta());
                    fillHist2D(m_histAcceptEta3Eta1[i],(*tob3)->eta(),(*tob1)->eta());
		  } else if(fillReject) {
		    fillHist1D(m_histReject[i],std::sqrt(invmass2));
                    fillHist2D(m_histRejectEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                    fillHist2D(m_histRejectEta2Eta3[i],(*tob2)->eta(),(*tob3)->eta());
                    fillHist2D(m_histRejectEta3Eta1[i],(*tob3)->eta(),(*tob1)->eta());
		  }
		  TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " invmass2 = " << invmass2);
		  
		}
	      }
            }
          }
   } else {

      TCS_EXCEPTION("InvariantMassThreeTOBsIncl1Charge alg must have 1 input list, but got " << input.size());

   }

   return TCS::StatusCode::SUCCESS;

}

TCS::StatusCode
TCS::InvariantMassThreeTOBsIncl1Charge::process( const std::vector<TCS::TOBArray const *> & input,
					  const std::vector<TCS::TOBArray *> & output,
					  Decision & decision )
{

   if(input.size() == 1) {     

      for( TCS::TOBArray::const_iterator tob1 = input[0]->begin(); 
           tob1 != input[0]->end() && distance( input[0]->begin(), tob1) < p_NumberLeading1;
           ++tob1) {
            
           TCS::TOBArray::const_iterator tob2 = tob1; ++tob2;      
           for( ;
                tob2 != input[0]->end() && distance( input[0]->begin(), tob2) < p_NumberLeading1;
                ++tob2) {

	        TCS::TOBArray::const_iterator tob3 = tob2; ++tob3;
	        for( ;
		     tob3 != input[0]->end() && distance( input[0]->begin(), tob3) < p_NumberLeading1;
		     ++tob3) {

		// OneBarrel
		if (p_OneBarrel && static_cast<parType_t>(std::abs((*tob1)->eta())) > 10 && static_cast<parType_t>(std::abs((*tob2)->eta())) > 10 && static_cast<parType_t>(std::abs((*tob3)->eta())) > 10 ) continue;
		
		// Inv Mass calculation            
		unsigned int invmass2_12 = calcInvMass( *tob1, *tob2 );
		unsigned int invmass2_13 = calcInvMass( *tob1, *tob3 );
		unsigned int invmass2_23 = calcInvMass( *tob2, *tob3 );
		unsigned int invmass2 = invmass2_12 + invmass2_13 + invmass2_23;
                // Charge cut ( 1 = positive, -1 = negative, 0 = undefined (RPC) )
                int charge1 = (*tob1)->charge();
                int charge2 = (*tob2)->charge();
                int charge3 = (*tob3)->charge();
                int totalCharge = charge1 + charge2 + charge3;
                bool acceptCharge = true;
                if ( std::abs(totalCharge) == 3 ) { acceptCharge = false; }
		for(unsigned int i=0; i<numberOutputBits(); ++i) {
		  bool accept = false;
		  if( static_cast<parType_t>((*tob1)->Et()) <= p_MinET1[i]) continue; // ET cut
		  if( static_cast<parType_t>((*tob2)->Et()) <= p_MinET1[i]) continue; // ET cut
		  if( static_cast<parType_t>((*tob3)->Et()) <= p_MinET1[i]) continue; // ET cut

		  accept = invmass2 >= p_InvMassMin[i] && invmass2 <= p_InvMassMax[i] && acceptCharge;
		  const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
		  const bool fillReject = fillHistos() and not fillAccept;
		  const bool alreadyFilled = decision.bit(i);
		  if( accept ) {
		    std::vector<TCS::GenericTOB*> TOBvector;
		    TOBvector.push_back( *tob1 );
		    TOBvector.push_back( *tob2 );
		    TOBvector.push_back( *tob3 );
		    decision.setBit(i, true);
		    output[i]->push_back( TCS::CompositeTOB(TOBvector) );
		    TOBvector.clear();
		  }
		  if(fillAccept and not alreadyFilled) {
		    fillHist1D(m_histAccept[i],std::sqrt(invmass2));
                    fillHist2D(m_histAcceptEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                    fillHist2D(m_histAcceptEta2Eta3[i],(*tob2)->eta(),(*tob3)->eta());
                    fillHist2D(m_histAcceptEta3Eta1[i],(*tob3)->eta(),(*tob1)->eta());
		  } else if(fillReject) {
		    fillHist1D(m_histReject[i],std::sqrt(invmass2));
                    fillHist2D(m_histRejectEta1Eta2[i],(*tob1)->eta(),(*tob2)->eta());
                    fillHist2D(m_histRejectEta2Eta3[i],(*tob2)->eta(),(*tob3)->eta());
                    fillHist2D(m_histRejectEta3Eta1[i],(*tob3)->eta(),(*tob1)->eta());
		  }
		  TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " invmass2 = " << invmass2);
		  
		}
	      }
	    }
	  }
   } else {

      TCS_EXCEPTION("InvariantMassThreeTOBsIncl1Charge alg must have either 1 input list, but got " << input.size());

   }

   return TCS::StatusCode::SUCCESS;
}
