# Warnings:
# UserHooks Warning: the calculation of uncertainty variations will only be consistent in the absence of any external modifications to the shower branching probabilities via the UserHooks framework. It is therefore strongly advised to avoid combining the automated uncertainty calculations with any such UserHooks modifications.
#
#Merging Warning: in multi-jet merging approaches, trial showers are used to generate missing Sudakov factor corrections to the hard matrix elements. Currently that framework is not consistently combined with the variations introduced here, so the two should not be used simultaneously. This means shower weights should NOT be used with CKKLW or FxFx merging!
#
#
# Pythia8 shower weights are only available in release 2.26 and later.  The
# test below checks the Pythia8 release and also verifies that the Pythia_i
# tag is recent enought to store the shower weights.

if "ShowerWeightNames" in genSeq.Pythia8.__slots__.keys():
      print ("Initalizing Shower Weights from Pythia8_ShowerWeights.py")
      if("NNPDF" in evgenConfig.tune):
         genSeq.Pythia8.Commands += [
                            'UncertaintyBands:doVariations = on', 
                            'UncertaintyBands:List = {\
                             Var3cUp isr:muRfac=0.549241,\
                             Var3cDown isr:muRfac=1.960832,\
                             isr:muRfac=2.0_fsr:muRfac=2.0 isr:muRfac=2.0 fsr:muRfac=2.0,\
                             isr:muRfac=2.0_fsr:muRfac=1.0 isr:muRfac=2.0 fsr:muRfac=1.0,\
                             isr:muRfac=2.0_fsr:muRfac=0.5 isr:muRfac=2.0 fsr:muRfac=0.5,\
                             isr:muRfac=1.0_fsr:muRfac=2.0 isr:muRfac=1.0 fsr:muRfac=2.0,\
                             isr:muRfac=1.0_fsr:muRfac=0.5 isr:muRfac=1.0 fsr:muRfac=0.5,\
                             isr:muRfac=0.5_fsr:muRfac=2.0 isr:muRfac=0.5 fsr:muRfac=2.0,\
                             isr:muRfac=0.5_fsr:muRfac=1.0 isr:muRfac=0.5 fsr:muRfac=1.0,\
                             isr:muRfac=0.5_fsr:muRfac=0.5 isr:muRfac=0.5 fsr:muRfac=0.5,\
                             isr:muRfac=1.75_fsr:muRfac=1.0 isr:muRfac=1.75 fsr:muRfac=1.0,\
                             isr:muRfac=1.5_fsr:muRfac=1.0 isr:muRfac=1.5 fsr:muRfac=1.0,\
                             isr:muRfac=1.25_fsr:muRfac=1.0 isr:muRfac=1.25 fsr:muRfac=1.0,\
                             isr:muRfac=0.625_fsr:muRfac=1.0 isr:muRfac=0.625 fsr:muRfac=1.0,\
                             isr:muRfac=0.75_fsr:muRfac=1.0 isr:muRfac=0.75 fsr:muRfac=1.0,\
                             isr:muRfac=0.875_fsr:muRfac=1.0 isr:muRfac=0.875 fsr:muRfac=1.0,\
                             isr:muRfac=1.0_fsr:muRfac=1.75 isr:muRfac=1.0 fsr:muRfac=1.75,\
                             isr:muRfac=1.0_fsr:muRfac=1.5 isr:muRfac=1.0 fsr:muRfac=1.5,\
                             isr:muRfac=1.0_fsr:muRfac=1.25 isr:muRfac=1.0 fsr:muRfac=1.25,\
                             isr:muRfac=1.0_fsr:muRfac=0.625 isr:muRfac=1.0 fsr:muRfac=0.625,\
                             isr:muRfac=1.0_fsr:muRfac=0.75 isr:muRfac=1.0 fsr:muRfac=0.75,\
                             isr:muRfac=1.0_fsr:muRfac=0.875 isr:muRfac=1.0 fsr:muRfac=0.875,\
                             hardHi fsr:cNS=2.0 isr:cNS=2.0,\
                             hardLo fsr:cNS=-2.0 isr:cNS=-2.0,\
                             isr:PDF:plus isr:PDF:plus=1,\
                             isr:PDF:minus isr:PDF:minus=2\
                             }'] 

         genSeq.Pythia8.ShowerWeightNames = [ 
                             "Var3cUp",
                             "Var3cDown",
                             "isr:muRfac=2.0_fsr:muRfac=2.0",
                             "isr:muRfac=2.0_fsr:muRfac=1.0",
                             "isr:muRfac=2.0_fsr:muRfac=0.5",
                             "isr:muRfac=1.0_fsr:muRfac=2.0",
                             "isr:muRfac=1.0_fsr:muRfac=0.5",
                             "isr:muRfac=0.5_fsr:muRfac=2.0",
                             "isr:muRfac=0.5_fsr:muRfac=1.0",
                             "isr:muRfac=0.5_fsr:muRfac=0.5",
                             "isr:muRfac=1.75_fsr:muRfac=1.0",
                             "isr:muRfac=1.5_fsr:muRfac=1.0",
                             "isr:muRfac=1.25_fsr:muRfac=1.0",
                             "isr:muRfac=0.625_fsr:muRfac=1.0",
                             "isr:muRfac=0.75_fsr:muRfac=1.0",
                             "isr:muRfac=0.875_fsr:muRfac=1.0",
                             "isr:muRfac=1.0_fsr:muRfac=1.75",
                             "isr:muRfac=1.0_fsr:muRfac=1.5",
                             "isr:muRfac=1.0_fsr:muRfac=1.25",
                             "isr:muRfac=1.0_fsr:muRfac=0.625",
                             "isr:muRfac=1.0_fsr:muRfac=0.75",
                             "isr:muRfac=1.0_fsr:muRfac=0.875",
                             "hardHi",
                             "hardLo",
                             "isr:PDF:plus",
                             "isr:PDF:minus"
                             ]
      else:
         genSeq.Pythia8.Commands += [
                            'UncertaintyBands:doVariations = on', 
                            'UncertaintyBands:List = {\
                             Var3cUp isr:muRfac=0.549241,\
                             Var3cDown isr:muRfac=1.960832,\
                             isr:muRfac=2.0_fsr:muRfac=2.0 isr:muRfac=2.0 fsr:muRfac=2.0,\
                             isr:muRfac=2.0_fsr:muRfac=1.0 isr:muRfac=2.0 fsr:muRfac=1.0,\
                             isr:muRfac=2.0_fsr:muRfac=0.5 isr:muRfac=2.0 fsr:muRfac=0.5,\
                             isr:muRfac=1.0_fsr:muRfac=2.0 isr:muRfac=1.0 fsr:muRfac=2.0,\
                             isr:muRfac=1.0_fsr:muRfac=0.5 isr:muRfac=1.0 fsr:muRfac=0.5,\
                             isr:muRfac=0.5_fsr:muRfac=2.0 isr:muRfac=0.5 fsr:muRfac=2.0,\
                             isr:muRfac=0.5_fsr:muRfac=1.0 isr:muRfac=0.5 fsr:muRfac=1.0,\
                             isr:muRfac=0.5_fsr:muRfac=0.5 isr:muRfac=0.5 fsr:muRfac=0.5,\
                             isr:muRfac=1.75_fsr:muRfac=1.0 isr:muRfac=1.75 fsr:muRfac=1.0,\
                             isr:muRfac=1.5_fsr:muRfac=1.0 isr:muRfac=1.5 fsr:muRfac=1.0,\
                             isr:muRfac=1.25_fsr:muRfac=1.0 isr:muRfac=1.25 fsr:muRfac=1.0,\
                             isr:muRfac=0.625_fsr:muRfac=1.0 isr:muRfac=0.625 fsr:muRfac=1.0,\
                             isr:muRfac=0.75_fsr:muRfac=1.0 isr:muRfac=0.75 fsr:muRfac=1.0,\
                             isr:muRfac=0.875_fsr:muRfac=1.0 isr:muRfac=0.875 fsr:muRfac=1.0,\
                             isr:muRfac=1.0_fsr:muRfac=1.75 isr:muRfac=1.0 fsr:muRfac=1.75,\
                             isr:muRfac=1.0_fsr:muRfac=1.5 isr:muRfac=1.0 fsr:muRfac=1.5,\
                             isr:muRfac=1.0_fsr:muRfac=1.25 isr:muRfac=1.0 fsr:muRfac=1.25,\
                             isr:muRfac=1.0_fsr:muRfac=0.625 isr:muRfac=1.0 fsr:muRfac=0.625,\
                             isr:muRfac=1.0_fsr:muRfac=0.75 isr:muRfac=1.0 fsr:muRfac=0.75,\
                             isr:muRfac=1.0_fsr:muRfac=0.875 isr:muRfac=1.0 fsr:muRfac=0.875,\
                             hardHi fsr:cNS=2.0 isr:cNS=2.0,\
                             hardLo fsr:cNS=-2.0 isr:cNS=-2.0\
                             }'] 

         genSeq.Pythia8.ShowerWeightNames = [ 
                             "Var3cUp",
                             "Var3cDown",
                             "isr:muRfac=2.0_fsr:muRfac=2.0",
                             "isr:muRfac=2.0_fsr:muRfac=1.0",
                             "isr:muRfac=2.0_fsr:muRfac=0.5",
                             "isr:muRfac=1.0_fsr:muRfac=2.0",
                             "isr:muRfac=1.0_fsr:muRfac=0.5",
                             "isr:muRfac=0.5_fsr:muRfac=2.0",
                             "isr:muRfac=0.5_fsr:muRfac=1.0",
                             "isr:muRfac=0.5_fsr:muRfac=0.5",
                             "isr:muRfac=1.75_fsr:muRfac=1.0",
                             "isr:muRfac=1.5_fsr:muRfac=1.0",
                             "isr:muRfac=1.25_fsr:muRfac=1.0",
                             "isr:muRfac=0.625_fsr:muRfac=1.0",
                             "isr:muRfac=0.75_fsr:muRfac=1.0",
                             "isr:muRfac=0.875_fsr:muRfac=1.0",
                             "isr:muRfac=1.0_fsr:muRfac=1.75",
                             "isr:muRfac=1.0_fsr:muRfac=1.5",
                             "isr:muRfac=1.0_fsr:muRfac=1.25",
                             "isr:muRfac=1.0_fsr:muRfac=0.625",
                             "isr:muRfac=1.0_fsr:muRfac=0.75",
                             "isr:muRfac=1.0_fsr:muRfac=0.875",
                             "hardHi",
                             "hardLo"
                             ]


