/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************

   NAME: Calibrator.cxx    
PACKAGE: TRTCalibTools 

AUTHORS: Johan Lundquist  
CREATED: 27-03-2009 

PURPOSE: Class for calibrating a TRT sub-level
          
********************************************************************/

#include "Calibrator.h"
#include "CxxUtils/checker_macros.h"
#include <TH1F.h>
#include <TH2F.h>
#include <TH1D.h>
#include <TDirectory.h>
#include <TF1.h>
#include <TNtuple.h>
#include <TGraphErrors.h>
#include <TStyle.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>

caldata::caldata(){
  res = -999; 
  resMean = -999;
  reserr = -999;
  tres = -999;
  tresMean = -999;
  t0 = -999;
  t0err = -999;
  reft0  = -999; 
  t0off  = -999; 
  rtt0  = -999; 
  nhits = -999;
  x  = -999;
  y  = -999;
  z  = -999;
  oldt02  = -999;
  sumt0  = -999;
  sumx  = -999;
  sumy  = -999;
  sumz  = -999;
  calflag  = -999;
  rtflag  = -999;
  t0flag  = -999;
  det = -999;
  lay = -999;
  mod = -999;
  brd = -999;
  chp = -999;
  stl = -999;
  stw = -999;
  sid = -999;
  ntres = -999;
  nrt = -999;
  nres = -999;
  t0fittype = -999;
  rtgraph = nullptr;
}

caldata::~caldata(){
}

caldata::caldata(bool makehist, int nbinst, int nbinsr){
  res = -999; 
  resMean = -999;
  reserr = -999;
  tres = -999;
  tresMean = -999;
  t0 = -999;
  t0err = -999;
  reft0  = -999; 
  t0off  = -999; 
  rtt0  = -999; 
  nhits = -999;
  x  = -999;
  y  = -999;
  z  = -999;
  oldt02  = -999;
  sumt0  = -999;
  sumx  = -999;
  sumy  = -999;
  sumz  = -999;
  calflag  = -999;
  rtflag  = -999;
  t0flag  = -999;
  det = -999;
  lay = -999;
  mod = -999;
  brd = -999;
  chp = -999;
  stl = -999;
  stw = -999;
  sid = -999;
  ntres = -999;
  nrt = -999;
  nres = -999;
  t0fittype= -999;
  rtgraph = nullptr;

  if (makehist) {
    m_treshist.resize (100);
    reshist.resize (100);
    rthist.resize (nbinsr*nbinst+200);
  }
}

