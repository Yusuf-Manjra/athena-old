#########################################################
#
# SimulationJobOptions/preInclude.CavernBackgroundPostBeamDump.py
# Valeria Bartsch
# John Chapman
#
# Create an empty BeamIntensity pattern appart from the first
# entry. Then set the central bunch crossing to be fixed at the 32nd
# bunch in the pattern, this should mean that all pile-up comes from a
# bunch crossing 800ns before the trigger time. This can be used to
# look at the fall off of cavern background after the beam has been
# dumped.
#
# Use the run-dependent MC code so simulate a fall off in cavern
# background intensity overtime. Crucial to this is hacking the
# numberOfCollisions to be locked.
#
# This job option should be added via the postInclude
# command line argument. 
#
#########################################################
from Digitization.DigitizationFlags import digitizationFlags
digitizationFlags.numberOfCollisions.set_Value_and_Lock(1.0)

# central bunch crossing is 32
digitizationFlags.FixedT0BunchCrossing = 32

## Cavern Background is *dependent* on the bunch pattern.

from Digitization.DigitizationFlags import digitizationFlags

digitizationFlags.bunchSpacing = 50 # This now sets the bunch slot length.
digitizationFlags.numberOfCavern = 1 #FIXME need to check on appropriate values
# digitizationFlags.numberOfCollisions = 2.0
digitizationFlags.initialBunchCrossing = -32
digitizationFlags.finalBunchCrossing = 32
#digitizationFlags.numberOfBeamHalo = 0.05 #FIXME need to check on appropriate values
#digitizationFlags.numberOfBeamGas = 0.0003 #FIXME need to check on appropriate values
digitizationFlags.BeamIntensityPattern = [
        1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
        0.0,0.0,0.0,0.0]

digitizationFlags.cavernIgnoresBeamInt=False

from AthenaCommon.BeamFlags import jobproperties
#jobproperties.Beam.bunchSpacing = 150 # Set this to the spacing between filled bunch-crossings within the train.

####################
## File outfile: autogenerated configuration file from command
##./RunDepTaskMaker.py --outfile=outfile runlist.xml
## Created on Mon Aug  1 11:32:17 2011
####################
#Run-dependent digi job configuration file.
#SimuJobTransforms/ConfigLumi_preOptions.py

#We need to be able to adjust for different dataset sizes.
if not 'ScaleTaskLength' in dir():   ScaleTaskLength = 1
_evts = lambda x: int(ScaleTaskLength * x)

