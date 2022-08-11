#!/usr/bin/env python
#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# @file:    rootcomp.py
# @purpose: Script to compare the histograms in two root files
# @author:  Frank Winklmeier, Will Buttinger
#
from __future__ import print_function
import sys
import os
import os.path

def main():
      
   from optparse import OptionParser
   parser = OptionParser(usage = "usage: %prog [options] reference file",
                         description = "Compare the histograms in two root files. See https://twiki.cern.ch/twiki/bin/view/Atlas/TrigValTools#rootcomp_py for more details and examples.")
   
   parser.add_option("-c", "--chi2",
                     action = "store_true",
                     help = "use chi2 comparison instead of bin-by-bin")

   parser.add_option("-a", "--axis",
                     action = "store_true",
                     help = "use axis comparison instead of bin-by-bin")

   parser.add_option("-l", "--sortLabels",
                     action = "store_true", default=False,
                     help = "sort/deflate alphanumeric axis before comparing")

   parser.add_option("-t", "--threshold",
                     action = "store", type="float",
                     help = "threshold for bin or chi2 comparison (default: 1e-6 or 0.95)")

   parser.add_option("-o", "--output",
                     action = "store", dest = "outFile",
                     metavar = "NAME", default = "rootcomp",
                     help = "write output to NAME.[ps,root] (default: rootcomp)")
   
   parser.add_option("-s", "--skip",
                     action = "append", default=[], type="string",
                     help = "add objects to skip (regular expression, can be used multiple times, mutual exclusive with -e)")

   parser.add_option("-e", "--select",
                     action = "append", default=[], type="string",
                     help = "select histograms to compare (regular expression, can be used multiple times, mutual exclusive with -s)")

   parser.add_option("--refBaseDir",
                     action = "store", default = "",
                     help = "base directory of reference file")

   parser.add_option("--fileBaseDir",
                     action = "store", default = "",
                     help = "base directory of file")
   
   parser.add_option("-S", "--noSkipList",
                     action = "store_true", default=False,
                     help = "do not use default list of histograms to skip")
      
   parser.add_option("-n", "--noRoot",
                     action = "store_true",
                     help = "do not write .root file")

   parser.add_option("--noPS",
                     action = "store_true",
                     help = "do not write ps/pdf file")
   
   parser.add_option("-N", "--norm",
                     action = "store_true", default = False,
                     help = "draw normalized")

   parser.add_option("--noDiff",
                     action = "store_true", default = False,
                     help = "do not plot difference histogram")

   parser.add_option("--ignoreMissingRef",
                     action = "store_true", default = False,
                     help = "ignore missing references for overall test result")
    
   parser.add_option("-z", "--zip",
                     action = "store_true",
                     help = "gzip postscript output file")

   parser.add_option("-p", "--pdf",
                     action = "store_true",
                     help = "create pdf instead of ps output file")   

   parser.add_option("-v", "--verbose",
                     action = "store_true", default = False,
                     help = "be verbose")

   parser.add_option("--html",action="store_true",default=False,help="generate root html code to view results in web browser")

   (opts, args) = parser.parse_args()
   
   if len(args)!=2:
      parser.print_help()
      return 255

   if not opts.noSkipList:
      opts.skip += ["Unpck$", "BufFreeCnt$", "CalEvtSize$"]     # muon calibration buffer
      opts.skip += ["/TIME_"]                          # timers from Monitored framework
      opts.skip += ["/athenaHLT.*/.*Time$"]            # HLTMPPU timing histograms
      opts.skip += ["HltEventLoopMgr/.*Time.*"]        # custom timers
      opts.skip += ["HltEventLoopMgr/PopScheduler.*"]  # scheduler monitoring
      opts.skip += ["MessageSvc/MessageCount"]         # MessageSvc
      opts.skip += ["TrigSignatureMoni/.*Rate"]        # Rate monitoring
      opts.skip += ["TrigOpMonitor/GeneralOpInfo"]     # release number, etc.
      opts.skip += ["TrigOpMonitor/IOVDb.*"]           # conditions data IOVs, size and time
      opts.skip += ["TrigOpMonitor/.*ReadTime"]        # conditions read time
      opts.skip += ["TrigOpMonitor/.*BytesRead"]       # conditions read size
      opts.skip += ["HLTFramework/ROBDataProviderSvc"] # RDP histograms differ in MT due to caching
      opts.skip += ["HLTFramework/SchedulerMonSvc"]    # ATR-22345, not reproducible by definition
      opts.skip += ["HLTSeeding/Random"]               # Prescale validation (ATR-21935)


   # Default thresholds
   if not opts.threshold:
      if opts.chi2: opts.threshold = 0.95
      else: opts.threshold = 1e-6
      
   print("-"*70)
   print("Command    : rootcomp.py %s\n" % (" ".join(sys.argv[1:])))
   print("Reference  : %s" % args[0])
   print("File       : %s" % args[1])
   print("Comparison : ", end="")
   if opts.chi2: print("CHI2 (%.2f)" % opts.threshold)
   elif opts.axis: print("AXIS")
   else: print("BIN-BY-BIN (%.1e)" % opts.threshold)
   if not opts.skip==[]: print("Ignored histograms: %s" % (", ".join(opts.skip)))
   if not opts.select==[]: print("Selected histograms: %s" % (", ".join(opts.select)))

   print("-"*70)


   # Now import ROOT
   import cppyy  # noqa: F401
   try:
      from PerfMonAna import PyRootLib
      ROOT = PyRootLib.importRoot( batch=True )
   except ImportError:
      import ROOT   # noqa: F401

   sys.stdout.flush()
   sys.stderr.flush()
   
   from ROOT import TRootCompare
   from ROOT import gROOT, gStyle
   gROOT.SetStyle("Plain")
   gStyle.SetOptStat(111111)

   valid = TRootCompare()
   valid.setVerbose(opts.verbose)

   # Set algorithm
   if opts.chi2:
      valid.setAlg(TRootCompare.CHI2, opts.threshold)
   elif opts.axis:
      valid.setAlg(TRootCompare.AXIS, opts.threshold)
   else:
      valid.setAlg(TRootCompare.BIN, opts.threshold)

   # Select these histograms
   for s in opts.select:
      valid.passBeforeFailRegexp()
      valid.addPassRegexp(s)
      
   # Skip these histograms
   for s in opts.skip:
      valid.addFailRegexp(s)
  
   # Select output (root, ps)
   if not opts.noRoot: valid.setOutputFile(opts.outFile+".root")

   if opts.noPS:
      valid.setPsFile("")
   elif opts.pdf:
      valid.setPsFile(opts.outFile+".pdf")
   else:
      valid.setPsFile(opts.outFile+".ps")

   valid.sortLabels(opts.sortLabels)
   valid.drawNormalized(opts.norm)
   valid.drawDiff(not opts.noDiff)
   
   # Run
   rc = valid.setReferenceFile(args[0],opts.refBaseDir)
   if rc is False:
      return 255

   rc = valid.run(args[1],opts.fileBaseDir)

   sys.stdout.flush()
   sys.stderr.flush()
   
   if opts.zip and not opts.pdf:
      print("GZipping postscript file -> %s.ps.gz" % opts.outFile)
      os.system("gzip -f %s.ps" % (opts.outFile))

   if rc != 0:
      result = 255
   elif valid.totalHist()>0:
      # Return 0 if all histograms are matching
      if opts.ignoreMissingRef: result = min(valid.totalHist()-valid.matchingHist(),255)
      # Return 0 if all histograms are matching and none missing
      else: result = min(valid.totalHist()-valid.matchingHist()-valid.missingHist(),255)
   elif valid.totalHist()==0 and valid.missingHist()==0 and valid.matchingHist()==0:
      # Return 0 if no histograms found
      result = 0
   else:
      result  = 255

   if opts.html:
      os.system("root2html.py *.root")

   print("Overall test result: %i" % result)
   return result

if __name__ == "__main__":
   sys.exit(main())