RtGraph::RtGraph(TH2F* rtHist, int binvar, const char* binlabel, bool pflag, TDirectory* dir){

  int fitresult;
  npoints = binvar==0 ? rtHist->GetNbinsX() : rtHist->GetNbinsY() ;


  m_mean = -20.0; 
  m_t = -20.0;
  m_d = -20.0;
  m_et = -20.0;
  m_ed = -20.0;
  m_ff =  nullptr ;

  
  hslizes = new TH1D*[npoints];
  m_btype = new bintype[npoints];
  m_tv = new float[npoints];
  m_dv = new float[npoints];
  m_etv = new float[npoints];
  m_edv = new float[npoints];
  m_rightsig = new float[npoints];
  m_leftsig = new float[npoints];
  m_leftval = new float[npoints];
  m_rightval = new float[npoints];
  m_maxbin = new int[npoints];
  m_maxval = new float[npoints];
  
  m_ipoint=0;
  mintime=999;
  m_mindistance=0;
  
  if (pflag){
    TDirectory* binhistdir = dir->mkdir("binhist");
    binhistdir->cd();
  }

  // check and classify the bin-histograms
  for (int i=0;i<npoints;i++) {
    
    m_chname = std::string(Form("%s slize_%i",binlabel,i));
    m_chtit = binvar==0 ? std::string(Form("bin %i : %4.2f < %s < %4.2f  [ns]",i,rtHist->GetXaxis()->GetBinLowEdge(i+1),binlabel,rtHist->GetXaxis()->GetBinUpEdge(i+1))) :
                        std::string(Form("bin %i : %4.2f < %s < %4.2f  [mm]",i,rtHist->GetYaxis()->GetBinLowEdge(i+1),binlabel,rtHist->GetYaxis()->GetBinUpEdge(i+1))) ;
    hslizes[i] = binvar==0 ? rtHist->ProjectionY(m_chname.data(),i+1,i+1) :
                             rtHist->ProjectionX(m_chname.data(),i+1,i+1);
    hslizes[i]->SetTitle(m_chtit.data());
    if (!pflag){
      hslizes[i]->SetDirectory(nullptr);
    }
    
    m_maxbin[i]=1; m_maxval[i]=0;
    for (int j=1;j<=hslizes[i]->GetNbinsX();j++) {
      float val=hslizes[i]->GetBinContent(j);
      if (val>m_maxval[i]){
        m_maxval[i]=val;
        m_maxbin[i]=j;
        m_leftval[i] = hslizes[i]->GetBinContent(j-1);
        m_rightval[i] = hslizes[i]->GetBinContent(j+1);
      }
    }
    
    m_rightsig[i] = m_rightval[i]==0 ? 100 : std::sqrt(m_maxval[i])/m_maxval[i] + std::sqrt(m_rightval[i])/m_rightval[i];
    m_leftsig[i]  = m_leftval[i]==0 ? 100 : std::sqrt(m_maxval[i])/m_maxval[i] + std::sqrt(m_leftval[i])/m_leftval[i];
    
    m_btype[i]=EMPTY;
    if(true){
      m_btype[i]=GOOD;
      m_btype[i-1]=GOOD;
    } 
    else{
      if (hslizes[i]->GetEntries()>0){
        if (i<(float)hslizes[i]->GetNbinsX()/2)  m_btype[i]=LOW;
        else m_btype[i]=HIGH;
      }
    }
    printf("%s ... %8f %8i %4i %8f %8f %4i\n", m_chtit.data(), (float)hslizes[i]->GetEntries()/(float)rtHist->GetEntries(), (int)hslizes[i]->GetEntries(), m_maxbin[i], m_leftsig[i], m_rightsig[i], m_btype[i]);
  }
  
  float frmin=0,frmax=0;
  for (int i=0;i<npoints;i++) {
    
    // prepare initial fit parameters   
    m_mean=hslizes[i]->GetBinCenter(m_maxbin[i]);
    if (m_btype[i]==LOW) {frmin = hslizes[i]->GetBinCenter(1);        frmax = hslizes[i]->GetBinCenter(m_maxbin[i]+4);}
    if (m_btype[i]==GOOD) {frmin = hslizes[i]->GetBinCenter(m_maxbin[i]-3); frmax = hslizes[i]->GetBinCenter(m_maxbin[i]+3);}
    if (m_btype[i]==HIGH) {frmin = hslizes[i]->GetBinCenter(m_maxbin[i]-3); frmax = hslizes[i]->GetBinCenter(m_maxbin[i]+4);}
    
    m_ff = new TF1("dtfitfunc","gaus");
    
    m_ff->SetRange(frmin,frmax);

    m_t = binvar==0 ? rtHist->GetXaxis()->GetBinCenter(i+1) : rtHist->GetYaxis()->GetBinCenter(i+1);
    m_et = binvar==0 ? rtHist->GetXaxis()->GetBinWidth(1)/std::sqrt(12.0) : rtHist->GetYaxis()->GetBinWidth(1)/std::sqrt(12.0);
    m_d = 0;
    m_ed = 0;
    
    if (m_btype[i]==LOW){
      m_d =  0;
      m_ed = 0;
    }
    if (m_btype[i]==HIGH){
      m_d =  2;
      m_ed = 0;
    }
    if (m_btype[i]==EMPTY){
      m_d =  0;
      m_ed = 0;
    }
    if (m_btype[i]==GOOD){
      if (m_btype[i-1]==GOOD){
        fitresult=hslizes[i]->Fit("dtfitfunc","QR");
        m_ff->SetRange(m_mean-1.0*m_ff->GetParameter(2), m_mean+1.0*m_ff->GetParameter(2));
        fitresult=hslizes[i]->Fit("dtfitfunc","QR");
        m_d =  m_ff->GetParameter(1);
        m_ed = m_ff->GetParError(1);
        std::cout << fitresult << " " << m_ed << std::endl;
      }
      else{
        m_d =  0;
        m_ed = 0;
      }
    }


    if (m_btype[i]==GOOD && m_ed!=0){
      m_tv[m_ipoint]=m_t;
      m_dv[m_ipoint]=m_d;
      m_etv[m_ipoint]=m_et;
      m_edv[m_ipoint]=m_ed;
      m_ipoint++;
      if (binvar==0) {
        rval.push_back(m_d);
        tval.push_back(m_t);
      }
      else {
        rval.push_back(m_t);
        tval.push_back(m_d);
      }
    }

    
    if(m_d>=m_mindistance && m_t<mintime) mintime = m_t; 
  }  

  rtgr = binvar==0 ? new TGraphErrors(m_ipoint,m_tv,m_dv,m_etv,m_edv) :  new TGraphErrors(m_ipoint,m_dv,m_tv,m_edv,m_etv);
  rtgr->SetTitle((std::string("binning in ") + binlabel).data());
  trgr = binvar==0 ? new TGraphErrors(m_ipoint,m_dv,m_tv,m_edv,m_etv) :  new TGraphErrors(m_ipoint,m_tv,m_dv,m_etv,m_edv);
  trgr->SetTitle((std::string("binning in ") + binlabel).data());

  rtgr->SetMarkerStyle(1) ;
  rtgr->SetMarkerColor(2) ;
  rtgr->SetLineColor(2) ;
  rtgr->SetName("rtgraph") ;
  trgr->SetMarkerStyle(1) ;
  trgr->SetMarkerColor(2) ;
  trgr->SetLineColor(2) ;
  trgr->SetName("trgraph") ;

  dir->cd();




}



RtGraph::~RtGraph(){  

 delete [] hslizes   ;
 delete [] m_btype     ;
 delete [] m_tv        ;
 delete [] m_dv        ;
 delete [] m_etv       ;
 delete [] m_edv       ;
 delete [] m_rightsig  ;
 delete [] m_leftsig   ;
 delete [] m_leftval   ;
 delete [] m_rightval  ;
 delete [] m_maxbin    ;
 delete [] m_maxval    ;
}


double pol3deg(double *x, double *par) {
  double r = x[0];
  double t = par[0]+r*(par[1]+(r*(par[2]+r*par[3])));
  return t;
}

double pol3deg2(double *x, double *par) {
  double t = x[0];
  double r = par[1]*(t-par[0]) + par[2]*(t-par[0])*(t-par[0]) + par[3]*(t-par[0])*(t-par[0])*(t-par[0]);
  return r;
}


double trrel_dines(double *x, double *par){
  double rmin0 = par[0];
  double rho = par[1];
  double v = par[2];
  double t_const = par[3];
  double r = x[0];
  double t = t_const + 2*rho/v*std::asin(std::sqrt(rmin0*rmin0*(1-0.25*r*r)+r*r)/(2*rho));
  return t;
}

double rtrel_dines(double *x, double*par){
  double rmin0 = par[0];
  double rho = par[1];
  double v = par[2];
  double t_const = par[3];
  double t = x[0];
  double r_squared = (4*rho*rho*std::sin(v*(t-t_const)/(2*rho))-rmin0*rmin0)/(1-0.25*rmin0*rmin0);
  double r=r_squared>0 ? std::sqrt(r_squared) : 0.0;
  return r;
}



