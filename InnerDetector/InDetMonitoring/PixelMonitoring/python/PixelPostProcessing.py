#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

import ROOT
import math 
import importlib.resources
from PixelMonitoring.PixelAthMonitoringBase import LabelX, LabelY, baselayers, xbinsl

LB_deg = ROOT.TH1F('TotalDegradationPerLumi', 'b-tag degradation;LB;total b-tag degradation', 3000, -0.5, 2999.5)
degFactor70 = [0.0032, 0.0078, 0.011, 0.020, 0.023, 0.018, 0.098, 0.10, 0.26, 0.36, 0.33, 0.17, 0.65, 0.79, 0.81]

def normalize_perEvent(inputs):
    layer = inputs[0][0]['sec'] 
    nEventLB = inputs[0][1][1]
    nAllEvents = nEventLB.Integral(1, nEventLB.GetNbinsX()+1)
 
    histoName = inputs[0][1][0].GetName()
    histo = inputs[0][1][0].Clone()
    errorName = histoName.split('_')[0]
    histo.SetName(errorName + '_Norm_' + layer)
    histoTitle = histo.GetTitle().split(",")[0]
    if nAllEvents != 0:
        histo.Scale(1.0/nAllEvents)
        histo.SetTitle(histoTitle + " per event, " + layer)
    return [histo]

