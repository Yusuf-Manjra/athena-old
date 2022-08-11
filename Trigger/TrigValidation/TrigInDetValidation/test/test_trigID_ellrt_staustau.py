#!/usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# art-description: art job for mu_Zmumu_pu40
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-input-nfiles: 4
# art-athena-mt: 8
# art-html: https://idtrigger-val.web.cern.ch/idtrigger-val/TIDAWeb/TIDAart/?jobdir=
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: *.root
# art-output: *.check*
# art-output: HLT*
# art-output: times*
# art-output: cost-perCall
# art-output: cost-perEvent
# art-output: cost-perCall-chain
# art-output: cost-perEvent-chain
# art-output: *.dat 


Slices  = ['L2electronLRT']
Events  = 8000 
Threads = 8 
Slots   = 8
Input   = 'StauStau'    # defined in TrigValTools/share/TrigValInputs.json
GridFiles = False
Release = "current"

ExtraAna = ' -c LRT="True" '

preinclude_file = 'all:TrigInDetValidation/TIDAlrt_preinclude.py'


Jobs = [ ( "Truth",  " TIDAdata-run3-lrt.dat -o data-hists.root -p 11", "Test_bin_lrt.dat" ),
         ( "Offline",    " TIDAdata-run3-offline-lrt.dat -r Offline -o data-hists-offline.root", "Test_bin_lrt.dat" ) ]

Comp = [ ( "L2electronLRT",  "L2electronLRT",  "data-hists.root",  " -c TIDAhisto-panel.dat -d HLTL2-plots -sx Reference Truth " ),
         ( "EFelectronLRT",  "EFelectronLRT", "data-hists.root",   " -c TIDAhisto-panel.dat -d HLTEF-plots -sx Reference Truth " ),
         ( "L2electronLRToffline",   "L2electronLRT","data-hists-offline.root",   " -c TIDAhisto-panel.dat -d HLTL2-plots-offline -sx Reference Offline " ),
         ( "EFelectronLRToffline",   "EFelectronLRT", "data-hists-offline.root",   " -c TIDAhisto-panel.dat -d HLTEF-plots-offline -sx Reference Offline " )
       ]


from AthenaCommon.Include import include 
include("TrigInDetValidation/TrigInDetValidation_Base.py")