Calibrator::Calibrator()
  : dort(false),
    dot0(false),
    dores(false),
    nort(false),
    not0(false),
    usebref(false),
    bequiet(false),
    printlog(false),
    printt0(false),
    printrt(false),
    usep0(false),
    floatp3(false),
    useshortstraws(true),

    m_name ("None"),
    m_rtbinning ("None"),
    m_minrtstat (-10),
    m_mint0stat (-10),
    m_t0shift (-100.),

    m_nbinsr(100),
    m_nbinst(55),
    m_nbinstres(100),
    m_nbinsres(100),
    m_minr(0),
    m_maxr(2),
    m_mint(-5),
    m_maxt(50),
    m_mintres(-10),
    m_maxtres(10),
    m_minres(-0.6),
    m_maxres(0.6),

    m_ntreshits(0),
    m_nreshits(0),
    m_nrthits(0),
    m_nhits(0),

    m_isdines ( false)
{
  selection.insert(-3);
  level =-10;
}

Calibrator::Calibrator(int lev, const std::string& nme, int mint0, int minrt, const std::string& rtr, const std::string& rtb, float t0sft)
  :  dort(false),
     dot0(false),
     dores(false),
     nort(false),
     not0(false),
     usebref(false),
     bequiet(false),
     printlog(false),
     printt0(false),
     printrt(false),
     usep0(false),
     floatp3(false),
     useshortstraws(true),

     m_name(nme), 
     m_rtbinning(rtb),
     m_minrtstat(minrt),
     m_mint0stat(mint0),
     m_t0shift(t0sft),

     m_nbinsr(100),
     m_nbinst(55),
     m_nbinstres(100),
     m_nbinsres(100),
     m_minr(0),
     m_maxr(2),
     m_mint(-5),
     m_maxt(50),
     m_mintres(-10),
     m_maxtres(10),
     m_minres(-0.6),
     m_maxres(0.6),

     m_ntreshits(0),
     m_nreshits(0),
     m_nrthits(0),
     m_nhits(0),

     m_isdines ( rtr.find("dines")!=std::string::npos)
{
  level=lev;
  selection.insert(-3);
}

Calibrator::~Calibrator(){
}

int Calibrator::Simple1dHist(float min, float max, int nbins, float value){
  if ( (value<min) || (value>max) ) return -1;
  int binno=(int)(nbins*((value-min)/(max-min)));
  return binno;
}

int Calibrator::Simple2dHist(float minx, float maxx, int nbinsx, float miny, float maxy, int nbinsy, float valuex, float valuey){
  if ( (valuex<minx) || (valuex>maxx) ||  (valuey<miny) || (valuey>maxy) ) return -1;
  int binnox=(int)(nbinsx*((valuex-minx)/(maxx-minx)));
  int binnoy=(int)(nbinsy*((valuey-miny)/(maxy-miny)));
  return binnoy*nbinsx+binnox;
}

float Calibrator::AccumulativeMean(float n, float oldmean, float newvalue){
    return oldmean*((n-1)/n)+newvalue/n;
}

bool Calibrator::HasKey(const std::string &key) const {

  return data.find(key) != data.end();
}

bool Calibrator::CheckSelection(int level){
  if (selection.find(level)!=selection.end() || selection.find(-3)!=selection.end()) return true;
  else return false;
}

bool Calibrator::Skip(){
  if (selection.find(-4)!=selection.end()) return true;
  else return false;
}

std::string Calibrator::PrintInfo(){
  std::string yn[2]={"NO","YES"};
  std::string info = std::string(Form("CONFIGURATION %-16s: dort=%-3s, dot0=%-3s, dores=%-3s, selection=",m_name.data(),yn[dort].data(),yn[dot0].data(),yn[dores].data()));
  for (int isel : selection) {
    if (isel==-3) info += std::string("all"); 
    else if (isel==-4) info += std::string("none"); 
    else info += std::string(Form("%2i,",isel)); 
  } 
  return info;
}

std::string Calibrator::PrintStat(){
  std::string info = std::string(Form("STATISTICS %16s: nunits=%8i, nhits=%9i, hits/unit=%11.1f", m_name.data(), (int)data.size(), m_nhits, (float)m_nhits/(float)data.size() ));
  return info;
}

std::string Calibrator::GetOptString() const {
  std::string optstr="";
  if (dort) optstr += 'R';
  if (dot0) optstr += 'T';
  if (printlog) optstr +='P';
  if (printt0) optstr += 'F';
  if (printrt) optstr += 'G';
  if (bequiet) optstr += 'Q';
  if (usebref) optstr += 'B';
  if (useshortstraws) optstr += 'S';
  if (usep0 && dort) optstr += '0';
  if (floatp3 && dort) optstr += '3';
  if (!(dort || dot0 || usebref || bequiet || printlog || printt0 || printrt)) optstr += 'N';
  
  return optstr;
}

int Calibrator::GetNDirs(){
  return data.size();
}

std::string Calibrator::GetSelString(){
  std::string selstr="";
  if (selection.find(-3)!=selection.end()) selstr="*";
  else if (selection.find(-4)!=selection.end()) selstr="-";
  else for (int isel : selection)  selstr += std::string(Form("%i",isel));
  return selstr;
}

std::string Calibrator::GetBinnedRt(const std::string& key){

  int nbins=data[key].rtgraph->tval.size();
  double maxt=data[key].rtgraph->tval[nbins-1];
  double mint=data[key].rtgraph->tval[0];

  std::string brt = "";
  brt += Form("%f %f %i ",mint,maxt,nbins);

  for (int ip=0; ip<nbins;ip++){
    brt += Form("%f ",data[key].rtgraph->rval[ip]);
  }
  return brt;
}

int Calibrator::UpdateOldConstants(){

  std::string line;
  std::ifstream oldconstfile("calib_constants_in.txt");
  std::string key;
  float t0;
  
  if (oldconstfile.is_open())
    {
      while (!oldconstfile.eof() )
      {
        oldconstfile >> key >> t0;
        if (data.find(key) != data.end()){
          std::cout << "UPDATED OLD T0: " << key << " " << data[key].oldt02 << " -> " << t0 << std::endl; 
          data[key].oldt02=t0;
        }
      }
      oldconstfile.close();
      return 0;
    }  
  else {
    std::cout << "NO OLD T0S FILE FOUND. USING AVERAGE VALUES!" << std::endl;
    return -1;
  } 
}

