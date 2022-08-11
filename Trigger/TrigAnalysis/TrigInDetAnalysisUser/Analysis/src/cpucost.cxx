/**
 **     @file    cpucost.cxx
 **
 **     @author  ben sowden
 **     @date    Mon 04 Aug 2014 10:45:00 BST
 **
 **     Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 **/


#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h> 

#include <iostream>
#include <string>
#include <vector>

#include "utils.h"
#include "label.h"
#include "DrawLabel.h"

#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TSystem.h"

#include "TKey.h"
#include "TList.h"
#include "TDirectory.h"

#include "computils.h"

#include "AtlasStyle.h"
#include "AtlasLabels.h"

/// Prints usage instructions to standard output and returns given status
int usage(const std::string& name, int status) {
  std::ostream& s = std::cout;
  s << "Usage: " << name << " [OPTIONS] expert-monitoring.root reference.root algorithm1 algorithm2 algorithm3 ...\n\n";
  s << "  TIDA \'" << name << "\' extracts timing histograms\n\n";
  s << "Options: \n";
  s << "    -o,  --outputfolder value\t puts output in folder 'value' making it if it doesn't exist, \n\n";
  s << "    -t,  --tag       value   \t appends tag 'value' to the end of output plot names, \n";
  s << "    -k,  --key       value   \t prepends key 'value' to the front of output plot names, \n\n";
  s << "    -a,  --auto              \t process all histograms that are in the file, \n";
  s << "    -r,  --replace   patt rep\t replace patt wiht rep in the file name\n"; 
  s << "    -d,  --directory value   \t if auto is set, search only in specifed directory, \n";
  s << "         --nodir             \t do not print the directory name on the plot,\n";
  s << "    -p,  --pattern   value   \t if auto is set, search for histograms containing this string, \n\n";
  s << "    -f,  --frac              \t explicitly include the fractional plots\n";
  s << "    -nr, --noref             \t do not use the reference file, \n\n";
  s << "    -x,  --xoffset   value   \t offset the key by value \n";
  s << "         --logx               \t force logx \n";
  s << "    -w,  --binwidth          \t normalise by bin width\n";
  s << "    -as, --atlasstyle        \t use the ATLAS style \n\n";
  s << "    -al, --atlaslabel        \t show the ATLAS label \n\n";
  s << "    -v,  --verbose           \t verbose output\n\n";
  s << "    -h,  --help              \t this help\n";
  s << std::endl;
  return status;
}



struct histoinfo {
  histoinfo( const std::string& f, const std::string& d ) : fname(f), dname(d) { } 
  std::string fname; // File name
  std::string dname; // Display name
};


std::ostream& operator<<( std::ostream& s, const histoinfo& h ) { 
  return s << h.fname << " : " << h.dname; 
}


void binwidth( TH1F* h ) {
  for ( int i=0 ; i<h->GetNbinsX() ; i++ ) {
    double w = h->GetBinLowEdge(i+2)-h->GetBinLowEdge(i+1);
    h->SetBinContent( i+1, h->GetBinContent(i+1)/w );
    h->SetBinError( i+1, h->GetBinError(i+1)/w );
  }
}


