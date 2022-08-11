/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration 
*/
// Written by Dominik Derendarz (dominik.derendarz@cern.ch)
// Based on MultiBjetFilter by Bill Balunas

#ifndef GENERATORFILTERSMULTICJETFILTER_H
#define GENERATORFILTERSMULTICJETFILTER_H

#include "GeneratorModules/GenFilter.h"

//Random number generator required for accepting light jets
class TRandom3;

class MultiCjetFilter:public GenFilter {

  public:
    MultiCjetFilter(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MultiCjetFilter();
    virtual StatusCode filterInitialize();
    virtual StatusCode filterFinalize();
    virtual StatusCode filterEvent();

private:

    // Basic jet requirements
    double m_deltaRFromTruth;
    double m_jetPtMin;
    double m_jetEtaMax;
    int m_nJetsMin;
    int m_nJetsMax;

    // Variables for cutting sample into pt slices
    double m_leadJet_ptMin;
    double m_leadJet_ptMax;

    // Flavor filter variables
    double m_bottomPtMin;
    double m_bottomEtaMax;
    double m_charmPtMin;
    double m_charmEtaMax;
    int m_nCJetsMin;
    int m_nCJetsMax;

    std::string m_TruthJetContainerName;

    // Internal bookkeeping variables
    int    m_NPass;
    int    m_Nevt;
    double m_SumOfWeights_Pass;
    double m_SumOfWeights_Evt;

    bool isBwithWeakDK(const int pID) const;
    bool isCwithWeakDK(const int pID) const;


};

#endif


