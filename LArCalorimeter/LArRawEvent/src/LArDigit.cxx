/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <typeinfo>
#include <stdio.h>

#include "LArRawEvent/LArDigit.h"


// default constructor 
LArDigit::LArDigit() : 
  m_gain(CaloGain:: UNKNOWNGAIN) 
{}

LArDigit::operator std::string() const{
  /*
 LArCablingService * cablingService = LArCablingService::getInstance() ;
 
 if( cablingService == 0 ) {
   std::string error_message = "Unable to get cabling Service for printing HardwareID" ;
   return error_message ;
 }
  */
 
 
  std::string digitString = typeid( *this ).name();
 /*
	  if( m_ChannelID.id() == 0x80000000 ) {  
	     digitString += " in cell " ; 
             digitString += this->cellID().getString() ; // convert Identifier
	  } else {
	     digitString += " in channel " ;
	     digitString += cablingService->cnvToString( this->channelID() ) ; // convert LArSignalChannelID 
	  }
             digitString += "  ";
             char stNumberOfSamples[30];
             sprintf( stNumberOfSamples , "# of samples = %d " , this->nsamples() ) ;
	     digitString += stNumberOfSamples ;
             char stGain[20];
             sprintf( stGain , "gain = %d " , this->gain() ) ;
             digitString += stGain ;
	     digitString += "samples = " ;
	     const std::vector<short> & l_samples = this->samples() ;
	     int l_nsamples = this->nsamples() ;
	     for(int i = 0 ; i < l_nsamples ; i++ ) {  // loop over sample values
              char stSamples[20];
	      sprintf( stSamples   , "%d "    , l_samples[i]   ) ;
              digitString += stSamples ; 
	     }
 */
 
 return digitString ;


}

// set method
void LArDigit::setSamples(const std::vector<short>& samples)
{
  m_samples.clear();
  m_samples = samples;
}
