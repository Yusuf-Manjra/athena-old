## Config for Py8 tune A14 with MSTW2008LO
include("Pythia8_i/Pythia8_Base_Fragment.py")

genSeq.Pythia8.Commands += [
    "Tune:pp = 5",
#    "PDF:pSet=LHAPDF6:MSTW2008lo68cl",
    "SpaceShower:rapidityOrder = on",
    "SigmaProcess:alphaSvalue = 0.140",
    "SpaceShower:pT0Ref = 1.62",
    "SpaceShower:pTmaxFudge = 0.92",
    "SpaceShower:pTdampFudge = 1.14",
    "SpaceShower:alphaSvalue = 0.129",
    "TimeShower:alphaSvalue = 0.129",
    "BeamRemnants:primordialKThard = 1.82",
    "MultipartonInteractions:pT0Ref = 2.22",
    "MultipartonInteractions:alphaSvalue = 0.127"]
#    "BeamRemnants:reconnectRange = 1.87"]

rel = os.popen("echo $AtlasVersion").read()

genSeq.Pythia8.Commands += ["PDF:pSet=LHAPDF6:MSTW2008lo68cl",
   "ColourReconnection:range = 1.87"]                            

evgenConfig.tune = "A14 MSTW2008LO"
