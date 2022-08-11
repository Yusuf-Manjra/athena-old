/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// File: $Id: resolution_phi.cc,v 1.9 2008-07-23 20:51:23 adye Exp $
// file      : phi_resolution.cc
// author    : Sven Vahsen, March 2006
// synoposis : Root script to be used with TTree generated by InDetRecStatistics. 
//             Performs gaussian fits for phi-resolution in different eta region, 
//             and then combines the result of thse fits to make a summary histogram
//             of resolution versus eta. Number of bins, cuts etc are can be set by 
//             the user, the lines immediatly below.

#include <string>
#include <iostream>

#include "TROOT.h"
#include "TString.h"
#include "TTree.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCut.h"
#include "TLegend.h"

using std::cout;
using std::endl;
using std::string;

void resolution_phi (TTree* tree[3], const char* name[3],
                    const string & plotfile= "",
		    const int abseta = 1, const int pull = 0, const float etamax=2.5, // added by SCZ
                    const int ntrack_finders=3, const TCut data_cut="", double match=-1)
{
  string resstring = pull ? "pull" : "resolution";
      
  cout << "Generating phi " << resstring << " plots..." << endl;

  // -------------- parameters ---------------------
  const int n_eta_bins           = int(10*etamax/2.5)*(1+(!abseta));
  const int n_canvas_columns     = 4;
  const int n_phi_fit_bins       = 40;
  const float resolution_eta_max = etamax;
  const float resolution_eta_min = -1*(!abseta)*resolution_eta_max;

  float fithisto_min;
  float fithisto_max;
  if(pull) {
    fithisto_min       = -5.0;
    fithisto_max       = 5.0;
  } else {
    fithisto_min       = -0.001;
    fithisto_max       =  0.001;
  }

  // -------------- cuts ---------------------------
  TCut truth_cut ("track_truth_prob>"+TString::Format("%g",(match>=0?match:0.5)));
  //cppcheck-suppress incorrectStringBooleanError
  TCut cut_track_preselection = data_cut && "abs(1/track_qoverpt)>1000" && truth_cut;

  // -------------- style --------------------------
  TStyle plotStyle ("PlotStyle", "My Plot Style");
  plotStyle.cd();
  plotStyle.SetOptStat(0); 
  plotStyle.SetLineWidth(2);
  int color       []={1,2,4};
  int style       []={1,2,3};
  int marker_style[]={20,21,22};
  int linewidth                   = 3;

  // -------------- canvases -----------------------
  char fitstitle [500];
  char resolutiontitle [500];
  sprintf(fitstitle,"Fit results used to create histograms of %s vs eta",resstring.c_str());
  sprintf(resolutiontitle,"phi %s vs eta",resstring.c_str());

  TCanvas *c_fits       = new TCanvas("c_fits"      ,  fitstitle);
  TCanvas *c_resolution = new TCanvas("c_resolution",  resolutiontitle);
  c_resolution -> SetWindowPosition(500,1);
  c_fits->Divide(n_canvas_columns, abs(n_eta_bins/n_canvas_columns)+ ((n_eta_bins % n_canvas_columns) ? 1 : 0));

  // -------------- histograms and legend ---------------------
  TH1F **resolution_fits  = new TH1F*[n_eta_bins];
  TH1F *resolution       [ntrack_finders];
  string resname         [ntrack_finders];

  TLegend* leg_eff;
  if (abseta) {
    leg_eff = new TLegend(0.15,0.88,0.44,0.65);
  } else {
    leg_eff = new TLegend(0.35,0.88,0.64,0.65); 
  }

  // loop over the three trackfinders
  for (int i=0; i<ntrack_finders; i++) {
    resname         [i] = resstring + string("_phi_eta_") + name[i];
    resolution      [i] = new TH1F(resname [i].c_str(), "", n_eta_bins, resolution_eta_min, resolution_eta_max);

    float eta_binsize = (resolution_eta_max - resolution_eta_min) / n_eta_bins; 
  
    for (int eta_bin = 0; eta_bin < n_eta_bins; eta_bin++)
    {
      float eta_low        = eta_binsize * eta_bin + resolution_eta_min;
      float eta_high       = eta_binsize * (eta_bin + 1) + resolution_eta_min;
      char eta_bin_cut [500];

      if (abseta) {
	sprintf (eta_bin_cut, "abs(track_eta) >= %f && abs(track_eta) < %f && %s",eta_low, eta_high, cut_track_preselection.GetTitle());
      } else {
	sprintf (eta_bin_cut, "track_eta >= %f && track_eta < %f && %s",eta_low, eta_high, cut_track_preselection.GetTitle());
      }

      cout << "cut:" << eta_bin_cut << endl;
      char hist_name [500];
      sprintf( hist_name , "res_fits_%s_%d", name[i], eta_bin);
      resolution_fits [eta_bin] = new TH1F(hist_name, eta_bin_cut, n_phi_fit_bins, fithisto_min, fithisto_max);

      if (pull) {
	tree[i]->Project(hist_name, "(track_phi-track_truth_phi)/track_error_phi", eta_bin_cut);
      } else {
	tree[i]->Project(hist_name, "track_phi-track_truth_phi", eta_bin_cut);
      }

      c_fits->cd(1+eta_bin);
      resolution_fits [eta_bin]->Fit("gaus");
      resolution_fits [eta_bin]->Draw();
      c_fits->Update();

      // access fit parameters, and stuff sigma into summary histogram

      TF1 *func = (TF1*)resolution_fits [eta_bin]->GetFunction("gaus");
      
      if (func) {
        //	int npar = func->GetNpar();
        //	float chi2 = func->GetChisquare();
	
	//printf("Function has %d parameters. Chisquare = %f\n",
	//       npar,chi2);
	//for (Int_t i=0;i<npar;i++) {
	  // printf("   %s = %g +- %g\n",
	// func->GetParName(i),
	// func->GetParameter(i),
	// func->GetParError(i));
	//}
	       
	float sigma_phi           = func->GetParameter(2);
	float error_on_sigma_phi  = func->GetParError(2);
	if (pull) {
	  resolution[i]->SetBinContent (1+eta_bin, sigma_phi);
	  resolution[i]->SetBinError   (1+eta_bin, error_on_sigma_phi);
	} else {
          resolution[i]->SetBinContent (1+eta_bin, 1000*sigma_phi);
          resolution[i]->SetBinError   (1+eta_bin, 1000*error_on_sigma_phi);
	}          
      }
         
      // axis labels
     
      if (abseta) { 
	resolution[i]->GetXaxis()->SetTitle("|#eta|"); 
      } else {
        resolution[i]->GetXaxis()->SetTitle("#eta");
      }

      if(pull) {
        resolution[i]->GetYaxis()->SetTitle("Pull(#phi)");
      } else {
        resolution[i]->GetYaxis()->SetTitle("#sigma(#phi) (mrad)");
      }
 
      c_resolution->cd(); 
      resolution[i]->SetMarkerStyle(marker_style[i%3]);
      resolution[i]->SetMarkerColor(color[i%3]); 
      resolution[i]->SetLineStyle(style[i%3]); 
      resolution[i]->SetLineColor(color[i%3]);
      resolution[i]->SetLineWidth(linewidth); 
      resolution[i]->Draw(i ? "p e same" : "p e");
      c_resolution->Update();
    }
    leg_eff->AddEntry(resolution[i], name[i], "e p l"); 
    if (ntrack_finders>=2) leg_eff->Draw();
  }

  // --------------------------- print out .eps files --------------------------

  if (plotfile != "") c_resolution->Print(plotfile.c_str());

  gROOT->SetStyle("Default");
  delete leg_eff;
  delete c_fits;
  delete c_resolution;
  delete [] resolution_fits;
}




