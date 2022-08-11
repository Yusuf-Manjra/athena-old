####################
## File configLumi_mc12_v2_run212399.py: autogenerated configuration file from command
##/afs/cern.ch/atlas/software/builds/nightlies/devval/AtlasSimulation/rel_1/InstallArea/share/bin/RunDepTaskMaker.py --externalDict={0.0: 5.0, 1.0: 5.0, 2.0: 5.0, 3.0: 5.0, 4.0: 5.0, 5.0: 5.0, 6.0: 5.0, 7.0: 18.0, 8.0: 47.0, 9.0: 103.0, 10.0: 147.0, 11.0: 205.0, 12.0: 248.0, 13.0: 276.0, 14.0: 285.0, 15.0: 303.0, 16.0: 30.0, 17.0: 303.0, 18.0: 303.0, 19.0: 293.0, 20.0: 282.0, 21.0: 275.0, 22.0: 249.0, 23.0: 236.0, 24.0: 210.0, 25.0: 185.0, 26.0: 156.0, 27.0: 141.0, 28.0: 105.0, 29.0: 88.0, 30.0: 65.0, 31.0: 51.0, 32.0: 31.0, 33.0: 18.0, 34.0: 16.0, 35.0: 9.0, 36.0: 6.0, 37.0: 2.0, 38.0: 2.0, 39.0: 2.0, 40.0: 2.0} --lumimethod=EXTERNAL --nMC=1 --trigger=L1_MBTS_2 --outfile=configLumi_mc12_v2_run212399.py run212272.xml
## Note, timestamps and LB numbers from 212272!
## Created on Fri Mar 28 14:42:54 2014
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
   {'run':212399, 'lb':344, 'starttstamp':1349622992, 'dt':0.000, 'evts':_evts(5), 'mu':0.00001, 'force_new':False},
   {'run':212399, 'lb':345, 'starttstamp':1349623054, 'dt':0.000, 'evts':_evts(5), 'mu':1.000, 'force_new':False},
   {'run':212399, 'lb':346, 'starttstamp':1349623115, 'dt':0.000, 'evts':_evts(5), 'mu':2.000, 'force_new':False},
   {'run':212399, 'lb':347, 'starttstamp':1349623175, 'dt':0.000, 'evts':_evts(5), 'mu':3.000, 'force_new':False},
   {'run':212399, 'lb':348, 'starttstamp':1349623195, 'dt':0.000, 'evts':_evts(5), 'mu':4.000, 'force_new':False},
   {'run':212399, 'lb':349, 'starttstamp':1349623247, 'dt':0.000, 'evts':_evts(5), 'mu':5.000, 'force_new':False},
   {'run':212399, 'lb':350, 'starttstamp':1349623308, 'dt':0.000, 'evts':_evts(5), 'mu':6.000, 'force_new':False},
   {'run':212399, 'lb':351, 'starttstamp':1349623368, 'dt':0.000, 'evts':_evts(18), 'mu':7.000, 'force_new':False},
   {'run':212399, 'lb':352, 'starttstamp':1349623430, 'dt':0.000, 'evts':_evts(47), 'mu':8.000, 'force_new':False},
   {'run':212399, 'lb':353, 'starttstamp':1349623491, 'dt':0.000, 'evts':_evts(103), 'mu':9.000, 'force_new':False},
   {'run':212399, 'lb':354, 'starttstamp':1349623552, 'dt':0.000, 'evts':_evts(147), 'mu':10.000, 'force_new':False},
   {'run':212399, 'lb':355, 'starttstamp':1349623613, 'dt':0.000, 'evts':_evts(205), 'mu':11.000, 'force_new':False},
   {'run':212399, 'lb':356, 'starttstamp':1349623661, 'dt':0.000, 'evts':_evts(248), 'mu':12.000, 'force_new':False},
   {'run':212399, 'lb':357, 'starttstamp':1349623722, 'dt':0.000, 'evts':_evts(276), 'mu':13.000, 'force_new':False},
   {'run':212399, 'lb':358, 'starttstamp':1349623784, 'dt':0.000, 'evts':_evts(285), 'mu':14.000, 'force_new':False},
   {'run':212399, 'lb':359, 'starttstamp':1349623844, 'dt':0.000, 'evts':_evts(303), 'mu':15.000, 'force_new':False},
   {'run':212399, 'lb':361, 'starttstamp':1349623966, 'dt':0.000, 'evts':_evts(303), 'mu':16.000, 'force_new':False},
   {'run':212399, 'lb':362, 'starttstamp':1349624002, 'dt':0.000, 'evts':_evts(303), 'mu':17.000, 'force_new':False},
   {'run':212399, 'lb':363, 'starttstamp':1349624012, 'dt':0.000, 'evts':_evts(303), 'mu':18.000, 'force_new':False},
   {'run':212399, 'lb':364, 'starttstamp':1349624023, 'dt':0.000, 'evts':_evts(293), 'mu':19.000, 'force_new':False},
   {'run':212399, 'lb':365, 'starttstamp':1349624084, 'dt':0.000, 'evts':_evts(282), 'mu':20.000, 'force_new':False},
   {'run':212399, 'lb':366, 'starttstamp':1349624145, 'dt':0.000, 'evts':_evts(275), 'mu':21.000, 'force_new':False},
   {'run':212399, 'lb':367, 'starttstamp':1349624206, 'dt':0.000, 'evts':_evts(249), 'mu':22.000, 'force_new':False},
   {'run':212399, 'lb':368, 'starttstamp':1349624267, 'dt':0.000, 'evts':_evts(236), 'mu':23.000, 'force_new':False},
   {'run':212399, 'lb':369, 'starttstamp':1349624328, 'dt':0.000, 'evts':_evts(210), 'mu':24.000, 'force_new':False},
   {'run':212399, 'lb':370, 'starttstamp':1349624389, 'dt':0.000, 'evts':_evts(185), 'mu':25.000, 'force_new':False},
   {'run':212399, 'lb':371, 'starttstamp':1349624450, 'dt':0.000, 'evts':_evts(156), 'mu':26.000, 'force_new':False},
   {'run':212399, 'lb':372, 'starttstamp':1349624511, 'dt':0.000, 'evts':_evts(141), 'mu':27.000, 'force_new':False},
   {'run':212399, 'lb':373, 'starttstamp':1349624572, 'dt':0.000, 'evts':_evts(105), 'mu':28.000, 'force_new':False},
   {'run':212399, 'lb':374, 'starttstamp':1349624633, 'dt':0.000, 'evts':_evts(88), 'mu':29.000, 'force_new':False},
   {'run':212399, 'lb':375, 'starttstamp':1349624694, 'dt':0.000, 'evts':_evts(65), 'mu':30.000, 'force_new':False},
   {'run':212399, 'lb':376, 'starttstamp':1349624756, 'dt':0.000, 'evts':_evts(51), 'mu':31.000, 'force_new':False},
   {'run':212399, 'lb':377, 'starttstamp':1349624816, 'dt':0.000, 'evts':_evts(31), 'mu':32.000, 'force_new':False},
   {'run':212399, 'lb':378, 'starttstamp':1349624877, 'dt':0.000, 'evts':_evts(18), 'mu':33.000, 'force_new':False},
   {'run':212399, 'lb':379, 'starttstamp':1349624938, 'dt':0.000, 'evts':_evts(16), 'mu':34.000, 'force_new':False},
   {'run':212399, 'lb':380, 'starttstamp':1349624999, 'dt':0.000, 'evts':_evts(9), 'mu':35.000, 'force_new':False},
   {'run':212399, 'lb':381, 'starttstamp':1349625060, 'dt':0.000, 'evts':_evts(6), 'mu':36.000, 'force_new':False},
   {'run':212399, 'lb':382, 'starttstamp':1349625121, 'dt':0.000, 'evts':_evts(2), 'mu':37.000, 'force_new':False},
   {'run':212399, 'lb':383, 'starttstamp':1349625182, 'dt':0.000, 'evts':_evts(2), 'mu':38.000, 'force_new':False},
   {'run':212399, 'lb':384, 'starttstamp':1349625244, 'dt':0.000, 'evts':_evts(2), 'mu':39.000, 'force_new':False},
   {'run':212399, 'lb':385, 'starttstamp':1349625304, 'dt':0.000, 'evts':_evts(2), 'mu':40.000, 'force_new':False},
#--> end hiding
]

include('RunDependentSimData/configCommon.py')

#cleanup python memory
if not "RunDMC_testing_configuration" in dir():
    del JobMaker
