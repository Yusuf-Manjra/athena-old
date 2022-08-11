/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAGDDDescription/MMDetectorHelper.h"
#include "MuonAGDDDescription/MMDetectorDescription.h"
#include "AGDDKernel/AGDDDetectorStore.h"
#include "AGDDKernel/AGDDPositionerStore.h"
#include "AGDDKernel/AGDDDetectorPositioner.h"
#include "AGDDControl/AGDDController.h"

#include <vector>

MMDetectorHelper::MMDetectorHelper()
{
        if (m_svc.retrieve().isFailure()) {
          std::abort();
        }
        IAGDDtoGeoSvc::LockedController c = m_svc->getController();
	detectorList vl= c->GetDetectorStore().GetDetectorList();
	
	for ( const auto& vl_iter: vl)
	{
		MMDetectorDescription* st=dynamic_cast<MMDetectorDescription*>(vl_iter.second);
		if (st) {
			m_MicromegasList[vl_iter.first]=st;
			m_MicromegasListSubType[vl_iter.second->subType()]=st;
		}
	}
	
}

MMDetectorDescription* MMDetectorHelper::Get_MMDetector(char type,int ieta,int iphi,int layer,char side)
{
	MMDetectorDescription* mm=nullptr;
	
        IAGDDtoGeoSvc::LockedController c = m_svc->getController();
        AGDDPositionerStore& ps = c->GetPositionerStore();
	for (unsigned int i=0;i<ps.size();i++)
	{
		AGDDDetectorPositioner* dp=dynamic_cast<AGDDDetectorPositioner*>(ps[i]);
		if (dp)
		{
			if (dp->ID.detectorType != "Micromegas") continue;
			std::string dad=dp->ID.detectorAddress;

			char dtype=dad[3];
			char dctype='L';
			if (dtype=='3') dctype='S';
			if (dctype!=type) continue;
			int deta=atoi(dad.substr(5,1).c_str());
			if (deta!=ieta) continue;
			int dphi=atoi(dad.substr(12,1).c_str());
			if (dphi!=iphi) continue;
			int dlayer=atoi(dad.substr(7,1).c_str());
			if (dlayer!=layer) continue;
			char dside=dad[13];
			if (dside!=side) continue;
			
			mm=dynamic_cast<MMDetectorDescription*>(dp->theDetector);
		}
		if(mm) break;
	}
	if (!mm) std::cout<<" could not find a positioned Micromegas!!!! "<<std::endl;
	return mm;
}

AGDDPositionedDetector MMDetectorHelper::Get_MMPositionedDetector(char type,int ieta,int iphi,int layer,char side)
{
	MMDetectorDescription* mm=nullptr;
	AGDDDetectorPositioner* dp=nullptr;
	
        IAGDDtoGeoSvc::LockedController c = m_svc->getController();
        AGDDPositionerStore& ps = c->GetPositionerStore();
	for (unsigned int i=0;i<ps.size();i++)
	{
                dp=dynamic_cast<AGDDDetectorPositioner*>(ps[i]);
		if (dp)
		{
			if (dp->ID.detectorType != "Micromegas") continue;
			std::string dad=dp->ID.detectorAddress;

			char dtype=dad[3];
			char dctype='L';
			if (dtype=='3') dctype='S';
			if (dctype!=type) continue;
			int deta=atoi(dad.substr(5,1).c_str());
			if (deta!=ieta) continue;
			int dphi=atoi(dad.substr(12,1).c_str());
			if (dphi!=iphi) continue;
			int dlayer=atoi(dad.substr(7,1).c_str());
			if (dlayer!=layer) continue;
			char dside=dad[13];
			if (dside!=side) continue;
			
			mm=dynamic_cast<MMDetectorDescription*>(dp->theDetector);
		}
		if(mm) break;
	}
	if (!mm) std::cout<<" could not find a positioned Micromegas!!!! "<<std::endl;
	AGDDPositionedDetector p_mm(mm,dp);
	return p_mm;
}

MMDetectorDescription* MMDetectorHelper::Get_MMDetectorType(const std::string& type)
{
	if (m_MicromegasList.find(type) != m_MicromegasList.end()) return m_MicromegasList[type];
	return nullptr;
}

MMDetectorDescription* MMDetectorHelper::Get_MMDetectorSubType(const std::string& type)
{
	if (m_MicromegasListSubType.find(type) != m_MicromegasListSubType.end()) return m_MicromegasListSubType[type];
	return nullptr;
}


IAGDDtoGeoSvc::LockedController MMDetectorHelper::Get_Controller()
{
  return m_svc->getController();
}