def badEtaPhi_forAllMaskPatterns(inputs):
    Th = 0.5
    LB = inputs[0][0]['LB']
    rv = []
    rv1 = []
    rv_IBL = ROOT.TH2F() 
    rv_BLayer = ROOT.TH2F() 
    rv_Layer1 = ROOT.TH2F() 
    rv_Layer2 = ROOT.TH2F() 
    totalDeg = 0.0
    for i in range(len(inputs[0][1])):
        plots = [_[1][i] for _ in inputs] # all plots passed as first element of list
        for m, plot in enumerate(plots):
            etaMin = []
            etaMax = []
            phiMin = []
            phiMax = []
            sec = inputs[m][0]['sec']
            rv.append(ROOT.TH2F('defectPlot', 'badFERegion', 500, -3.0, 3.0, 500, -math.pi, math.pi))
            rv[m].SetTitle('badFE_EtaPhi_' + sec)
            rv[m].GetXaxis().SetTitle('#eta')
            rv[m].GetYaxis().SetTitle('#phi')
            with importlib.resources.open_text('PixelMonitoring', 'FE_EtaEdge_' + sec + '.txt') as etaInfo: 
                for line in etaInfo.readlines():
                    toks = line.split()
                    etaMin.append(float(toks[3]))
                    etaMax.append(float(toks[4]))           
            with importlib.resources.open_text('PixelMonitoring', 'FE_PhiEdge_' + sec + '.txt') as phiInfo: 
                for line in phiInfo.readlines():
                    toks = line.split()
                    if float(toks[3]) < -math.pi:
                        phiMin.append(float(toks[3]) + 2*math.pi)
                    else:
                        phiMin.append(float(toks[3]))
                    if float(toks[4]) > math.pi:
                        phiMax.append(float(toks[4]) - 2*math.pi)
                    else: 
                        phiMax.append(float(toks[4]))
            for xbin in range(plot.GetNbinsX()):
                for ybin in range(plot.GetNbinsY()):
                    if(plot.GetBinContent(xbin+1, ybin+1) < Th):
                        continue
                    etaMin_bin = rv[m].GetXaxis().FindBin(etaMin[xbin] + 3.0/500)
                    etaMax_bin = rv[m].GetXaxis().FindBin(etaMax[xbin] + 3.0/500)
                    phiMin_bin = rv[m].GetYaxis().FindBin(phiMin[ybin] + math.pi/500)
                    phiMax_bin = rv[m].GetYaxis().FindBin(phiMax[ybin] + math.pi/500)
                    for eta_bin in range(etaMin_bin, etaMax_bin):
                        eta = rv[m].GetXaxis().GetBinCenter(eta_bin)
                        for phi_bin in range(phiMin_bin, phiMax_bin):
                            phi = rv[m].GetYaxis().GetBinCenter(phi_bin)
                            rv[m].Fill(eta, phi)
                        if phiMin[ybin] > phiMax[ybin]:
                            for phi_bin in range(1, phiMax_bin):
                                phi = rv[m].GetYaxis().GetBinCenter(phi_bin)
                                rv[m].Fill(eta, phi)
                            for phi_bin in range(phiMin_bin, rv[m].GetNbinsY()+1):
                                phi = rv[m].GetYaxis().GetBinCenter(phi_bin)
                                rv[m].Fill(eta, phi)

            if sec == 'IBL':
                rv_IBL = rv[m].Clone()
                rv_IBL.SetName('badFE_EtaPhi_IBL_new')
            elif sec == 'BLayer':
                rv_BLayer = rv[m].Clone()
                rv_BLayer.SetName('badFE_EtaPhi_BLayer_new')
            elif sec == 'Layer1':
                rv_Layer1 = rv[m].Clone()
                rv_Layer1.SetName('badFE_EtaPhi_Layer1_new')
            elif sec == 'Layer2':
                rv_Layer2 = rv[m].Clone()
                rv_Layer2.SetName('badFE_EtaPhi_Layer2_new')
 
        for m in range(0, 15):
            rv1.append(ROOT.TH2F('defectPlot', 'badFEOverlaps', 500, -3.0, 3.0, 500, -math.pi, math.pi))
            rv1[m].GetXaxis().SetTitle('#eta')
            rv1[m].GetYaxis().SetTitle('#phi')
        for xbin in range(rv[0].GetNbinsX()):
            eta = rv[0].GetXaxis().GetBinCenter(xbin+1)
            for ybin in range(rv[0].GetNbinsY()):
                phi = rv[0].GetYaxis().GetBinCenter(ybin+1)
                entIBL = rv_IBL.GetBinContent(xbin+1, ybin+1)
                entBLayer = rv_BLayer.GetBinContent(xbin+1, ybin+1)
                entLayer1 = rv_Layer1.GetBinContent(xbin+1, ybin+1)
                entLayer2 = rv_Layer2.GetBinContent(xbin+1, ybin+1)
                if entIBL >= 1 and entBLayer >= 1 and entLayer1 >= 1 and entLayer2 >= 1: # IBL, B-Layer, Layer1, Layer2
                    rv1[0].SetTitle('badFE_EtaPhi_IBL_BLayer_Layer1_Layer2')
                    rv1[0].Fill(eta, phi) 
                elif entIBL >= 1 and entBLayer >= 1 and entLayer1 >= 1: # IBL, B-Layer, Layer1
                    rv1[1].SetTitle('badFE_EtaPhi_IBL_BLayer_Layer1')
                    rv1[1].Fill(eta, phi) 
                elif entIBL >= 1 and entBLayer >= 1 and entLayer2 >= 1: # IBL, B-Layer, Layer2
                    rv1[2].SetTitle('badFE_EtaPhi_IBL_BLayer_Layer2')
                    rv1[2].Fill(eta, phi) 
                elif entIBL >= 1 and entLayer1 >= 1 and entLayer2 >= 1: # IBL, Layer2, Layer2
                    rv1[3].SetTitle('badFE_EtaPhi_IBL_Layer1_Layer2')
                    rv1[3].Fill(eta, phi) 
                elif entBLayer >= 1 and entLayer1 >= 1 and entLayer2 >= 1: # B-Layer, Layer2, Layer2
                    rv1[4].SetTitle('badFE_EtaPhi_BLayer_Layer1_Layer2')
                    rv1[4].Fill(eta, phi) 
                elif entIBL >= 1 and entBLayer >= 1: # IBL, B-Layer
                    rv1[5].SetTitle('badFE_EtaPhi_IBL_BLayer')
                    rv1[5].Fill(eta, phi) 
                elif entIBL >= 1 and entLayer1 >= 1: # IBL, Layer1
                    rv1[6].SetTitle('badFE_EtaPhi_IBL_BLayer')
                    rv1[6].Fill(eta, phi) 
                elif entIBL >= 1 and entLayer2 >= 1: # IBL, Layer2
                    rv1[7].SetTitle('badFE_EtaPhi_IBL_BLayer')
                    rv1[7].Fill(eta, phi) 
                elif entBLayer >= 1 and entLayer1 >= 1: # B-Layer, Layer1
                    rv1[8].SetTitle('badFE_EtaPhi_BLayer_Layer1')
                    rv1[8].Fill(eta, phi) 
                elif entBLayer >= 1 and entLayer2 >= 1: # B-Layer, Layer2
                    rv1[9].SetTitle('badFE_EtaPhi_BLayer_Layer2')
                    rv1[9].Fill(eta, phi) 
                elif entLayer1 >= 1 and entLayer2 >= 1: # Layer1, Layer2
                    rv1[10].SetTitle('badFE_EtaPhi_Layer1_Layer2')
                    rv1[10].Fill(eta, phi) 
                elif entIBL >= 1: # IBL
                    rv1[11].SetTitle('badFE_EtaPhi_onlyIBL')
                    rv1[11].Fill(eta, phi) 
                elif entBLayer >= 1: # B-Layer
                    rv1[12].SetTitle('badFE_EtaPhi_onlyBLayer')
                    rv1[12].Fill(eta, phi) 
                elif entLayer1 >= 1: # Layer1
                    rv1[13].SetTitle('badFE_EtaPhi_onlyLayer1')
                    rv1[13].Fill(eta, phi) 
                elif entLayer2 >= 1: # Layer2
                    rv1[14].SetTitle('badFE_EtaPhi_onlyLayer2')
                    rv1[14].Fill(eta, phi) 

        for m in range(0, 15):
            nBadRegion = rv1[m].Integral()
            badFrac = nBadRegion/(500*500)
            deg = (1.0-degFactor70[m])*badFrac
            totalDeg = totalDeg + deg

    a = LB.split('_')
    LB_deg.Fill(int(a[1]), totalDeg) 
    binNum = LB_deg.FindBin(int(a[1]))
    LB_deg.SetBinError(binNum, 0) 
    return [rv_IBL, rv_BLayer, rv_Layer1, rv_Layer2, rv1[0], rv1[1], rv1[2], rv1[3], rv1[4], rv1[5], rv1[6], rv1[7], rv1[8], rv1[9], rv1[10], rv1[11], rv1[12], rv1[13], rv1[14], LB_deg] 