float Calibrator::FitRt ATLAS_NOT_THREAD_SAFE (const std::string& key, const std::string& opt, TH2F* rtHist, TDirectory* dir){ // Global gStyle is used.

  float rtpars[4];

  //create r-m_t and m_t-r graphs
  RtGraph* rtg = m_rtbinning.find('t')==std::string::npos ? new RtGraph(rtHist,1,std::string("abs(rtrack)").data(),!bequiet,dir) : new RtGraph(rtHist,0,std::string("t-t0").data(),!bequiet,dir);

  TF1 dtfitfunc("dtfitfunc","gaus(0)");

  gStyle->SetOptFit(1111);
  gStyle->SetPalette(1);

  // select type of R-m_t relation
  TF1 *trfunc,*rtfunc,*oldrtfunc;
  if (m_isdines){
    trfunc = new TF1("trfunc",trrel_dines,0,3,4);
    rtfunc = new TF1("rtfunc",rtrel_dines,rtg->mintime,200,4);
    oldrtfunc = new TF1("oldrtfunc",rtrel_dines,rtg->mintime,200,4);
    trfunc->SetParameters(0.3, 1.0, 0.05, -3.);
    double rmin0Limits[2]={0.0,2.0};
    double rhoLimits[2]={0.0,10.};
    double vLimits[2]={0.0,1.0};
    trfunc->SetParLimits(0,rmin0Limits[0],rmin0Limits[1]);
    trfunc->SetParLimits(1,rhoLimits[0],rhoLimits[1]);
    trfunc->SetParLimits(2,vLimits[0],vLimits[1]);
    rtg->trgr->SetTitle("Dines' R-t sqrt(..)");
  }
  else {
    trfunc = new TF1("trfunc",pol3deg,0,3,4);
    rtfunc = new TF1("rtfunc",pol3deg,rtg->mintime,200,4);
    oldrtfunc = new TF1("oldrtfunc",pol3deg,rtg->mintime,200,4);
    trfunc->SetParameters(0.0, 0.0, 0.0, 0.0);
    rtg->trgr->SetTitle("Polynomial R-t");
  }

  // m_t-r relation
  trfunc->SetRange(0,2);
  if (opt.find('3')==std::string::npos) trfunc->FixParameter(3,0);
  trfunc->SetLineColor(4) ;
  rtg->trgr->Fit(trfunc,"QR"); // always fit the m_t-r relation
  if (!bequiet) rtg->trgr->Write();


  // r-m_t relation
  rtfunc->SetRange(0,45);
  if (opt.find('3')==std::string::npos) rtfunc->FixParameter(3,0);
  rtfunc->SetLineColor(4) ;
  if (m_isdines) { 
    rtfunc->SetParameters(trfunc->GetParameter(0),trfunc->GetParameter(1),trfunc->GetParameter(2),trfunc->GetParameter(3));
   
  }
  else { // else do a fit
    rtfunc->SetParameters(0.000000e+00, 6.269950e-02, -3.370054e-04, -1.244642e-07);
    rtg->rtgr->Fit(rtfunc,"QR"); //fit Rt first time
    for (int ipnt=0; ipnt<rtg->rtgr->GetN(); ipnt++){ //calculate m_t-errors
      rtg->rtgr->SetPointError(ipnt , rtg->rtgr->GetEY()[ipnt]/rtfunc->Derivative(rtg->rtgr->GetX()[ipnt]) , rtg->rtgr->GetEY()[ipnt]);
    }
    rtg->rtgr->Fit(rtfunc,"QR"); //fit again

  }
  if (!bequiet)  rtg->rtgr->Write();

  // old r-m_t relation
  oldrtfunc->SetRange(0,45);
  oldrtfunc->SetLineColor(1) ;
  oldrtfunc->SetLineStyle(2) ;
  oldrtfunc->SetLineWidth(1) ;
  oldrtfunc->SetParameters(data[key].oldrtpar[0],data[key].oldrtpar[1],data[key].oldrtpar[2],data[key].oldrtpar[3]);

  if (!bequiet) oldrtfunc->Write();

  rtpars[0] = rtfunc->GetParameter(0);
  rtpars[1] = rtfunc->GetParameter(1);
  rtpars[2] = rtfunc->GetParameter(2);
  rtpars[3] = rtfunc->GetParameter(3);

  //get the t0 from the tr relation
  float tdrift = 20;
 
  if (!m_isdines) {
    float rdrift = 0.0;
    int ntries = 0;
    float precision = 0.0001;
    int maxtries = 500;
    float drdt;
    float driftradius = rtpars[0]+tdrift*(rtpars[1]+tdrift*(rtpars[2]));
    float residual = std::abs(rdrift) - driftradius;
    while (std::abs(residual) > precision) {
      
      drdt = rtpars[1]+tdrift*(2*rtpars[2]);
      tdrift = tdrift + residual/drdt;
      
      driftradius = rtpars[0]+tdrift*(rtpars[1]+tdrift*(rtpars[2]));
      residual = rdrift - driftradius;
      
      ntries = ntries + 1;
      if (ntries>maxtries){
        break;
      }
    }
  }
  
  if (opt.find('0')==std::string::npos) {
    if (m_isdines){
      data[key].rtpar[0] = rtpars[0];
      data[key].rtpar[1] = rtpars[1];
      data[key].rtpar[2] = rtpars[2];
      data[key].rtpar[3] = 0;
    } else {
      data[key].rtpar[0] = 0;
      data[key].rtpar[1] = rtpars[1] + 2*tdrift*rtpars[2] + 3*tdrift*tdrift*rtpars[3];
      data[key].rtpar[2] = rtpars[2] + 3*tdrift*rtpars[3];
      data[key].rtpar[3] = rtpars[3];
    }
  }
  else {
    data[key].rtpar[0] = rtpars[0];
    data[key].rtpar[1] = rtpars[1];
    data[key].rtpar[2] = rtpars[2];
    data[key].rtpar[3] = rtpars[3];
  }
 
  data[key].rtgraph=rtg;

  delete rtfunc;
  delete oldrtfunc;
  delete trfunc;

  return 0.0;
}


