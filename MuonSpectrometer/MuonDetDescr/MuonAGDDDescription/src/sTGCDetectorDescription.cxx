/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/


#include "MuonAGDDDescription/sTGCDetectorDescription.h"
#include "MuonAGDDDescription/sTGC_Technology.h"
#include "AGDDModel/AGDDParameterStore.h"
#include "AGDDKernel/AGDDDetectorStore.h"

#include <sstream>


sTGCDetectorDescription::sTGCDetectorDescription(const std::string& s,
                                                 AGDDDetectorStore& ds):
    AGDDDetector(s,"sTGC"),
    m_yCutout(0),
    m_ds (ds)
{
}

void sTGCDetectorDescription::Register()
{
	m_ds.RegisterDetector(this);
}


void sTGCDetectorDescription::SetDetectorAddress(AGDDDetectorPositioner* p)
{
		//std::cout<<"This is AGDDsTGC::SetDetectorAddress "<<GetName()<<" "<<
		// m_sType;
		p->ID.detectorType="sTGC";
		p->theDetector=this;
		std::stringstream stringone;
		std::string side="A";
		if (p->ID.sideIndex<0) side="C";
		int ctype=0;
		int ml=1;
		if (subType().substr(1,1)=="S" && subType().substr(3,1)=="P") ml=2;
		else if (subType().substr(1,1)=="L" && subType().substr(3,1)=="C") ml=2;
		if (subType().substr(1,1)=="S") ctype=3;
		else if (subType().substr(1,1)=="L") ctype=1;
		stringone<<"sTG"<<ctype<<"-"<<subType().substr(2,1)<<"-"<<ml<<"-phi"<<p->ID.phiIndex+1<<side<<std::endl;
		//std::cout<<" stringone "<<stringone.str()<<std::endl;
		p->ID.detectorAddress=stringone.str();
}

MuonGM::sTGC_Technology* sTGCDetectorDescription::GetTechnology()
{
   MuonGM::sTGC_Technology* t =
     dynamic_cast<MuonGM::sTGC_Technology*>(m_ds.GetTechnology("sTGC_1")); //This needs to be the tech name not the chamber name

   return t;
}
