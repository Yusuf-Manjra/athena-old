/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCoudeElectrodes.h"

LArCoudeElectrodes* LArCoudeElectrodes::s_instance=nullptr;

PhysicalVolumeAccessor* LArCoudeElectrodes::s_theCoudes=nullptr;

std::once_flag LArCoudeElectrodes::s_flag;

LArCoudeElectrodes*  LArCoudeElectrodes::GetInstance(const std::string& strDetector)
{
  std::call_once( s_flag, [&](){ s_instance = new LArCoudeElectrodes(strDetector); } );
  return s_instance;
}


LArCoudeElectrodes::LArCoudeElectrodes(const std::string& strDetector)
{
  if (s_theCoudes==nullptr)
    {
      if (strDetector.empty()) {
        s_theCoudes=
          new PhysicalVolumeAccessor("LAr::EMB::STAC",
                                     "LAr::EMB::Electrode::CornerDownFold");
        s_theCoudes->SetPhysicalVolumeList("LAr::EMB::Electrode::CornerUpFold");
      }
      else {
        s_theCoudes=
          new PhysicalVolumeAccessor(strDetector+"::LAr::EMB::STAC",
                                     strDetector+"::LAr::EMB::Electrode::CornerDownFold");
        s_theCoudes->SetPhysicalVolumeList(strDetector+"::LAr::EMB::Electrode::CornerUpFold");
      }
    }
  m_filled=false;
  for (int stackid=0; stackid<15; stackid++) {
    for (int cellid=0; cellid<1024; cellid++) {
      m_xcent[cellid][stackid] = XCentCoude(stackid,cellid);
      m_ycent[cellid][stackid] = YCentCoude(stackid,cellid);
      m_phirot[cellid][stackid] = PhiRot(stackid,cellid);

    }
  }
  m_filled=true;
}
double LArCoudeElectrodes::XCentCoude(int stackid, int cellid) const
{
  if (m_filled) {
    return m_xcent[cellid][stackid];
  }
  else {
    const int id=cellid+stackid*10000;
    const G4VPhysicalVolume *pv=s_theCoudes->GetPhysicalVolume(id);
    if (!pv) return 0.;
    const G4ThreeVector& tv=pv->GetTranslation();
    return tv.x();
  }
}
double LArCoudeElectrodes::YCentCoude(int stackid, int cellid) const
{
  if (m_filled) {
    return m_ycent[cellid][stackid];
  }
  else {
    const int id=cellid+stackid*10000;
    const G4VPhysicalVolume *pv=s_theCoudes->GetPhysicalVolume(id);
    if (!pv) return 0.;
    const G4ThreeVector& tv=pv->GetTranslation();
    return tv.y();
  }
}
double LArCoudeElectrodes::PhiRot(int stackid, int cellid) const
{
  if (m_filled) {
    return m_phirot[cellid][stackid];
  }
  else {
    const int id=cellid+stackid*10000;
    const G4VPhysicalVolume *pv=s_theCoudes->GetPhysicalVolume(id);
    if (!pv) return 0.;
    const G4RotationMatrix *rm=pv->GetRotation();
    double alpha;
    if (!rm) alpha=0.;
    else alpha = rm->phiX();
    // for down fold
    if (pv->GetName().find("DownFold") != std::string::npos) alpha=alpha-3.14159;
    // old way was assuming we start with a down fold if (stackid%2==0) alpha=alpha-3.14159;
    return alpha;
  }
}
