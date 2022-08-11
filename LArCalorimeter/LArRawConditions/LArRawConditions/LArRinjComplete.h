/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARRAWCONDITIONS_LARRINJCOMPLETE_H
#define LARRAWCONDITIONS_LARRINJCOMPLETE_H

#include "LArElecCalib/ILArRinj.h" 
#include "LArRawConditions/LArRinjP.h"
#include "LArRawConditions/LArConditionsContainer.h"

/** This class implements the ILArRinj interface
 *
 * @author M. Fanti
 * @version  0-0-1 , 22/12/05
 *
 *
 */

/**  use LArConditionsContainer. 
   Hong Ma , Aug 5, 2005
*/ 


class LArRinjComplete: public ILArRinj,
	public LArConditionsContainer<LArRinjP>{
  
 public: 
  
  typedef LArConditionsContainer<LArRinjP> CONTAINER ;
  LArRinjComplete();
  
  virtual ~LArRinjComplete();
  
  // retrieving Rinj using online ID
  
  virtual const float& Rinj(const HWIdentifier&  CellID) const ;
    
   // set method filling the data members individually (if one
  // wants to fill this class not using the DB)
  void set(const HWIdentifier& CellID, float vRinj);
};

#include "AthenaKernel/CondCont.h"
CLASS_DEF( LArRinjComplete,2370948,1)
CONDCONT_DEF( LArRinjComplete, 167204058, ILArRinj );

#endif 
