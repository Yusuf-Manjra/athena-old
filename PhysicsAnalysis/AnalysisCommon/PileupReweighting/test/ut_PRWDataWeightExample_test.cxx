/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/



//example program for how to use Data weight for unprescaling data

#ifdef XAOD_ANALYSIS
#ifndef XAOD_STANDALONE

#include "AsgTools/StandaloneToolHandle.h"
#include "AsgAnalysisInterfaces/IPileupReweightingTool.h"
#include "TrigDecisionInterface/ITrigDecisionTool.h"

#include "PileupReweighting/TPileupReweighting.h"

#include "TROOT.h"

#include "TTree.h"
#include "TH1D.h"

#include <assert.h>

using namespace asg::msgUserCode;

int main() {
  ANA_CHECK_SET_TYPE (int); //makes ANA_CHECK return ints if exiting function


  asg::StandaloneToolHandle<CP::IPileupReweightingTool> prwTool("CP::PileupReweightingTool/prw");
  
  std::vector<std::string> lcFiles = {"GoodRunsLists/data16_13TeV/20160803/physics_25ns_20.7.lumicalc.OflLumi-13TeV-005.root", //the unprescaled lc file
                                      "dev/PileupReweighting/ilumicalc_histograms_HLT_e12_lhvloose_nod0_L1EM10VH_297730-304494_OflLumi-13TeV-005.root:HLT_e12_lhvloose_nod0_L1EM10VH",
                                      "dev/PileupReweighting/ilumicalc_histograms_HLT_e24_lhvloose_nod0_L1EM20VH_297730-304494_OflLumi-13TeV-005.root:HLT_e24_lhvloose_nod0_L1EM20VH"}; //feed with lc files for each trigger
  
  ANA_CHECK(prwTool.setProperty( "DataScaleFactor", 1/1.09 ));
  ANA_CHECK(prwTool.setProperty( "LumiCalcFiles" , lcFiles ));
  //ANA_CHECK(prwTool.setProperty( "OutputLevel", 1 )); //VERBOSE OUTPUT LEVEL

  ANA_CHECK(prwTool.initialize());
  
  std::cout << prwTool->expert()->GetDataWeight( 297730 , "HLT_e12_lhvloose_nod0_L1EM10VH", 4) << std::endl;
  if(! ( fabs(prwTool->expert()->GetDataWeight( 297730 , "HLT_e12_lhvloose_nod0_L1EM10VH", 4)  - 8797.7) < 1e-3 ) ) {
    throw std::runtime_error("data weight not expected value 8797.7");
  }
  
  std::cout << prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH", 4) << std::endl; 
  if(! ( fabs(prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH", 4)  - 109.155) < 1e-3 ) ) {
    throw std::runtime_error("data weight not expected value 109.155");
  }
  
  std::cout << prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH||HLT_e12_lhvloose_nod0_L1EM10VH", 4) << std::endl; 
  if(! ( fabs(prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH||HLT_e12_lhvloose_nod0_L1EM10VH", 4)  - 107.829) < 1e-3 ) ) {
    throw std::runtime_error("data weight not expected value  107.829");
  }

  std::cout << prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH&&HLT_e12_lhvloose_nod0_L1EM10VH", 4) << std::endl; 
  if(! ( fabs(prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH&&HLT_e12_lhvloose_nod0_L1EM10VH", 4)  - 960312) < 1.0) ) {
    throw std::runtime_error("data weight not expected value 960312");
  }
  
  //compute expected prescale for mu of 24-25 in run 297730 for single trigger
  TFile f1("/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data16_13TeV/20160803/physics_25ns_20.7.lumicalc.OflLumi-13TeV-005.root");
  TTree* t1 = (TTree*)f1.Get("LumiMetaData");
  TH1* h1 = new TH1D("h1","h1",1,-0.5,0.5);
  t1->Draw("0>>h1","IntLumi*(RunNbr==297730&&AvergeInteractionPerXing>=(24*1.09)&&AvergeInteractionPerXing<(25*1.09))");
  TFile f2("/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/PileupReweighting/ilumicalc_histograms_HLT_e12_lhvloose_nod0_L1EM10VH_297730-304494_OflLumi-13TeV-005.root");
  TTree* t2 = (TTree*)f2.Get("LumiMetaData");
  TH1* h2 = new TH1D("h2","h2",1,-0.5,0.5);
  t2->Draw("0>>h2","IntLumi*(RunNbr==297730&&AvergeInteractionPerXing>=(24*1.09)&&AvergeInteractionPerXing<(25*1.09))");
  
  std::cout << "Expected run-dependent DataWeight  = " << h1->Integral()/h2->Integral() << std::endl;
  std::cout << prwTool->expert()->GetDataWeight( 297730 , "HLT_e12_lhvloose_nod0_L1EM10VH", 24.5,true) << std::endl; 
  if(! ( fabs(prwTool->expert()->GetDataWeight( 297730 , "HLT_e12_lhvloose_nod0_L1EM10VH", 24.5, true)  - (h1->Integral()/h2->Integral()) ) < 1e-3) ) {
     throw std::runtime_error("data weight not expected value");
  }
//   
  //in this version 4.0 onwards, trigger bits default to assuming the trigger passed
  
  prwTool->expert()->SetTriggerBit("HLT_e12_lhvloose_nod0_L1EM10VH" , 1);
  prwTool->expert()->SetTriggerBit("HLT_e24_lhvloose_nod0_L1EM20VH" , 0);
  std::cout << prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH||HLT_e12_lhvloose_nod0_L1EM10VH", 4) << std::endl; 
  if(! ( fabs(prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH||HLT_e12_lhvloose_nod0_L1EM10VH", 4)  - 8797.7) < 1e-3 ) ) {
    throw std::runtime_error("data weight not expected value 8797.7");
  }
  
  prwTool->expert()->SetTriggerBit("HLT_e24_lhvloose_nod0_L1EM20VH" , 1);
  std::cout << prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH||HLT_e12_lhvloose_nod0_L1EM10VH", 4) << std::endl; 
  if(! ( fabs(prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH||HLT_e12_lhvloose_nod0_L1EM10VH", 4)  - 107.829) < 1e-3 ) ) {
    throw std::runtime_error("data weight not expected value 107.829");
  }
  
  prwTool->expert()->SetTriggerBit("HLT_e12_lhvloose_nod0_L1EM10VH" , 0);
  std::cout << prwTool->expert()->GetDataWeight( 297730 ,  "HLT_e24_lhvloose_nod0_L1EM20VH||HLT_e12_lhvloose_nod0_L1EM10VH", 4) << std::endl; 
  if(! ( fabs(prwTool->expert()->GetDataWeight( 297730 , "HLT_e24_lhvloose_nod0_L1EM20VH||HLT_e12_lhvloose_nod0_L1EM10VH", 4)  - 109.155) < 1e-3 ) ) {
    throw std::runtime_error("data weight not expected value 109.155");
  }
  
  
  
  /*
  asg::StandaloneToolHandle<CP::IPileupReweightingTool> prwTool2("CP::PileupReweightingTool/prw2");
  prwTool2.setProperty( "LumiCalcFiles" , lcFiles );
  prwTool2.setProperty( "TrigDecisionTool" , "Trig::TrigDecisionTool/TrigDecisionTool" ); //only works for athena, not for eventloop
  prwTool2.initialize();
  evt.getEntry(0);
  ToolHandle<Trig::ITrigDecisionTool> tdt("Trig::TrigDecisionTool/TrigDecisionTool");
  tdt.retrieve();
  evt.getEntry(0);
  std::cout << prwTool2->expert()->GetDataWeight( 297730 ,  "HLT_e24_lhvloose_nod0_L1EM20VH||HLT_e12_lhvloose_nod0_L1EM10VH", 4) << std::endl; 
  */

   return 0;
}

#else //don't run this test in EventLoop release
int main() {
  return 0;
}
#endif
#else //StandaloneToolHandle not yet in AtlasDerivation

int main() {
  return 0;
}

#endif
