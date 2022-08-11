# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python
from array import array
import ROOT, argparse,  sys, gc, os, math
from PlotUtils import PlotUtils, DiagnosticHisto
 


class ReleaseComparer(object):
    def __init__(self,
                 var_name = "", axis_title ="",
                 bins = -1, bmin = 0., bmax = 0., bin_width = -1,
                 bdir = None,
                       
                 name_old_rel ="", name_new_rel ="",
                 
                 test_tree = None,
                 branch_old = "",   branch_new = "",
                 weight_old = None, weight_new = None,
                 log_binning = False
                 ):
        ### Direct access to the branch which are going to be compared
        self.__old_branch = test_tree.GetLeaf(branch_old)
        self.__new_branch = test_tree.GetLeaf(branch_new)
        if not self.__old_branch:
            raise NameError("Could not find "+branch_old+" in the Tree")
            
        if not self.__new_branch:
            raise NameError("Could not find "+branch_new+" in the Tree")
        ### Weights as a function of the muon kinematics
        self.__old_weight = 1. if not weight_old else test_tree.GetLeaf(weight_old)
        self.__new_weight = 1. if not weight_new else test_tree.GetLeaf(weight_new)
        if weight_old and not self.__old_weight:
            raise NameError("Could not find "+weight_old+" in the Tree")
        if weight_new and not self.__new_weight:
            raise NameError("Could not find "+weight_new+" in the Tree")
            
        self.__quality_branch = test_tree.GetLeaf("Muon_quality")
        if branch_old.find("HighPt") != -1 : self.__quality_branch = test_tree.GetLeaf("Muon_isHighPt")
        if branch_old.find("LowPt")  != -1 : self.__quality_branch = test_tree.GetLeaf("Muon_isLowPt")
        
        self.__min_quality = 0 if branch_old.find("HighPt") != -1 and branch_old.find("LowPt") != -1 else 1
        self.__max_quality = 2 
        if branch_old.find("Medium") != -1:  self.__max_quality = 1
        if branch_old.find("Tight") != -1:   self.__max_quality = 0
        
        
        self.__var_name = var_name
        self.__old_histo = DiagnosticHisto(
                                    name = "%s_%s"%(name_old_rel, var_name),
                                    axis_title = axis_title,
                                    bins = bins, bmin = bmin, bmax = bmax, 
                                    bin_width = bin_width, bdir = bdir, log_binning = log_binning)
        self.__new_histo = DiagnosticHisto(
                                    name = "%s_%s"%(name_new_rel, var_name),
                                    axis_title = axis_title,
                                    bins = bins, bmin = bmin, bmax = bmax, 
                                    bin_width = bin_width, bdir = bdir, log_binning = log_binning)
                
    def pass_cut(self):
        return self.__min_quality >= self.__quality_branch .GetValue() and self.__quality_branch.GetValue() <= self.__max_quality
    def fill(self):
        if not self.pass_cut(): return
        self.__old_histo.fill(value = self.__old_branch.GetValue(), weight= self.get_old_weight())
        self.__new_histo.fill(value = self.__new_branch.GetValue(), weight= self.get_new_weight())
    def get_old_histo(self): 
        return self.__old_histo
    def get_new_histo(self): 
        return self.__new_histo
    def get_old_var(self): 
        return self.__old_branch
    def get_new_var(self): 
        return self.__new_branch
    def get_old_weight(self):
        if isinstance(self.__old_weight, float): return self.__old_weight
        return self.__old_weight.GetValue()
    def get_new_weight(self): 
        if isinstance(self.__new_weight, float): return self.__new_weight
        return self.__new_weight.GetValue()
    def finalize(self):
        minimum = min(self.get_old_histo().min(), self.get_new_histo().min()) - 0.01
        maximum = max(self.get_old_histo().max(), self.get_new_histo().max()) + 0.01
        for H in [ self.get_old_histo(), self.get_new_histo()]:
            H.setMinimum(minimum)
            H.setMaximum(maximum)
            H.write()
               
    def name(self):
        return self.__var_name