float Calibrator::FitTimeResidual ATLAS_NOT_THREAD_SAFE (const std::string& key, TH1F* tresHist){ // Global gStyle is used.

  float mean = tresHist->GetMean();
  float rms = tresHist->GetRMS();
  float val,maxval=0;
  int maxbin=1;
  for (int j=1; j<=tresHist->GetNbinsX()+1;j++){
    val=tresHist->GetBinContent(j);
    if (val>maxval){
      maxval=val;
      maxbin=j;
    }
  }  


  data[key].tresMean =  0;
  float fitmean=tresHist->GetBinCenter(maxbin);  
  if (std::abs(fitmean)>5.0){ 
    data[key].tresMean =  mean;
    data[key].t0err = rms;
    data[key].t0fittype = 1;
    return mean;
  }

  float fitrms=rms;
  if (fitrms>5.0) fitrms=5.0;
  
  tresHist->Fit("gaus","QR","",mean-rms,mean+rms);
  if (tresHist->GetFunction("gaus")->GetParameter(1) - tresHist->GetFunction("gaus")->GetParameter(2) < m_mintres ||
      tresHist->GetFunction("gaus")->GetParameter(1) + tresHist->GetFunction("gaus")->GetParameter(2) > m_maxtres) {
    data[key].tresMean =  mean;
    data[key].t0err = rms;
    data[key].t0fittype = 1;
    return mean;
  }

  tresHist->Fit("gaus","QR","",tresHist->GetFunction("gaus")->GetParameter(1) - tresHist->GetFunction("gaus")->GetParameter(2),tresHist->GetFunction("gaus")->GetParameter(1) + tresHist->GetFunction("gaus")->GetParameter(2));
  if (tresHist->GetFunction("gaus")->GetParameter(1) - tresHist->GetFunction("gaus")->GetParameter(2) < m_mintres ||
      tresHist->GetFunction("gaus")->GetParameter(1) + tresHist->GetFunction("gaus")->GetParameter(2) > m_maxtres) {
    data[key].tres = tresHist->GetFunction("gaus")->GetParameter(2);
    data[key].tresMean = tresHist->GetFunction("gaus")->GetParameter(1);
    data[key].t0err = tresHist->GetFunction("gaus")->GetParError(1);
    data[key].t0fittype = 2;
    return tresHist->GetFunction("gaus")->GetParameter(1);
  }

  tresHist->Fit("gaus","QR","",tresHist->GetFunction("gaus")->GetParameter(1) - tresHist->GetFunction("gaus")->GetParameter(2),tresHist->GetFunction("gaus")->GetParameter(1) + tresHist->GetFunction("gaus")->GetParameter(2));
  if (tresHist->GetFunction("gaus")->GetParameter(1) - tresHist->GetFunction("gaus")->GetParameter(2) < m_mintres ||
      tresHist->GetFunction("gaus")->GetParameter(1) + tresHist->GetFunction("gaus")->GetParameter(2) > m_maxtres) {
    data[key].tres = tresHist->GetFunction("gaus")->GetParameter(2);
    data[key].tresMean = tresHist->GetFunction("gaus")->GetParameter(1);
    data[key].t0err = tresHist->GetFunction("gaus")->GetParError(1);
    data[key].t0fittype = 2;
    return tresHist->GetFunction("gaus")->GetParameter(1);
  }

  tresHist->Fit("gaus","QR","",tresHist->GetFunction("gaus")->GetParameter(1) - tresHist->GetFunction("gaus")->GetParameter(2),tresHist->GetFunction("gaus")->GetParameter(1) + tresHist->GetFunction("gaus")->GetParameter(2));
  if (tresHist->GetFunction("gaus")->GetParameter(1) - tresHist->GetFunction("gaus")->GetParameter(2) < m_mintres ||
      tresHist->GetFunction("gaus")->GetParameter(1) + tresHist->GetFunction("gaus")->GetParameter(2) > m_maxtres) {
    data[key].tresMean =  mean;
    data[key].t0err = rms;
    data[key].t0fittype = 1;
    return mean;
  }

  data[key].tres = tresHist->GetFunction("gaus")->GetParameter(2);
  data[key].tresMean = tresHist->GetFunction("gaus")->GetParameter(1);
  data[key].t0err = tresHist->GetFunction("gaus")->GetParError(1);
  data[key].t0fittype = 2;
 
  // protection against wrong fits:
  if ( std::abs(tresHist->GetFunction("gaus")->GetParameter(2))>10 || std::abs(tresHist->GetFunction("gaus")->GetParameter(1))>10   ) {
    
    data[key].tres = tresHist->GetRMS();
    data[key].tresMean =  tresHist->GetMean();
    data[key].t0err = tresHist->GetRMS();
    data[key].t0fittype = 6;
    return tresHist->GetMean(); 
  }
  
  
  gStyle->SetOptFit(1111);
  
  return tresHist->GetFunction("gaus")->GetParameter(1);
  }