####################################################################

def evaluateModuleHistograms(inputs, minBinStat=5, mvaThr=0.5, excludeOutOfAcc=True):
    layer   = inputs[0][0]['layer']
    ohisto  = inputs[0][1][1].Clone()
    ohisto.Reset()
    i_layer = baselayers.index(layer)

    histos = [_[1][0] for _ in inputs]

    for ih, histo in enumerate(histos):
        #
        # collect info from module's past behaviour
        #
        nInpBins = histo.GetNbinsX()
        stat = 0
        cont = 0
        for inputbin in range(1, nInpBins + 1):
            stat += histo.GetBinEntries(inputbin)
            cont += histo.GetBinContent(inputbin)*histo.GetBinEntries(inputbin)
        #
        # from module name get binx, biny of output histo
        #
        splits = histo.GetName().split('_')
        if (layer in ['BLayer','Layer1','Layer2']):
            x = splits[3]
            y = splits[1] + '_' + splits[2]
        elif layer=='IBL':
            #S0_M1A -> A1_0
            x = splits[3][2] + splits[3][1] + '_' + splits[2][1]
            #B14 -> #S14
            y = 'S'+splits[1][1:]
        else: 
            # D1 -> Disk 1
            x = 'Disk ' + splits[0][1]
            # remove 'A' or 'C'
            y = splits[1] + '_' + splits[2] + '_' + splits[3][:-1]
        i_x = LabelX[i_layer].index(x)+1
        i_y = LabelY[i_layer].index(y)+1
        i_bin = i_y*(xbinsl[i_layer]+2) + i_x
 
        # assessment
        if (i_x<5 or i_x>ohisto.GetNbinsX()-4) and layer=='IBL' and excludeOutOfAcc :
            ohisto.SetBinContent(i_x,i_y,0)
            ohisto.SetBinEntries(i_bin,1) #OK (out of acceptance)
        elif (i_x==1 or i_x==ohisto.GetNbinsX()) and layer=='BLayer' and excludeOutOfAcc :
            ohisto.SetBinContent(i_x,i_y,0)
            ohisto.SetBinEntries(i_bin,1) #OK (out of acceptance)
        else:
            if stat>minBinStat:
                if cont/stat>mvaThr: #not OK
                    ohisto.SetBinContent(i_x,i_y,1.0)
                    ohisto.SetBinEntries(i_bin,1)
                else: #OK
                    ohisto.SetBinContent(i_x,i_y,0)
                    ohisto.SetBinEntries(i_bin,1)
            else: #not enough info - empty
                ohisto.SetBinContent(i_x,i_y,0)
                ohisto.SetBinEntries(i_bin,0)

    ## known exception
    if layer=='IBL':
        ohisto.SetBinContent(11,13,0) # FE S13-C3-M0 - OK
        ohisto.SetBinEntries(ohisto.GetBin(11,13),1)
    ##
    ohisto.SetName('FixMe_'+str(layer))
    if 'IBL' in layer:
        ohisto.SetTitle('Front-Ends to fix, '+str(layer))
    else:
        ohisto.SetTitle('Modules to fix, '+str(layer))
    ohisto.SetOption("colztext")
    return [ohisto]
####################################################################
