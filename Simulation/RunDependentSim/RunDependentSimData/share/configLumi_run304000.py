####################
## File configLumi_run304000.py: autogenerated configuration file from command
##../../RunDependentSimComps/share/RunDepProfileGenerator.py -e {0.50:0.015,1.50:0.501,2.50:0.466,3.50:0.011,4.50:0.004,5.50:0.002,6.50:0.001} -r 304000 -s 1530000000 -c 1000
## Created on Fri Jan 12 18:39:06 2018
####################
#Run-dependent digi job configuration file.
#RunDependentSimData/OverrideRunLBLumiDigitConfig.py

#We need to be able to adjust for different dataset sizes.
if not 'ScaleTaskLength' in dir():   ScaleTaskLength = 1
_evts = lambda x: int(ScaleTaskLength * x)

if not 'logging' in dir(): import logging
digilog = logging.getLogger('Digi_trf')
digilog.info('doing RunLumiOverride configuration from file.')
JobMaker=[
  {'run':304000, 'lb':1, 'starttstamp':1530000000, 'dt':0.000, 'evts':_evts(15), 'mu':0.500, 'force_new':False},
  {'run':304000, 'lb':2, 'starttstamp':1530000060, 'dt':0.000, 'evts':_evts(501), 'mu':1.500, 'force_new':False},
  {'run':304000, 'lb':3, 'starttstamp':1530000120, 'dt':0.000, 'evts':_evts(466), 'mu':2.500, 'force_new':False},
  {'run':304000, 'lb':4, 'starttstamp':1530000180, 'dt':0.000, 'evts':_evts(11), 'mu':3.500, 'force_new':False},
  {'run':304000, 'lb':5, 'starttstamp':1530000240, 'dt':0.000, 'evts':_evts(4), 'mu':4.500, 'force_new':False},
  {'run':304000, 'lb':6, 'starttstamp':1530000300, 'dt':0.000, 'evts':_evts(2), 'mu':5.500, 'force_new':False},
  {'run':304000, 'lb':7, 'starttstamp':1530000360, 'dt':0.000, 'evts':_evts(1), 'mu':6.500, 'force_new':False},
#--> end hiding
]

include('RunDependentSimData/configCommon.py')

#cleanup python memory
if not "RunDMC_testing_configuration" in dir():
    del JobMaker