class SystematicComparer(ReleaseComparer):
     def __init__(self,
                 var_name = "",
                 axis_title ="",
                 bins = -1,
                       
                 name_old_rel ="",
                 name_new_rel ="",
                 
                 test_tree = None,
                 branch_old = "",
                 branch_new = "",
                 weight_old = None,
                 weight_new = None,
                 
                 branch_sys_old = "",
                 branch_sys_new = "",
                 ):
        ReleaseComparer.__init__(self,
                                 var_name = var_name,
                                 axis_title = axis_title,
                                 bins = bins, bmin = 5.e-4, bmax = 4.,
                                 name_old_rel =name_old_rel, 
                                 name_new_rel =name_new_rel,
                                 test_tree = test_tree,
                                 branch_old = branch_old,
                                 branch_new = branch_new,
                                 weight_old = weight_old,
                                 weight_new = weight_new,
                                 log_binning = True)
        self.__sys_old = test_tree.GetLeaf(branch_sys_old)
        self.__sys_new = test_tree.GetLeaf(branch_sys_new)
        if not self.__sys_old:
            raise NameError("Failed to retrieve "+branch_sys_old)
        if not self.__sys_new:
            raise NameError("Failed to retrieve "+branch_sys_new)
        
        
     def fill(self):
        self.get_old_histo().fill(value = math.fabs(self.get_old_var().GetValue() - self.__sys_old.GetValue()) / (self.get_old_var().GetValue() if self.get_old_var().GetValue() != 0. else 1.) , 
                                  weight= self.get_old_weight())
        self.get_new_histo().fill(value = math.fabs(self.get_new_var().GetValue() - self.__sys_new.GetValue()) / (self.get_new_var().GetValue() if self.get_new_var().GetValue() != 0. else 1.), 
                                  weight= self.get_new_weight())
        
        

KnownWPs = {
    "Loose" : "RECO",
    "Medium" : "RECO",
    "Tight" : "RECO",
    "HightPt3Layers":"RECO",
    "HighPt" : "RECO",
    "LowPt" : "RECO",
    "LowPtMVA" : "RECO",
    
    "TTVA" : "TTVA",
    "FCLooseIso": "ISO",                    
    "FCTight_FixedRadIso": "ISO",
    "FCLoose_FixedRadIso": "ISO",           
    "FixedCutHighPtTrackOnlyIso": "ISO",
    "FCTightIso": "ISO",                    
    "FixedCutPflowLooseIso": "ISO",
    "FCTightTrackOnlyIso": "ISO",           
    "FixedCutPflowTightIso": "ISO",
    "FCTightTrackOnly_FixedRadIso": "ISO",
    "BadMuonVeto_HighPt" : "BADMUON",
    }

