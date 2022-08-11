/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
///////////////////////////////////////////////////////////////////
// L1CPMTools.h, 
///////////////////////////////////////////////////////////////////


#ifndef ILVL1L1CPMTOOLS_H
#define ILVL1L1CPMTOOLS_H

#include "GaudiKernel/IAlgTool.h"
#include "AthContainers/DataVector.h"
#include "xAODTrigL1Calo/CPMTower.h"
#include "xAODTrigL1Calo/CPMTobRoIContainer.h"

#include "TrigConfData/L1Menu.h"

namespace LVL1 
{

class CPMTobAlgorithm;
class CPMTobRoI;

  /** Interface definition for L1CPMTools

      @author  Alan Watson <Alan.Watson@cern.ch>
  */  

  static const InterfaceID IID_IL1CPMTools("LVL1::IL1CPMTools", 1, 0);

  class IL1CPMTools : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    // enter declaration of your interface-defining member functions here
      virtual void mapTowers(const DataVector<xAOD::CPMTower>* cpmts, xAOD::CPMTowerMap_t* towers) const = 0;
      
      virtual void findRoIs(const TrigConf::L1Menu* l1menu, const xAOD::CPMTowerMap_t* towers, DataVector<CPMTobAlgorithm>* tobs, int slice = -1) const = 0;
      
      virtual void findRoIs(const TrigConf::L1Menu* l1menu, const DataVector<xAOD::CPMTower>* cpmts, DataVector<CPMTobAlgorithm>* tobs, int slice = -1) const = 0;
     
      virtual void findCPMTobRoIs(const TrigConf::L1Menu* l1menu, const xAOD::CPMTowerMap_t* towers, xAOD::CPMTobRoIContainer* rois, int slice = -1) const = 0;
     
      virtual void findCPMTobRoIs(const TrigConf::L1Menu* l1menu, const DataVector<xAOD::CPMTower>* cpmts, xAOD::CPMTobRoIContainer* rois, int slice = -1) const = 0;

      virtual void findCPMResults(const TrigConf::L1Menu* l1menu, const xAOD::CPMTowerMap_t* towers, int crate, int module,
                                  DataVector<CPMTobRoI>* rois, std::vector<unsigned int>& emCMXData,
                                  std::vector<unsigned int>& tauCMXData, int slice = -1) const = 0;
      
      virtual CPMTobAlgorithm findRoI(const TrigConf::L1Menu* l1menu, double RoIeta, double RoIphi, const xAOD::CPMTowerMap_t* towers, int slice = -1) const = 0;
      
      virtual CPMTobAlgorithm formSums(const TrigConf::L1Menu* l1menu, double RoIeta, double RoIphi, const xAOD::CPMTowerMap_t* towers, int slice = -1) const = 0;
      
      virtual CPMTobAlgorithm formSums(const TrigConf::L1Menu* l1menu, uint32_t roiWord, const xAOD::CPMTowerMap_t* towers, int slice = -1) const = 0;
      
    };

    inline const InterfaceID& LVL1::IL1CPMTools::interfaceID()
    { 
      return IID_IL1CPMTools;
    }
       
} 

#endif 