from RunDependentSimComps.RunDependentMCTaskIterator import getRunLumiInfoFragment
digilog = logging.getLogger('Digi_trf')
digilog.info('doing RunLumiOverride configuration from file.')
JobMaker=[
{'run':186049, 'lb':1, 'starttstamp':1311249272, 'dt':1.029, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':2, 'starttstamp':1311249273, 'dt':1.029, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':3, 'starttstamp':1311249274, 'dt':1.029, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':4, 'starttstamp':1311249275, 'dt':1.029, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':5, 'starttstamp':1311249276, 'dt':1.029, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':6, 'starttstamp':1311249277, 'dt':1.029, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':7, 'starttstamp':1311249278, 'dt':1.029, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':8, 'starttstamp':1311249279, 'dt':1.029, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':9, 'starttstamp':1311249280, 'dt':1.029, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':10, 'starttstamp':1311249281, 'dt':1.029, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':11, 'starttstamp':1311249282, 'dt':6.107, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':12, 'starttstamp':1311249288, 'dt':6.107, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':13, 'starttstamp':1311249294, 'dt':6.107, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':14, 'starttstamp':1311249300, 'dt':6.107, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':15, 'starttstamp':1311249306, 'dt':6.107, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':16, 'starttstamp':1311249313, 'dt':6.107, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':17, 'starttstamp':1311249319, 'dt':6.107, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':18, 'starttstamp':1311249325, 'dt':6.107, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':19, 'starttstamp':1311249331, 'dt':6.107, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':20, 'starttstamp':1311249337, 'dt':6.107, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':21, 'starttstamp':1311249343, 'dt':6.092, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':22, 'starttstamp':1311249349, 'dt':6.092, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':23, 'starttstamp':1311249355, 'dt':6.092, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':24, 'starttstamp':1311249361, 'dt':6.092, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':25, 'starttstamp':1311249367, 'dt':6.092, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':26, 'starttstamp':1311249373, 'dt':6.092, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':27, 'starttstamp':1311249380, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':28, 'starttstamp':1311249386, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':29, 'starttstamp':1311249392, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':30, 'starttstamp':1311249398, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':31, 'starttstamp':1311249404, 'dt':6.096, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':32, 'starttstamp':1311249410, 'dt':6.096, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':33, 'starttstamp':1311249416, 'dt':6.096, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':34, 'starttstamp':1311249422, 'dt':6.096, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':35, 'starttstamp':1311249428, 'dt':6.096, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':36, 'starttstamp':1311249434, 'dt':6.096, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':37, 'starttstamp':1311249441, 'dt':6.096, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':38, 'starttstamp':1311249447, 'dt':6.096, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':39, 'starttstamp':1311249453, 'dt':6.096, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':40, 'starttstamp':1311249459, 'dt':6.096, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':41, 'starttstamp':1311249465, 'dt':6.079, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':42, 'starttstamp':1311249471, 'dt':6.079, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':43, 'starttstamp':1311249477, 'dt':6.079, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':44, 'starttstamp':1311249483, 'dt':6.079, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':45, 'starttstamp':1311249489, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':46, 'starttstamp':1311249495, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':47, 'starttstamp':1311249501, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':48, 'starttstamp':1311249508, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':49, 'starttstamp':1311249514, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':50, 'starttstamp':1311249520, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':51, 'starttstamp':1311249526, 'dt':6.018, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':52, 'starttstamp':1311249532, 'dt':6.018, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':53, 'starttstamp':1311249538, 'dt':6.018, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':54, 'starttstamp':1311249544, 'dt':6.018, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':55, 'starttstamp':1311249550, 'dt':6.018, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':56, 'starttstamp':1311249556, 'dt':6.018, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':57, 'starttstamp':1311249562, 'dt':6.018, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':58, 'starttstamp':1311249568, 'dt':6.018, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':59, 'starttstamp':1311249574, 'dt':6.018, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':60, 'starttstamp':1311249580, 'dt':6.018, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':61, 'starttstamp':1311249586, 'dt':6.102, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':62, 'starttstamp':1311249592, 'dt':6.102, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':63, 'starttstamp':1311249598, 'dt':6.102, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':64, 'starttstamp':1311249604, 'dt':6.102, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':65, 'starttstamp':1311249610, 'dt':6.102, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':66, 'starttstamp':1311249617, 'dt':6.102, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':67, 'starttstamp':1311249623, 'dt':6.102, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':68, 'starttstamp':1311249629, 'dt':6.102, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':69, 'starttstamp':1311249635, 'dt':6.102, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':70, 'starttstamp':1311249641, 'dt':6.102, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':71, 'starttstamp':1311249647, 'dt':4.956, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':72, 'starttstamp':1311249652, 'dt':4.956, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':73, 'starttstamp':1311249657, 'dt':4.956, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':74, 'starttstamp':1311249662, 'dt':4.956, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':75, 'starttstamp':1311249667, 'dt':4.956, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':76, 'starttstamp':1311249672, 'dt':4.956, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':77, 'starttstamp':1311249677, 'dt':4.956, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':78, 'starttstamp':1311249682, 'dt':4.956, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':79, 'starttstamp':1311249687, 'dt':4.956, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':80, 'starttstamp':1311249692, 'dt':4.956, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':81, 'starttstamp':1311249697, 'dt':6.061, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':82, 'starttstamp':1311249703, 'dt':6.061, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':83, 'starttstamp':1311249709, 'dt':6.061, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':84, 'starttstamp':1311249715, 'dt':6.061, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':85, 'starttstamp':1311249721, 'dt':6.061, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':86, 'starttstamp':1311249727, 'dt':6.061, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':87, 'starttstamp':1311249733, 'dt':6.061, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':88, 'starttstamp':1311249739, 'dt':6.061, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':89, 'starttstamp':1311249745, 'dt':6.061, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':90, 'starttstamp':1311249752, 'dt':6.061, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':91, 'starttstamp':1311249757, 'dt':6.095, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':92, 'starttstamp':1311249763, 'dt':6.095, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':93, 'starttstamp':1311249769, 'dt':6.095, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':94, 'starttstamp':1311249775, 'dt':6.095, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':95, 'starttstamp':1311249781, 'dt':6.095, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':96, 'starttstamp':1311249787, 'dt':6.095, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':97, 'starttstamp':1311249794, 'dt':6.095, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':98, 'starttstamp':1311249800, 'dt':6.095, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':99, 'starttstamp':1311249806, 'dt':6.095, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':100, 'starttstamp':1311249812, 'dt':6.095, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':101, 'starttstamp':1311249818, 'dt':6.101, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':102, 'starttstamp':1311249824, 'dt':6.101, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':103, 'starttstamp':1311249830, 'dt':6.101, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':104, 'starttstamp':1311249836, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':105, 'starttstamp':1311249842, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':106, 'starttstamp':1311249849, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':107, 'starttstamp':1311249855, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':108, 'starttstamp':1311249861, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':109, 'starttstamp':1311249867, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':110, 'starttstamp':1311249873, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':111, 'starttstamp':1311249879, 'dt':4.617, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':112, 'starttstamp':1311249884, 'dt':4.617, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':113, 'starttstamp':1311249888, 'dt':4.617, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':114, 'starttstamp':1311249893, 'dt':4.617, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':115, 'starttstamp':1311249897, 'dt':4.617, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':116, 'starttstamp':1311249902, 'dt':4.617, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':117, 'starttstamp':1311249907, 'dt':4.617, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':118, 'starttstamp':1311249911, 'dt':4.617, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':119, 'starttstamp':1311249916, 'dt':4.617, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':120, 'starttstamp':1311249921, 'dt':4.617, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':121, 'starttstamp':1311249925, 'dt':6.079, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':122, 'starttstamp':1311249931, 'dt':6.079, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':123, 'starttstamp':1311249937, 'dt':6.079, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':124, 'starttstamp':1311249943, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':125, 'starttstamp':1311249949, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':126, 'starttstamp':1311249955, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':127, 'starttstamp':1311249961, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':128, 'starttstamp':1311249968, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':129, 'starttstamp':1311249974, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':130, 'starttstamp':1311249980, 'dt':6.079, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':131, 'starttstamp':1311249986, 'dt':6.090, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':132, 'starttstamp':1311249992, 'dt':6.090, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':133, 'starttstamp':1311249998, 'dt':6.090, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':134, 'starttstamp':1311250004, 'dt':6.090, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':135, 'starttstamp':1311250010, 'dt':6.090, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':136, 'starttstamp':1311250016, 'dt':6.090, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':137, 'starttstamp':1311250023, 'dt':6.090, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':138, 'starttstamp':1311250029, 'dt':6.090, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':139, 'starttstamp':1311250035, 'dt':6.090, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':140, 'starttstamp':1311250041, 'dt':6.090, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':141, 'starttstamp':1311250047, 'dt':6.109, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':142, 'starttstamp':1311250053, 'dt':6.109, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':143, 'starttstamp':1311250059, 'dt':6.109, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':144, 'starttstamp':1311250065, 'dt':6.109, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':145, 'starttstamp':1311250071, 'dt':6.109, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':146, 'starttstamp':1311250078, 'dt':6.109, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':147, 'starttstamp':1311250084, 'dt':6.109, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':148, 'starttstamp':1311250090, 'dt':6.109, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':149, 'starttstamp':1311250096, 'dt':6.109, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':150, 'starttstamp':1311250102, 'dt':6.109, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':151, 'starttstamp':1311250108, 'dt':6.092, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':152, 'starttstamp':1311250114, 'dt':6.092, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':153, 'starttstamp':1311250120, 'dt':6.092, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':154, 'starttstamp':1311250126, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':155, 'starttstamp':1311250132, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':156, 'starttstamp':1311250138, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':157, 'starttstamp':1311250145, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':158, 'starttstamp':1311250151, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':159, 'starttstamp':1311250157, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':160, 'starttstamp':1311250163, 'dt':6.092, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':161, 'starttstamp':1311250169, 'dt':6.108, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':162, 'starttstamp':1311250175, 'dt':6.108, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':163, 'starttstamp':1311250181, 'dt':6.108, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':164, 'starttstamp':1311250187, 'dt':6.108, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':165, 'starttstamp':1311250193, 'dt':6.108, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':166, 'starttstamp':1311250200, 'dt':6.108, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':167, 'starttstamp':1311250206, 'dt':6.108, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':168, 'starttstamp':1311250212, 'dt':6.108, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':169, 'starttstamp':1311250218, 'dt':6.108, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':170, 'starttstamp':1311250224, 'dt':6.108, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':171, 'starttstamp':1311250230, 'dt':6.101, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':172, 'starttstamp':1311250236, 'dt':6.101, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':173, 'starttstamp':1311250242, 'dt':6.101, 'evts':_evts(10), 'mu':3, 'force_new':False},
{'run':186049, 'lb':174, 'starttstamp':1311250248, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':175, 'starttstamp':1311250254, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':176, 'starttstamp':1311250261, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':177, 'starttstamp':1311250267, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':178, 'starttstamp':1311250273, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':179, 'starttstamp':1311250279, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False},
{'run':186049, 'lb':180, 'starttstamp':1311250285, 'dt':6.101, 'evts':_evts(10), 'mu':2, 'force_new':False}
#--> end hiding
]
if 'runArgs' in dir():
    if hasattr(runArgs,"jobNumber") and hasattr(runArgs,"maxEvents"):
        trfJobNumber = runArgs.jobNumber
        trfMaxEvents = runArgs.maxEvents
    else:
        digilog.error('Tried to include RunLumiOverride configuration without knowing this runArgs.jobNumber or runArgs.maxEvents!')
else:
    #this is a test job not a trf job
    trfJobNumber=0
    trfMaxEvents=10


digitizationFlags.RunAndLumiOverrideList=getRunLumiInfoFragment(jobnumber=trfJobNumber, task=JobMaker, maxEvents=trfMaxEvents)
del JobMaker