int main(int argc, char** argv) {

  if (argc < 4) { return usage(argv[0], -1); }

  std::string output_dir = "";
  std::string tag = "";
  std::string key = "";

  gStyle->SetPadRightMargin(0.05);
  gStyle->SetPadTopMargin(0.05);

  gStyle->SetPadLeftMargin(0.14);
  gStyle->SetPadBottomMargin(0.14);
  //  gStyle->SetTitleXOffset(0.1);
  //  gStyle->SetTitleYOffset(0.1);


  TFile* ftest = 0;
  TFile* fref  = 0;

  bool atlasstyle = false;
  bool atlaslabel = false;
  bool ylog = true;
  bool nopng = false;

  bool autochains = false;

  std::string autopattern = "";

  std::vector<std::string> taglabels;

  std::string directory = "TIMERS";
  std::vector<std::string> patterns;

  TDirectory* tdir = gDirectory;

  bool verbose = false;
  
  bool noref = false;

  std::string frefname = "";

  double xoffset = 0.17;

  bool show_directory = true;

  bool norm_width = true;

  bool logx = false;

  bool withlumiblock = false;

  bool withfractional = false;

  std::vector<std::string> replace_list;

  // Parse the arguments
  std::vector<std::string> algorithms;
  for(int argnum = 1; argnum < argc; argnum++){
    std::string arg = argv[argnum];

    if (arg == "-h" || arg == "--help") {
      return usage(argv[0], 0);
    }
    else if (arg == "-o" || arg == "--outputfolder") {
      if (++argnum < argc) { output_dir = argv[argnum]; } 
      else { return usage(argv[0], -1); }
    }
    else if (arg == "-x" || arg == "--xoffset") {
      if (++argnum < argc) { xoffset = std::atof(argv[argnum]); } 
      else { return usage(argv[0], -1); }
    }
    else if (arg == "-t" || arg == "--tag") {
      if (++argnum < argc) { tag = std::string("-") + argv[argnum]; }
      else { return usage(argv[0], -1); }
    }
    else if (arg == "-k" || arg == "--key") {
      if (++argnum < argc) { key = argv[argnum] + std::string("-"); }
      else { return usage(argv[0], -1); }
    }
    else if (arg == "-r" || arg == "--replace") {
      if (++argnum < argc) replace_list.push_back( argv[argnum] );
      else { return usage(argv[0], -1); }
      if (++argnum < argc) replace_list.push_back( argv[argnum] );
      else { return usage(argv[0], -1); }
    }
    else if ( arg == "--logx") {
      logx = true;
    }
    else if (arg == "-np" || arg == "--nopng") {
      nopng = true;
    }
    else if (arg == "-f" || arg == "--frac" ) {
      withfractional = true;
    }
    else if (arg == "-a" || arg == "--auto") {
      autochains = true;
    }
    else if (arg == "--nodir") {
      show_directory = false;
    }
    else if (arg == "-v" || arg == "--verbose") {
      verbose = true;
    }
    else if (arg == "-lb" ) {
      withlumiblock = true;
    }
    else if (arg == "-nr" || arg == "--noref") {
      noref = true;
    }
    else if (arg == "-w" || arg == "--binwidth") {
      norm_width = true;
    }
    else if (arg == "-as" || arg == "--atlasstyle") {
      atlasstyle = true;
    }
    else if (arg == "-al" || arg == "--atlaslabel") {
      atlaslabel = true;
    }
    else if (arg == "-ap" || arg == "--autopattern") {
      if (++argnum < argc) autopattern = argv[argnum];
      else                 return usage(argv[0], -1); 
    }
    else if (arg == "-d" || arg == "--directory") {
      if (++argnum < argc) directory = argv[argnum];
      else                 return usage(argv[0], -1); 
    }
    else if (arg == "-p" || arg == "--pattern") {
      if (++argnum < argc) patterns.push_back(argv[argnum]);
      else                 return usage(argv[0], -1); 
    }
    else {
      if (ftest == 0) {
	std::string file = globbed(arg);
        if (exists(file)) {
          ftest = new TFile( file.c_str() );
        }
        else {
          std::cerr << "main(): test file " << arg << " does not exist" << std::endl;
          return -2;
        }
      }
      else if ( frefname=="" ) frefname = arg;
      else {
        algorithms.push_back(arg);
      }
    }
  }

  if ( patterns.empty() ) patterns.push_back( "_TotalTime" );

  
  if (ftest == 0 || ( noref==false && frefname=="" ) ) {
    return usage(argv[0], -4);
  }
  
  if ( fref == 0 && !noref ) {
    std::string file = globbed(frefname);
    if (exists(file)) {
      fref = new TFile( file.c_str() );
    }
    else {
      std::cerr << "main(): ref file " << frefname << " does not exist" << std::endl;
      return -3;
    }
  }

  if ( atlasstyle ) SetAtlasStyle();

  gStyle->SetErrorX(0);

  if ( noref ) fref = ftest;

  if ( noref ) Plotter::setmeanplotref(!noref);

  if ( ftest && autochains ) { 

    ftest->cd();

    std::vector<std::string> dirs;

    contents( dirs, gDirectory, directory, patterns );

    if ( autopattern=="" ) { 
      for ( unsigned j=0 ; j<dirs.size() ; j++ ) { 
	if ( verbose ) std::cout << "\talgorithm " << dirs[j] << std::endl;
	algorithms.push_back( dirs[j] );
      }
    }
    else { 
      std::cout << "autopattern : " << autopattern << std::endl; 
      for ( unsigned j=0 ; j<dirs.size() ; j++ ) { 
	if ( dirs[j].find(autopattern)!=std::string::npos ) { 
	  algorithms.push_back( dirs[j] );
	  std::cout << "adding " << algorithms.back() << std::endl;
	}
      }
    }

  }
  


  // Make output directory
  if ( output_dir != "" ) {
    if ( mkdir( output_dir.c_str(), 0777 ) ) { 
      if ( exists(output_dir) ) std::cerr << "main() directory " << output_dir << " aleady exists" << std::endl; 
      else               std::cerr << "main() could not create directory " << output_dir << std::endl; 
    }
    output_dir += "/";
  }

#if 0
  // Get the timers directories from input files
  //  TDirectoryFile* testtimers = 0;
  TDirectory* testtimers = 0;

  if ( directory!="" ) ftest->GetObject( directory.c_str(), testtimers );
  else                 testtimers = ftest;

  if (testtimers == 0 ) {
    std::cerr << "main(): can not find timers in test file" << std::endl;
    //    return -1;
  }

  //  TDirectoryFile* reftimers = 0;
  TDirectory* reftimers = 0;

  if ( directory!="" ) fref->GetObject( directory.c_str(), reftimers );
  else                 reftimers = fref;

  if (reftimers == 0 ) {
    std::cerr << "main(): can not find timers in ref file" << std::endl;
    // return -1;
  }
#endif

  TFile fcck( "fcck.root", "recreate" );


  std::vector<histoinfo> histograms;
  histograms.push_back( histoinfo("_TotalTime", "Total time") );

  // Provide output to user for progress status
  //  std::cout << "main() processing algorithms : " << algorithms << std::endl;

  // Loop over histograms
  //  for (unsigned int histogram = 0; histogram < histograms.size(); ++histogram) {
  for (unsigned int histogram=histograms.size(); histogram-- ; ) {

    

    std::cout << "\nhistogram " << histograms.at(histogram) << " : with " << algorithms.size() << " algorithms" << std::endl;


    std::string xaxis = histograms.at(histogram).dname + " [ms]";
    std::string yaxis = "Entries";

    
    // Loop over input algorithms
    //    for (unsigned int algorithm = 0; algorithm < algorithms.size(); ++algorithm) {
    for (unsigned int algorithm = algorithms.size(); algorithm-- ; ) {
      
      if ( !withlumiblock && algorithms[algorithm].find("LumiBlock")!=std::string::npos ) continue;

      if ( !withfractional && algorithms[algorithm].find("Fractional")!=std::string::npos ) continue;

      std::cout << "\nmain() processing algorithm : " << algorithms[algorithm] << std::endl;

      TCanvas* c1 = new TCanvas( label("canvas-%d",int(histogram)).c_str(), "histogram", 800, 600 );
      c1->cd();
            
      double x1 = xoffset;
      double x2 = xoffset+0.25;
      double y1 = 0.75;
      double y2 = 0.87;

      /// no longer adjust the legend if no reference times are to be 
      /// plotted as we now more properly set the legend size automatically 
      /// depending on the number of entries

      Legend legend(x1, x2, y1, y2);
     

      std::string histname = algorithms[algorithm]; // + histograms.at(histogram).fname;

      std::string _xaxis = xaxis;
      bool fractional = contains( histname, "Fractional" );
      if ( fractional ) _xaxis = "Fraction of " + histograms.at(histogram).dname;
      

      //      std::cout << "\t" << histname << "\t" << algorithms.at(algorithm) << " " <<  histograms.at(histogram).fname << std::endl;


      //      std::cout << "Directory: " << gDirectory->GetName() << std::endl;

      TH1F* testhist = (TH1F*)ftest->Get(histname.c_str());

      if (testhist == 0 ) {
        std::cerr << "main(): can not find hist " << histname << " in test file" << std::endl;
        continue;
      }

      std::cout << "mean time: " << testhist->GetMean() << "\t:: " << testhist->GetName() << std::endl;

      if ( norm_width ) binwidth( testhist );

      /// skip TH2 and TProfiles for the moment ... 
      if ( std::string(testhist->ClassName()).find("TH1")==std::string::npos ) continue; 
      
      testhist->SetName( tail(algorithms[algorithm],"/").c_str() );
      testhist->Write(); 

      //      std::cout << "\n\nfound histname " << histname << std::endl;

      std::string refhistname = histname;


      TH1F* refhist = (TH1F*)fref->Get(refhistname.c_str());


      /// if we cannot find the reference histogram, try replacing all patterns 
      /// that are requested in the reference hist name 

      if ( refhist==0 && replace_list.size()>=2 ) { 

	for ( size_t ir=0 ; ir<replace_list.size()-1 ; ir+=2 ) {
	  
	  size_t pos = refhistname.find(replace_list[ir]);
	  if ( pos != std::string::npos ) { 
	    
	    while( pos!=std::string::npos ) { 
	      refhistname.replace( pos, replace_list[ir].size(), "XXXX" );
	      pos = refhistname.find(replace_list[ir]);
	    }
	    
	    pos = refhistname.find("XXXX");
	    while( pos!=std::string::npos ) { 
	      refhistname.replace( pos, 4, replace_list[ir+1] );
	      pos = refhistname.find("XXXX");
	    }
	  }
	  
	}

	refhist = (TH1F*)fref->Get(refhistname.c_str());

      }
     
      if (refhist == 0 ) {
        std::cerr << "main(): can not find hist " << refhistname << " in ref file" << std::endl;
	continue;
      }

      if ( norm_width ) binwidth( refhist );

      testhist->GetYaxis()->SetTitle(yaxis.c_str());
      testhist->GetYaxis()->SetTitleOffset(1.5);

      refhist->GetYaxis()->SetTitle(yaxis.c_str());
      refhist->GetYaxis()->SetTitleOffset(1.5);
      
      testhist->GetXaxis()->SetTitle(_xaxis.c_str());
      testhist->GetXaxis()->SetTitleOffset(1.5);

      refhist->GetXaxis()->SetTitle(_xaxis.c_str());
      refhist->GetXaxis()->SetTitleOffset(1.5);


      Plots plots;

      std::string algname = tail(algorithms[algorithm], "/" );
      std::string dirname = tail( head(algorithms[algorithm], "/" ), "/" );
      std::string algpname = algorithms[algorithm];
      replace( algpname, '/', '_' );

      if ( algname.find("h_")==0 ) algname.erase(0, 2);
    
      //      size_t indetpos = algname.find("InDet");
      //      if ( indetpos!=std::string::npos ) algname.erase( indetpos, 5);

      plots.push_back( Plotter( testhist, refhist, " "+algname ) );

      std::string plotname = key + algpname + tag;
      
      std::string stub = directory;

      size_t pos = stub.find('/');
      while ( pos!=std::string::npos ) { stub.erase( pos, 1 ); pos = stub.find('/'); }

      while ( plotname.find(stub)!=std::string::npos ) { 
	plotname.erase( 0, plotname.find(stub)+stub.size() );
      }

      while ( plotname.find('_')==0 ) plotname.erase( 0, 1 );

      plotname = output_dir + plotname;

      std::cout << "output dir " << output_dir << "\tkey " << key << "\talgname " << algname << "\ttag " << tag << std::endl;  

      //      std::cout << "testhist " << testhist << " " << refhist << std::endl;


      std::vector<std::string> chains;
      chains.push_back( algname + tag );

      bool ylogt = ylog;
      
      double Nent     = plotable( testhist );
      double Nent_ref = plotable( refhist ); 
     
      if ( fractional ) ylogt = false;

      if ( Nent==0 || Nent_ref==0 ) { 
	ylogt = false;
	std::cerr << "histograms empty: " << testhist->GetName() << std::endl;
	continue;
      }


      testhist->SetTitle("");
      refhist->SetTitle("");


      plots.SetLogy(ylogt);

      if ( logx ) plots.SetLogx(true); 

      double rmin = plots.realmin();
      double rmax = plots.realmax();
      
      if ( rmin == rmax ) rmin = 0;
	
      if ( ylogt ) { 
	  if ( rmin == 0 ) rmin = rmax*0.0001; 
	  double delta = std::log10(rmax)-std::log10(rmin);
	  if ( atlasstyle ) plots.Max( rmax*std::pow(10,delta*0.15*2*(chains.size()+taglabels.size()+1.5)) );
	  else              plots.Max( rmax*std::pow(10,delta*0.15*2*(chains.size()+taglabels.size()+1.0)) );
	  plots.Min( rmin*std::pow(10,-delta*0.1) );
      }
      else { 
	  double delta = rmax-rmin;
	  plots.Max( rmax+delta*0.1*2*chains.size() );

	  double pmin = rmin-delta*0.1; 
	  if ( pmin>0 ) plots.Min( pmin );
	  else          plots.Min( 0 );
	  
      }
	
      std::vector<double> range = plots.findxrange();

      double lower = range[0];
      double upper = range[1];

      if ( lower<0 ) lower = 0;

      plots.SetRangeUser( lower, upper );

      plots.Draw( legend, true );

      std::string dirtitle = dirname;
      if ( dirtitle.find("HLT_")==0 && dirtitle.find("__")!=std::string::npos ) dirtitle.erase( dirtitle.find("__"), dirtitle.size() ); 

      if ( show_directory ) DrawLabel( x1+0.02, y2+0.02, dirtitle, kBlack, legend.TextSize(), legend.TextFont() );

      if ( atlasstyle && atlaslabel ) ATLASLabel(0.68, 0.88, "Internal");

      /// could simply run gPad->SetLogyx( logx );
      /// but that would interfere with the individual plot 
      /// setting from the config file 
      if ( logx ) gPad->SetLogx(true); 

      plots.back().Print( (plotname+".pdf").c_str() );
      if ( !nopng ) plots.back().Print( (plotname+".png").c_str() );

      delete c1;

      std::cout << "done algorithm " << algorithm << " " << algorithms[algorithm] << std::endl;
    }

    std::cout << "done hist " << histogram << " " << histograms.at(histogram).dname << " " << std::endl;
  }

  fcck.Write();
  fcck.Close();

  tdir->cd();


#ifdef USE_SLOW_ROOT_FILE_DELETION

  std::cout << "deleting ftest" << std::endl;

  /// AAAAARGH!!!! these deletes do not work!!! they just sit there for ever!!
  ///              what is wrong with these destructors !!! I ask you


  if ( fref!=ftest ) { 
    std::cout << "deleting fref" << std::endl;  
    
    //  delete reftimers;
    delete fref;
  }

  //  delete testtimers;
  delete ftest;

#endif

  std::cout << "done" << std::endl;

  return 0;
}
