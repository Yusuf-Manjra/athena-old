# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#--------------------------------------------------------------
# This is an example joboption to generate events with Powheg
# using ATLAS' interface. Users should optimise and carefully
# validate the settings before making an official sample request.
#--------------------------------------------------------------

#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.description = "POWHEG+Pythia8 W production in Standard Model Effective Field Theory with A14 NNPDF2.3 tune."
evgenConfig.keywords = ["SM", "W", "BSM"]
evgenConfig.contact = ["tpelzer@cern.ch"]

# --------------------------------------------------------------
# Load ATLAS defaults for the Powheg W_SMEFT process
# --------------------------------------------------------------
include("PowhegControl/PowhegControl_W_SMEFT_Common.py")

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------

# Enable dimension-6 operators
PowhegConfig.dim6 = 1

# Engable flavour-changing neutral currents
PowhegConfig.fcnc = 1

# Set scale of new physics (in GeV)
PowhegConfig.LambdaNP = 1000.0

# Set dimension-6 operator coefficients
PowhegConfig.ReGUw_uu = 0.0
PowhegConfig.ImGUw_uu = 0.0
PowhegConfig.ReGDw_ds = 0.0
PowhegConfig.ImGDw_dd = 0.0
PowhegConfig.QLu_uu = 0.001
PowhegConfig.QLd_dd = 0.001
PowhegConfig.ReLedQ_dd = 0.001
PowhegConfig.ReLeQu_uu = 0.001
PowhegConfig.ReLeQu3_uu = 0.001
PowhegConfig.ReLeQu_uc = 0.01
PowhegConfig.ReLedQ_ds = 0.01
PowhegConfig.ReLeQu3_uc = 0.01
PowhegConfig.QLd_sb = 0.01

PowhegConfig.generate()

#--------------------------------------------------------------
# Pythia8 showering with the A14 NNPDF2.3 tune, main31 routine
#--------------------------------------------------------------
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_Powheg_Main31.py")
