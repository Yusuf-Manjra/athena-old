/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMEVENT_TFCS1DFunctionRegression_h
#define ISF_FASTCALOSIMEVENT_TFCS1DFunctionRegression_h

#include "ISF_FastCaloSimEvent/TFCS1DFunction.h"
#include "TTree.h"
#include "TH1.h"
#include <vector>


class TFCS1DFunctionRegression:public TFCS1DFunction
{
  public:

    TFCS1DFunctionRegression() {};
    ~TFCS1DFunctionRegression() {};

    using TFCS1DFunction::rnd_to_fct;
    virtual double rnd_to_fct(double rnd) const;
    double  regression_value(double uniform) const;
    void    set_weights(std::vector<std::vector<double> > fWeightMatrix0to1, std::vector<std::vector<double> > fWeightMatrix1to2);
    double  sigmoid(double) const;

  private:

    std::vector<std::vector<double> > m_fWeightMatrix0to1;
    std::vector<std::vector<double> > m_fWeightMatrix1to2;


  ClassDef(TFCS1DFunctionRegression,1)  //TFCS1DFunctionRegression

};

#endif
