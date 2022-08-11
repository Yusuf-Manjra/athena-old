/**
 **     @file    TIDARoiParameters.cxx
 **
 **     @author  mark sutton
 **     @date    Wed  4 Mar 2015 16:22:25 CET 
 **
 **     Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
 **/

#include <cmath>

#include "TrigInDetAnalysis/TIDARoiParameters.h"

ClassImp( TIDARoiParameters )


/// Define M_PI ONLY if it is not already defined in <cmath>
#ifndef M_PI
#define M_PI  3.141592653589793238462643383279502884197
#endif

#ifndef M_2PI
static const float  M_2PI = 2*M_PI;
#endif

#ifndef M_PIF
static const float  M_PIF = float(M_PI);
#endif

double _phicheck(double phi) {
  if ( !(phi > -M_PIF && phi < M_PIF ) ) { // use ! of range rather than range to also catch nan etc
    if ( phi < -M_PIF  ) phi += M_2PI;
    else                 phi -= M_2PI;
  } 
  return phi;
}



TIDARoiParameters::TIDARoiParameters() : 
  m_eta(0), m_phi(0), m_zed(0),
  m_etaMinus(0), m_etaPlus(0),
  m_phiMinus(0), m_phiPlus(0),
  m_zedMinus(0), m_zedPlus(0)
{  }

TIDARoiParameters::TIDARoiParameters(double eta,  double etaMinus,   double etaPlus,
				     double phi,  double phiMinus,   double phiPlus,
				     double zed,  double zedMinus,   double zedPlus ) : 
  m_eta(eta), m_phi(phi), m_zed(zed),
  m_etaMinus(etaMinus), m_etaPlus(etaPlus),
  //  m_phiMinus(_phicheck(phiMinus)), m_phiPlus(_phicheck(phiPlus)),
  m_phiMinus(phiMinus), m_phiPlus(phiPlus),
  m_zedMinus(zedMinus), m_zedPlus(zedPlus)
{ 
  //  std::cout << "TIDAParameters     phi: " << phiMinus << " -> " << m_phiMinus << " : " << phiPlus << " -> " <<  m_phiPlus << std::endl;  
}


TIDARoiParameters::~TIDARoiParameters() { } 