float Calibrator::FitResidual ATLAS_NOT_THREAD_SAFE (const std::string& key, TH1F* resHist){ // Global gStyle is used.

  float mean = resHist->GetMean();
  //float rms = resHist->GetRMS();

  resHist->Fit("gaus","QRI","",mean-0.3,mean+0.3);
  resHist->Fit("gaus","QRI","",resHist->GetFunction("gaus")->GetParameter(1) - 1.5*resHist->GetFunction("gaus")->GetParameter(2),resHist->GetFunction("gaus")->GetParameter(1) + 1.5*resHist->GetFunction("gaus")->GetParameter(2));
  resHist->Fit("gaus","QRI","",resHist->GetFunction("gaus")->GetParameter(1) - 1.5*resHist->GetFunction("gaus")->GetParameter(2),resHist->GetFunction("gaus")->GetParameter(1) + 1.5*resHist->GetFunction("gaus")->GetParameter(2));
  resHist->Fit("gaus","QRI","",resHist->GetFunction("gaus")->GetParameter(1) - 1.5*resHist->GetFunction("gaus")->GetParameter(2),resHist->GetFunction("gaus")->GetParameter(1) + 1.5*resHist->GetFunction("gaus")->GetParameter(2));

  gStyle->SetOptFit(1111);

  data[key].res=resHist->GetFunction("gaus")->GetParameter(2);
  data[key].resMean=resHist->GetFunction("gaus")->GetParameter(1);
  data[key].reserr=resHist->GetFunction("gaus")->GetParError(2);

  return resHist->GetFunction("gaus")->GetParameter(2);

}

TDirectory* Calibrator::Calibrate ATLAS_NOT_THREAD_SAFE (TDirectory* dir, std::string key, const std::string& opt, caldata * caldata_above){ // Thread unsafe FitResidual, FitRt, FitTimeResidual are used.

  //set some bool flags
  bool calrt=opt.find('R')!=std::string::npos;
  bool calt0=opt.find('T')!=std::string::npos;
  bool donothing=opt.find('N')!=std::string::npos;
  bool prnt=opt.find('P')!=std::string::npos;
  bool useref=opt.find('B')!=std::string::npos;
  bool useshortstw=opt.find('S')!=std::string::npos;
  
  if (donothing) return dir;

  data[key].calflag=true; 

  bool enough_t0 = data[key].ntres>=m_mint0stat;
  bool enough_rt = data[key].nrt>=m_minrtstat;

  // TDirectory* newdir;
  if ((enough_rt && calrt) || (enough_t0 && calt0)) {   
    m_hdirs[key] = dir->mkdir(Form("%s%s",m_name.data(),key.data()));
    m_hdirs[key]->cd();
  }
  else m_hdirs[key]=dir;
  
      
  //Fit also the residual if an rt or t0 calibration is made
  if ((int)data[key].nres>50 && ((enough_rt && calrt) || (enough_t0 && calt0))) {    

    m_resHists[key] = new TH1F("residual","residual",m_nbinsres,m_minres,m_maxres);
    for (int i=0;i<100;i++) {
      m_resHists[key]->SetBinContent(i+1,data[key].reshist[i]);
    }
    m_resHists[key]->SetEntries((int)data[key].nres);
    if (bequiet) m_resHists[key]->SetDirectory(nullptr);                                 
    
    FitResidual(key,m_resHists[key]);

  }
  

  if (prnt) printf("TRTCalibrator: %8s %-14s: ",m_name.data(),key.data()); 

  //Calibrate r-m_t
  if (nort){
    //use old data
    data[key].rtflag=true;
    data[key].rtt0=caldata_above->rtt0;
    data[key].rtgraph=caldata_above->rtgraph;
    for (int i=0;i<4;i++) data[key].rtpar[i]=data[key].oldrtpar[i];
    if (prnt) printf("RT << %7i (%8.1e) %8.1e %8.1e %8.1e, %3.2f  : ", data[key].nrt,data[key].rtpar[0],data[key].rtpar[1],data[key].rtpar[2],data[key].rtpar[3], data[key].rtt0); 
  }
  else{
    //do fit
    if ((calrt && enough_rt) || level==0) {
      data[key].rtflag=true;
      m_rtHists[key] = new TH2F("rt-relation","rt-relation",m_nbinst,m_mint,m_maxt,m_nbinsr,m_minr,m_maxr); //make a root histogram
      for (int i=0;i<m_nbinst;i++) {
        for (int j=0;j<m_nbinsr;j++) {
          m_rtHists[key]->SetBinContent(i+1,j+1,data[key].rthist[i+m_nbinst*j]);
        }
      }
      m_rtHists[key]->SetEntries(data[key].nrt);
      if (bequiet) {
        m_rtHists[key]->SetDirectory(nullptr);
      }
      data[key].rtt0=FitRt(key,opt,m_rtHists[key],m_hdirs[key]); //do the fit
      if (prnt) printf("RT    %7i (%8.1e) %8.1e %8.1e %8.1e, %3.2f  : ", data[key].nrt, data[key].rtpar[0], data[key].rtpar[1], data[key].rtpar[2], data[key]. rtpar[3], data[key].rtt0);     
    }
    else{
      //use data from level above
      data[key].rtgraph=caldata_above->rtgraph;
      data[key].rtt0=caldata_above->rtt0;
      for (int i=0;i<4;i++) data[key].rtpar[i]=caldata_above->rtpar[i];
      if (prnt) printf("RT /\\ %7i (%8.1e) %8.1e %8.1e %8.1e, %3.2f  : ", data[key].nrt,data[key].rtpar[0],data[key].rtpar[1],data[key].rtpar[2],data[key].rtpar[3], data[key].rtt0); 
    }
  }


  //Calibrate t0
  if (not0){ 
    //use old data
    data[key].t0flag=true;
    data[key].t0=data[key].oldt02 + data[key].rtt0;
    data[key].t0err=0;
    data[key].t0off=0;
    data[key].t0fittype = 5;
    if (prnt) printf("T0 << %7i  %05.2f%+05.2f%+05.2f=%05.2f", data[key].ntres, data[key].oldt02, 0.0, data[key].rtt0, data[key].t0);       
  }
  else{
    if (useref && level==5){
      //use chip reference values 
      data[key].t0flag=true;
      data[key].t0=caldata_above->t0 + data[key].reft0 + data[key].rtt0;
      data[key].t0err=caldata_above->t0err;
      data[key].t0off=data[key].t0-caldata_above->t0;
      data[key].t0fittype = 3;
      if (prnt) printf("T0 ** %7i  %05.2f%+05.2f%+05.2f=%05.2f", data[key].ntres, caldata_above->t0, data[key].reft0, data[key].rtt0, data[key].t0); 
    }
    else {
      //do fit
      if ((calt0 && enough_t0) || level==0) {
        data[key].t0flag=true;
        m_tresHists[key] = new TH1F("timeresidual","time residual",m_nbinstres,m_mintres,m_maxtres); //make a root histogram

        for (int i=0;i<100;i++) {
          m_tresHists[key]->SetBinContent(i+1,data[key].m_treshist[i]);
        }
        m_tresHists[key]->SetEntries(data[key].ntres);
        if (bequiet) {
          m_tresHists[key]->SetDirectory(nullptr);
        }      

        data[key].t0=data[key].oldt02 + FitTimeResidual(key,m_tresHists[key]) + data[key].rtt0 + m_t0shift; //do the fit and modify t0
        data[key].t0off=data[key].t0-caldata_above->t0; //calculate t0 offset from level above
        if (data[key].t0<0) data[key].t0=0;
        if (prnt) printf("T0    %7i  %05.2f%+05.2f%+05.2f%+05.2f=%05.2f", data[key].ntres, data[key].oldt02, data[key].t0-data[key].oldt02-data[key].rtt0, data[key].rtt0, m_t0shift, data[key].t0); 


      }
      //use data from level above
      else { 
        //TEMP FIX to dont destroy right T0s
        if (data[key].oldt02 + (caldata_above->t0 - caldata_above->oldt02)  >0)    data[key].t0=data[key].oldt02 + (caldata_above->t0 - caldata_above->oldt02);
        else data[key].t0= 0;
        //TEMP FIX to dont destroy right T0s
       
      //add the short straw correction here. In this way, the shift is only done when contants at STRAW level come from level above.
        if ((level == 6 && useshortstw) && std::abs(data[key].det)<2  && (data[key].lay==0 && data[key].stl<9) )         data[key].t0=caldata_above->t0-0.75; 
        data[key].t0err=caldata_above->t0err;
        data[key].t0off=data[key].t0-caldata_above->t0 + data[key].rtt0;
        data[key].t0fittype = 4;
//      if (prnt) printf("T0 /\\ %7i  %05.2f%+05.2f%+05.2f=%05.2f", data[key].ntres, caldata_above->t0, 0.0, 0.0, data[key].t0); 
      if (prnt) printf("T0 /\\ %7i  %05.2f%+05.2f%+05.2f=%05.2f", data[key].ntres, caldata_above->t0, caldata_above->oldt02, data[key].oldt02 ,  data[key].t0); 
      }
    }
  }


  if (prnt && !bequiet) std::cout << " H";

  if (prnt) std::cout << std::endl;


  return m_hdirs[key];

}