def getArgParser():
    parser = argparse.ArgumentParser(description='This script checks applied scale factors written to a file by MuonEfficiencyCorrections/MuonEfficiencyCorrectionsSFFilesTest. For more help type \"python CheckAppliedSFs.py -h\"', prog='CheckAppliedSFs', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-i', '--InputFile', help='Specify an input root file', default="SFTest.root")
    parser.add_argument('-o', '--outDir', help='Specify a destination directory', default="Plots")
    parser.add_argument('-l', '--label', help='Specify the dataset you used with MuonEfficiencyCorrectionsSFFilesTest', default="Internal")
    parser.add_argument('-w', '--WP', help='Specify a WP to plot', nargs='+', default=[])
    parser.add_argument('-c', '--SFConstituent', help='Specify if you want to plot nominal value, sys or stat error', nargs='+', default=["SF","DataEff","MCEff"])
    parser.add_argument('--bonusname', help='Specify a bonus name for the filename', default="")
    parser.add_argument('--bonuslabel', help='Specify a bonus label printed in the histogram', default="")
    parser.add_argument('--noComparison', help='do not plot comparison to old release', action='store_true', default=False)
    parser.add_argument('-n', '--nBins', help='specify number of bins for histograms', type=int, default=50)
    return parser

def getCalibReleasesAndWP(tree):
    branchesInFile = [key.GetName() for key in tree.GetListOfBranches()]
    calibReleases = []
    allWPs = set([wp for wp in KnownWPs.iterkeys() ])
    WPs = []
    for i in branchesInFile:
        print i
        if not i.endswith("SF"): continue
        if not i.startswith("c"): continue
        calibCand = i[1:-3]
        
        wp_str = i[ : i.rfind("_")]
        beststr = wp_str[wp_str.rfind("_")+1 : ]
        if beststr in allWPs:
            if not beststr in WPs: WPs.append(beststr)
            if not calibCand[ : calibCand.find(beststr)-1] in calibReleases: calibReleases.append(calibCand[ : calibCand.find(beststr)-1])
    print "INFO: Found the following working points: %s"%(", ".join(WPs))
    return calibReleases, WPs

def getSystematics(tree, wp, calib_release):
    search_str = "c%s_%s_SF"%(calib_release, wp)
    syst_names = [key.GetName()[len(search_str) + 2:] for key in tree.GetListOfBranches() if key.GetName().startswith(search_str) and key.GetName() != search_str]
    print syst_names
    return syst_names

if __name__ == "__main__":    
    Options = getArgParser().parse_args()

    if not os.path.exists(Options.InputFile):
        print 'ERROR: File %s does not exist!'%Options.InputFile
        sys.exit(1)
    infile  = ROOT.TFile(Options.InputFile)
    
    tree = infile.Get("MuonEfficiencyTest")
    calibReleases, WPs = getCalibReleasesAndWP(tree)
    
    if len(calibReleases)==2: print "INFO: Found the following calibration releases to compare: %s"%(",".join(calibReleases))
    
    if len(Options.WP)>0:
        userWPs = []
        for wp in Options.WP:
            if wp in WPs: userWPs.append(wp)
        WPs = userWPs
        print 'INFO: WPs given by user, only plot: %s'%(",".join(WPs))

        
    ROOT.gROOT.Macro("rootlogon.C")
    ROOT.gROOT.SetStyle("ATLAS")
    ROOT.gROOT.SetBatch(1)
    gc.disable()
    
    if os.path.isdir(Options.outDir) == False:
        os.system("mkdir -p %s"%(Options.outDir))
    
    bonusname=Options.bonusname

    Histos = []
    
    for wp in WPs:
        systematics = getSystematics(tree,wp, calibReleases[0])+[""]
        for t in Options.SFConstituent:
            corrType = "Scale Factor"
            if t == "DataEff": corrType = "Data efficiency"
            elif t == "MCEff": corrType = "MC efficiency"
                
            for var in systematics:
                if len(var) == 0:
                    Histos += [
                    ReleaseComparer(
                                var_name = "%s_%s"%(wp,t), axis_title = " %s %s"%(corrType,wp),
                                bmin = 1., bin_width = 0.0001,
                                name_old_rel =calibReleases[0], name_new_rel = calibReleases[1],
                                test_tree = tree,
                                branch_old = "c%s_%s_%s"%(calibReleases[0],wp,t),   branch_new = "c%s_%s_%s"%(calibReleases[1],wp,t),
                        )]
                else:
                   Histos +=[
                   SystematicComparer(
                             var_name = "%s_%s_%s"%(wp,t,var.replace("RECO",KnownWPs[wp])), 
                             axis_title = "%s %s (%s)"%(var.replace("RECO",KnownWPs[wp]), corrType,wp),
                             bins = Options.nBins,
                             name_old_rel =calibReleases[0], name_new_rel = calibReleases[1],
                             test_tree = tree,
                             branch_old = "c%s_%s_%s"%(calibReleases[0],wp,t),   branch_new = "c%s_%s_%s"%(calibReleases[1],wp,t),
                      
                            branch_sys_old = "c%s_%s_%s__%s"%(calibReleases[0],wp,t,var),
                            branch_sys_new = "c%s_%s_%s__%s"%(calibReleases[1],wp,t,var),
                        )] 
  
            continue
            Histos +=[
                ReleaseComparer(var_name = "pt_%s"%(wp), axis_title ="p_{T} #mu(%s) [MeV]"%(wp),
                            bins = 15, bmin = 15.e3, bmax = 200.e3,
                            name_old_rel =calibReleases[0], name_new_rel = calibReleases[1],
                            test_tree = tree,
                            branch_old = "Muon_pt",   branch_new = "Muon_pt",
                            weight_old = "c%s_%s_SF"%(calibReleases[0],wp),   
                            weight_new = "c%s_%s_SF"%(calibReleases[1],wp)),
                            
                ReleaseComparer(var_name = "eta_%s"%(wp), axis_title ="#eta #mu(%s) [MeV]"%(wp),
                            bins = 54, bmin = -2.7, bmax = 2.7,
                            name_old_rel =calibReleases[0], name_new_rel = calibReleases[1],
                            test_tree = tree,
                            branch_old = "Muon_eta",   branch_new = "Muon_eta",
                            weight_old = "c%s_%s_SF"%(calibReleases[0],wp),   
                            weight_new = "c%s_%s_SF"%(calibReleases[1],wp)),
                ReleaseComparer(var_name = "phi_%s"%(wp), axis_title ="#phi #mu(%s) [MeV]"%(wp),
                            bins = 16, bmin = -3.15, bmax = 3.15,
                            name_old_rel =calibReleases[0], name_new_rel = calibReleases[1],
                            test_tree = tree,
                            branch_old = "Muon_phi",   branch_new = "Muon_phi",
                            weight_old = "c%s_%s_SF"%(calibReleases[0],wp),   
                            weight_new = "c%s_%s_SF"%(calibReleases[1],wp)),
                
            ]
    
    for i in range(tree.GetEntries()):
        tree.GetEntry(i)
        if i > 0 and i % 2500 == 0: 
            print "INFO: %d/%d events processed"%(i, tree.GetEntries())
            
        if  math.fabs(tree.Muon_eta) > 2.5  or math.fabs(tree.Muon_eta) < 0.1 or math.fabs(tree.Muon_pt) > 15.e3: continue        
        for H in Histos: 
            H.fill()
        
    print "INFO: Histograms filled"
   
    
    
    if len(calibReleases)==2:

        dummy = ROOT.TCanvas("dummy", "dummy", 800, 600)
        dummy.SaveAs("%s/AllAppliedSFCheckPlots%s.pdf[" % (Options.outDir, bonusname))
       
        for comp in Histos:
            comp.finalize()
            histoCR1 = comp.get_old_histo().TH1()
            histoCR2 = comp.get_new_histo().TH1()
            
            pu = PlotUtils(status = Options.label)
            pu.Prepare1PadCanvas(comp.name())
            pu.GetCanvas().SetLogy()
            if comp.get_old_histo().has_log_binnnig(): pu.GetCanvas().SetLogx()
            
            histoCR1.GetYaxis().SetTitle("Fraction of muons")
         
            pu.drawStyling(histoCR1, 1, max([histoCR1.GetMaximum(),histoCR2.GetMaximum()]) *1.e3, TopPad = False)
            histoCR1.SetTitle("%s, Mean: %.8f"%(calibReleases[0],histoCR1.GetMean()))
            histoCR2.SetTitle("%s, Mean: %.8f"%(calibReleases[1],histoCR2.GetMean()))
                        
            histoCR1.Draw("sameHIST")
            histoCR2.SetLineColor(ROOT.kRed)
            histoCR2.SetMarkerColor(ROOT.kRed)
            histoCR2.SetLineStyle(9)
            histoCR2.Draw("sameHIST")
            pu.CreateLegend(0.2, 0.7, 0.6, 0.8,18)
            
            pu.AddToLegend([histoCR1, histoCR2])
           
            variationDrawn = "Nominal"
            if "STAT" in comp.name(): variationDrawn = "|Stat-Nominal|/Nominal"
            elif "SYS" in comp.name(): variationDrawn = "|Sys-Nominal|/Nominal"
            type_drawn = "Scale factor"
            if comp.name().split("_")[1] == "DataEff": type_drawn = "data efficiency"
            elif comp.name().split("_")[1] == "MCEff": type_drawn = "MC efficiency"
            pu.DrawTLatex(0.55, 0.5, Options.bonuslabel)
            pu.DrawTLatex(0.55, 0.85, "WP: %s, %s"%(comp.name().split("_")[0],variationDrawn))
            pu.DrawTLatex(0.55, 0.9, type_drawn)
            
            pu.DrawAtlas(0.2, 0.9)
            pu.DrawSqrtS(0.2, 0.85)
            
            pu.DrawLegend()
            pu.saveHisto("%s/AppliedSFCheck_%s%s"%(Options.outDir, comp.name(),bonusname), ["pdf"])
            pu.saveHisto("%s/AllAppliedSFCheckPlots%s" % (Options.outDir, bonusname), ["pdf"])

        dummy.SaveAs("%s/AllAppliedSFCheckPlots%s.pdf]" % (Options.outDir, bonusname))
        
    else:
        print "INFO: Currently, only release comaparisons are implemented"
       
