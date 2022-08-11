/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAGDDDescription/MMDetectorDescription.h"
#include "AGDDModel/AGDDParameterStore.h"
#include "AGDDKernel/AGDDDetectorStore.h"

#include <sstream>



MMDetectorDescription::MMDetectorDescription(const std::string& s,
                                             AGDDDetectorStore& ds):
	AGDDDetector(s,"Micromegas"),
        m_ds (ds)
{
}

void MMDetectorDescription::Register()
{
	m_ds.RegisterDetector(this);
}


void MMDetectorDescription::SetDetectorAddress(AGDDDetectorPositioner* p)
{
		//std::cout<<"This is AGDDMicromegas::SetDetectorAddress "<<GetName()<<" "<<
		//m_sType;
		p->ID.detectorType="Micromegas";
		p->theDetector=this;
		std::stringstream stringone;
		std::string side="A";
		if (p->ID.sideIndex<0) side="C";
		int ctype=0;
		int ml=atoi(subType().substr(3,1).c_str());
		if (subType().substr(2,1)=="L") ctype=1;
		else if (subType().substr(2,1)=="S") ctype=3;
		int etaIndex=atoi(subType().substr(1,1).c_str());
		stringone<<"sMD"<<ctype<<"-"<<etaIndex<<"-"<<ml<<"-phi"<<p->ID.phiIndex+1<<side<<std::endl;
		//std::cout<<" stringone "<<stringone.str()<<std::endl;
		p->ID.detectorAddress=stringone.str();
}

MuonGM::MM_Technology* MMDetectorDescription::GetTechnology()
{
   MuonGM::MM_Technology* t =
     dynamic_cast<MuonGM::MM_Technology*>(m_ds.GetTechnology(GetName()));
   return t;
}
