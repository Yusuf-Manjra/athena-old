## Config for Py8B tune A14 with CTEQ6L1 
## The default version of this tune fragment include EvtGen for standardised b fragmentation

# Reference the non-standard version without EvtGen
include("Pythia8B_i/Pythia8B_A14_CTEQ6L1_Common.py")

# Add EvtGen for b fragmentation as default.  No EvtGen is available in "nonStandard"
include("Pythia8B_i/Pythia8B_EvtGen.py")

