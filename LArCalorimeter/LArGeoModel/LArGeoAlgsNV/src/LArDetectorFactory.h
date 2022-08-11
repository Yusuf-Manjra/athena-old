/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file LArDetectorFactory.h
 *
 * @class LArGeo::LArDetectorFactory
 *
 * @brief LArDetectorFactory builds GeoModel description of LAr calorimeter by calling
 * relevant 'Construction' classes (Barrel, Endcap). It also builds readout geometry
 * description using LArReadoutGeometry objects.
 *
 * $Id: LArDetectorFactory.h,v 1.4 2009-02-10 16:43:17 tsulaia Exp $
 */

#ifndef LARGEOALGSNV_LARDETECTORFACTORY_H
#define LARGEOALGSNV_LARDETECTORFACTORY_H
#include "LArReadoutGeometry/LArDetectorManager.h"
#include "GeoModelKernel/GeoVDetectorFactory.h"

class StoreGateSvc;
class LArHVManager;

namespace LArGeo {
  class VDetectorParameters;

  class LArDetectorFactory : public GeoVDetectorFactory  {

  public:

    // Constructor:
    LArDetectorFactory(StoreGateSvc* detStore
		       , const LArHVManager* hvManager
		       , int testbeam
		       , bool fullGeo);

    // Destructor:
    virtual ~LArDetectorFactory();

    // Creation of geometry:
    virtual void create(GeoPhysVol* world);

    // Access to the results:
    virtual const LArDetectorManager* getDetectorManager() const;

    std::unique_ptr<LArGeo::VDetectorParameters> moveParameters();


    // Set parameters:

    void setBarrelSagging(bool flag)        {m_barrelSagging  = flag;}
    void setBarrelCellVisLimit(int maxCell) {m_barrelVisLimit = maxCell;}
    void setFCALVisLimit(int maxCell)       {m_fcalVisLimit   = maxCell;}
    void setBuildBarrel(bool flag)          {m_buildBarrel = flag;}
    void setBuildEndcap(bool flag)          {m_buildEndcap = flag;}
    void setEMECVariant(const std::string &inner, const std::string &outer)
    {
        m_EMECVariantInner = inner;
        m_EMECVariantOuter = outer;
    }
    void setActivateFT(bool flag)           { m_activateFT = flag; }
    void setEnableMBTS(bool flag)           { m_enableMBTS = flag; }

  private:

    // Illegal operations:
    const LArDetectorFactory & operator=(const LArDetectorFactory &right);
    LArDetectorFactory(const LArDetectorFactory &right);

    // The manager:
    LArDetectorManager*       m_detectorManager;
    StoreGateSvc*             m_detStore;
    const LArHVManager*       m_hvManager;
    bool                      m_barrelSagging;
    int                       m_barrelVisLimit;
    int                       m_fcalVisLimit;

    bool                      m_buildBarrel;
    bool                      m_buildEndcap;

    int                       m_testbeam;

    bool                      m_fullGeo;  // true->FULL, false->RECO
    std::string               m_EMECVariantInner;
    std::string               m_EMECVariantOuter;

    bool                      m_activateFT;
    bool                      m_enableMBTS;

    std::unique_ptr<VDetectorParameters> m_parameters;
  };

} // namespace LArGeo

#endif

