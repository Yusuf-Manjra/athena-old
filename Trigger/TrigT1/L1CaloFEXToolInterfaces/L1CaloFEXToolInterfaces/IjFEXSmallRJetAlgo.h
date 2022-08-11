/*
 Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for jFEXSmallRJetAlgo - Algorithm for small R jet Algorithm in jFEX
//                              -------------------
//     begin                : 03 11 2020
//     email                : varsiha.sothilingam@cern.ch
//***************************************************************************

#ifndef IjFEXSmallRJetAlgo_H
#define IjFEXSmallRJetAlgo_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/jFEXSmallRJetTOB.h"
#include "L1CaloFEXSim/jTowerContainer.h"

namespace LVL1{

    static const InterfaceID IID_IjFEXSmallRJetAlgo("LVL1::IjFEXSmallRJetAlgo",1, 0);
     
    class IjFEXSmallRJetAlgo : virtual public IAlgTool{
    public:
      static const InterfaceID& interfaceID ( ) ;
      virtual StatusCode safetyTest() = 0;
      virtual void setup(int inputTable[7][7]) = 0;
      virtual int realValue(int ID, int eta) =0;
      virtual bool isSeedLocalMaxima() = 0;
      virtual void buildSeeds() = 0;
      virtual unsigned int getRealPhi(unsigned int TTID ) =0;
      virtual int getRealEta(unsigned int TTID ) =0;
      virtual unsigned int getTTowerET(unsigned int TTID ) = 0;
      virtual unsigned int getSmallClusterET() =0;
      virtual unsigned int getSmallETRing() =0;
      virtual bool checkDisplacedLM() = 0;
      virtual std::unique_ptr<jFEXSmallRJetTOB> getSmallRJetTOBs() = 0; 
      virtual unsigned int getTTIDcentre() =0;
      virtual void setFPGAEnergy(std::unordered_map<int,std::vector<int> > et_map)   =0;

   private:

    }; 
    inline const InterfaceID& LVL1::IjFEXSmallRJetAlgo::interfaceID()
    {
      return IID_IjFEXSmallRJetAlgo;
    }

}
#endif

