from array import array
import ROOT as rt
import yoda

fName = 'MyOutput.yoda.gz'
yodaAOs = yoda.read(fName)
rtFile = rt.TFile(fName[:fName.find('.yoda')] + '.root', 'recreate')

for name_slashes in yodaAOs:
  name = name_slashes.replace("/", "_")
  yodaAO = yodaAOs[name_slashes]  
  rtAO = None

  if 'Histo1D' in str(yodaAO):
    rtAO = rt.TH1D(name, '', yodaAO.numBins(), array('d', yodaAO.xEdges()))
  

    rtAO.Sumw2()
    rtErrs = rtAO.GetSumw2()
    
    for i in range(rtAO.GetNbinsX()):
      
      rtAO.SetBinContent(i + 1, yodaAO.bin(i).sumW())
      rtErrs.AddAt(yodaAO.bin(i).sumW2(), i+1)
      
  elif 'Scatter2D' in str(yodaAO):
    rtAO = rt.TGraphAsymmErrors(yodaAO.numPoints())
    rtAO.SetName(name)
    
    for i in range(yodaAO.numPoints()):
      x = yodaAO.point(i).x(); y = yodaAO.point(i).y()
      xLo, xHi = yodaAO.point(i).xErrs()
      yLo, yHi = yodaAO.point(i).yErrs()
      rtAO.SetPoint(i, x, y)
      rtAO.SetPointError(i, xLo, xHi, yLo, yHi)
  else:
    continue

  rtAO.Write(name)

rtFile.Close()