int Calibrator::AddHit(const std::string& key, databundle d, int* binhist, bool makehist){
  
  int tresbin=Simple1dHist(m_mintres,m_maxtres,m_nbinstres,d.tres);
  int resbin=Simple1dHist(m_minres,m_maxres,m_nbinsres,d.res);
  int rtbin=Simple2dHist(m_mint,m_maxt,m_nbinst,m_minr,m_maxr,m_nbinsr,d.t,d.r);
  int npop;
  int ibin;
  int value;
  
  //if the it is the first hit or histogram
  if (data.find(key) == data.end()){
    
    //create a new histogram object
    caldata* hist=new caldata(makehist,m_nbinst,m_nbinsr);    
 
    //out of memory?
    if (hist == nullptr){
      std::cout << "OUT OF MEMORY!" << std::endl;
      return -1;
    }

    hist->ntres=0;
    hist->nres=0;
    hist->nrt=0;
    hist->sumt0=0;

    //zero histogram bins
    for (int i=0;i<m_nbinsr*m_nbinst+200;i++) {
      if (makehist) hist->rthist[i]=0;
      if (i<100) {
        if (makehist) hist->m_treshist[i]=0;
        if (makehist) hist->reshist[i]=0;
      }
    }

    if (binhist==nullptr){ //if it is a hit
      
      if (tresbin>=0){ 
        if (makehist) hist->m_treshist[tresbin]=d.weight; //add value to bin
        hist->ntres=1; //set bin content to 1
        m_nhits=1;
      }
      if (resbin>=0){ 
        if (makehist) hist->reshist[resbin]=1;
        hist->nres=1;
      }
      if (rtbin>=0){ 
        if (makehist) hist->rthist[rtbin]=1;
        hist->nrt=1;
      }

      hist->sumt0=d.t0;
    }
    else { //if it is a histogram

      npop=binhist[0]; //the number of populated (non-zero) bins
      
      for(int ipop=2;ipop<2*npop+2;ipop=ipop+2) { //loop over the data

        ibin=binhist[ipop]; //bin number
        value=binhist[ipop+1]; //value

        if (ibin<100) { //timeresidual histogram
          m_ntreshits+=value;
          hist->ntres+=value;
          m_nhits+=value;
          if (makehist) hist->m_treshist[ibin]=(float)value;
          hist->sumt0+=d.t0*value;
        }
        else if (ibin>=100 && ibin<200) { //residual histogram
          m_nreshits+=value;
          hist->nres+=value;
          if (makehist) hist->reshist[ibin-100]=(float)value;
        }
        else { //rt histogram
          m_nrthits+=value;
          hist->nrt+=value;
          if (makehist) hist->rthist[ibin-200]=(float)value;
        }

      }    
    }

    //initialize parameters
    if (level>0) hist->det=d.det; else hist->det=-3; //
    if (level>1) hist->lay=d.lay; else hist->lay=-3;
    if (level>2) hist->mod=d.mod; else hist->mod=-3;
    if (level>3) hist->brd=d.brd; else hist->brd=-3;
    if (level>4) hist->chp=d.chp; else hist->chp=-3;
    if (level>5) hist->stw=d.stw; else hist->stw=-3;
    if (level>2) hist->stl=d.stl; else hist->stl=-3;
    if (level>5) hist->sid=d.sid; else hist->sid=-3;
    hist->res=0;
    hist->resMean=0;
    hist->reserr=0;
    hist->tres=0;
    hist->tresMean=0;
    hist->t0=0;
    if (level==5) hist->reft0=d.rt0; else hist->reft0=0;
    hist->t0off=0;
    hist->t0err=0;
    hist->nhits=1;
    hist->t0fittype=0;
    
    hist->x=d.x; //straw x coordinate
    hist->y=d.y; //straw y coordinate
    hist->z=d.z; //straw z coordinate

    hist->oldt02=d.t0; //straw old t0 value
    hist->oldrtpar[0]=d.rtpar[0];
    hist->oldrtpar[1]=d.rtpar[1];
    hist->oldrtpar[2]=d.rtpar[2];
    hist->oldrtpar[3]=d.rtpar[3];

    hist->sumx=0;
    hist->sumy=0;
    hist->sumz=0;

    hist->rtflag=false;
    hist->t0flag=false;
    hist->calflag=false;


    for (unsigned int i =0; i < 4; i++){
      hist->rtpar[i]=-10.0;
    }

    data[key]=*hist; //save the histogram in the map 

    delete hist;

    data[key].oldt02 = AccumulativeMean(data[key].nhits, data[key].oldt02, d.t0); //update old t0 m_mean value

    data[key].nhits++; // increment m_nhits

    return 1;
  }
  else { //if not the first hit or histogram

    //increment histogram bins
    if (binhist==nullptr){
      
      if (tresbin>=0){
        if (makehist) data[key].m_treshist[tresbin]=data[key].m_treshist[tresbin]+d.weight;
        data[key].ntres++;
        m_nhits=1;
      }
      if (resbin>=0){
        if (makehist) data[key].reshist[resbin]++;
        data[key].nres++;
      }
      if (rtbin>=0){
        if (makehist) data[key].rthist[rtbin]++;
        data[key].nrt++;
      }

      data[key].sumt0+=d.t0;

    }

    else {

      npop=binhist[0];

      for(int ipop=2;ipop<2*npop+2;ipop=ipop+2) {

        ibin=binhist[ipop];
        value=binhist[ipop+1];

        if (ibin<100) {
          m_ntreshits+=value;
          m_nhits+=value;
          data[key].ntres+=value;
          if (makehist) data[key].m_treshist[ibin]+=(float)value;
          data[key].sumt0+=d.t0*value;
        }
        else if (ibin>=100 && ibin<200) {
          m_nreshits+=value;
          data[key].nres+=value;
          if (makehist) data[key].reshist[ibin-100]+=(float)value;
        }
        else {
          m_nrthits+=value;
          data[key].nrt+=value;
          if (makehist) data[key].rthist[ibin-200]+=(float)value;
        }

      }    
      
    }

        
    data[key].sumx+=d.x;
    data[key].sumy+=d.y;
    data[key].sumz+=d.z;
    data[key].oldt02 = AccumulativeMean(data[key].nhits, data[key].oldt02, d.t0);
    data[key].oldrtpar[0] = AccumulativeMean(data[key].nhits, data[key].oldrtpar[0], d.rtpar[0]);
    data[key].oldrtpar[1] = AccumulativeMean(data[key].nhits, data[key].oldrtpar[1], d.rtpar[1]);
    data[key].oldrtpar[2] = AccumulativeMean(data[key].nhits, data[key].oldrtpar[2], d.rtpar[2]);
    data[key].oldrtpar[3] = AccumulativeMean(data[key].nhits, data[key].oldrtpar[3], d.rtpar[3]);
    data[key].x = AccumulativeMean(data[key].nhits, data[key].x, d.x);
    data[key].y = AccumulativeMean(data[key].nhits, data[key].y, d.y);
    data[key].z = AccumulativeMean(data[key].nhits, data[key].z, d.z);

    data[key].nhits++; //increment hit counts

    return 0;
  }    


}

