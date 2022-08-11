#!/usr/bin/env python

# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
import TopExamples.grid
import DerivationTags
import Data_rel21
import MC16_TOPQ1

## fancy function to shorten the physics part of the name of a sample
#def MyFancyShortener(superLongInDSName):
    #splitted = superLongInDSName.split('.')
    #runNumber = splitted[1]
    #physicsName = splitted[2]
    #if splitted[0] == "user" or splitted[0] == "group": #this is in case we run on private derivations, either produced with user or group role
        #runNumber = splitted[2]
        #physicsName = splitted[3]
    #tags = splitted[-1].replace('/','')
    #physicsName = physicsName.split('_')[0]
    #outDSName = runNumber + '.' + physicsName + '.someFunnyTags'
    #return outDSName

config = TopExamples.grid.Config()
config.code          = 'top-xaod'
config.settingsFile  = 'validation-cuts.txt'
# in case you want to run multiple configurations in a single file:
#config.settingsFile  = 'validation-cuts_Topo.txt,validation-cuts_PFlow.txt'
#config.combine_outputFile = 'out.root'
#config.combine_prefixes = 'Topo,PFlow'

config.gridUsername  = 'iconnell' # use e.g. phys-top or phys-higgs for group production
config.suffix        = '17-10-03'
config.excludedSites = ''
config.noSubmit      = False # set to True if you just want to test the submission
config.mergeType     = 'Default' #'None', 'Default' or 'xAOD'
config.destSE        = '' #This is the default (anywhere), or try e.g. 'UKI-SOUTHGRID-BHAM-HEP_LOCALGROUPDISK'
# by default the requested memory is set to 2GB, if you need to increase this, see the option below
# config.memory = '4000' # NOTE grid sites with 4GB for single-core user jobs are rare
#config.nameShortener = MyFancyShortener # to use your own physics part shortening function - uncomment here and in the function definition above

###############################################################################

###Command line interface
###If you want a script that ask you what you want to run on interactively,
###and uses lists of primary xAODs to convert them as TOPQ derivations
###Otherwise you can edit the automatic scripts below
#names, derivation, ptag = DerivationTags.InteractiveSubmission()
#samples = TopExamples.grid.Samples(names)
#TopExamples.grid.convertAODtoTOPQ(derivation, ptag, samples)
#TopExamples.grid.submit(config, samples)

###############################################################################

# Data - look in Data_rel21.py
# Change if you want TOPQ2/3/4/5
names = ['Data15_TOPQ1', 
         'Data16_TOPQ1', 
         'Data17_TOPQ1']
samples = TopExamples.grid.Samples(names)
TopExamples.ami.check_sample_status(samples)  # Call with (samples, True) to halt on error
TopExamples.grid.submit(config, samples)

###############################################################################

###MC Simulation - look in MC16_TOPQ1.py
###Using list of TOPQ1 25ns MC samples, consistent mixture of p-tags
###Edit these lines if you don't want to run everything!
names = ['TOPQ1_ttbar_PowPy8',
         'TOPQ1_ttbar_dil_PowPy8',
         'TOPQ1_tchan_lep_PowPy6',
         'TOPQ1_Wt_inc_PowPy6',
         'TOPQ1_schan_noAllHad_PowPy6',
         'TOPQ1_diboson_Sherpa',
         'TOPQ1_Zjets_Sherpa221',
         'TOPQ1_Wjets_Sherpa221',
         'TOPQ1_ttV',    
         ]
samples = TopExamples.grid.Samples(names)
TopExamples.ami.check_sample_status(samples)  # Call with (samples, True) to halt on error
TopExamples.grid.submit(config, samples)