void Calibrator::WriteStat(TDirectory* dir){
  dir->cd();
  TNtuple* stattup = new TNtuple(Form("%stuple",m_name.data()),"statistics","det:lay:mod:brd:chp:sid:stl:stw:rtflag:t0flag:t0:oldt0:rt0:dt0:t0offset:ftype:nhits:nt0:nrt:res:resMean:dres:tres:tresMean:x:y:z");
  for(std::map<std::string,caldata>::iterator ihist=data.begin(); ihist!=data.end(); ++ihist){
    if ((ihist->second).calflag) {
      float const ntvar[27]={
        float((ihist->second).det),
        float((ihist->second).lay),
        float((ihist->second).mod),
        float((ihist->second).brd),
        float((ihist->second).chp),
        float((ihist->second).sid),
        float((ihist->second).stl),
        float((ihist->second).stw),
        float((int)(ihist->second).rtflag),
        float((int)(ihist->second).t0flag),
        (ihist->second).t0,
        float((ihist->second).oldt02),
        //oldt0(ihist->first),
        (ihist->second).reft0,
        (ihist->second).t0err,
        (ihist->second).t0off,
        float((ihist->second).t0fittype),
        (ihist->second).nhits,
        (float)(ihist->second).ntres,
        (float)(ihist->second).nrt,
        (float)(ihist->second).res,
        (float)(ihist->second).resMean,
        (ihist->second).reserr,
        (ihist->second).tres,
        (ihist->second).tresMean,
        (ihist->second).x,
        (ihist->second).y,
        (ihist->second).z
      };      
      stattup->Fill(ntvar);
    }
      
  }

  stattup->Write();
  stattup->Delete();
}

void Calibrator::DumpConstants(){

  std::ofstream dumpfile( "calib_constants_out.txt", std::ios::out | std::ios::app);

  for (const std::pair<const std::string, caldata>& p : data) {
    dumpfile << p.first << " " << p.second.t0 << " " << std::endl;
    
  }
  
  dumpfile.close();
  
}
