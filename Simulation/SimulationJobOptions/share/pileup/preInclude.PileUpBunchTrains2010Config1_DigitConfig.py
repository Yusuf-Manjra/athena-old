####################################################################################
## Trains of 24 filled bunch crossings with intra train spacings of 150ns expcept 
## for a gap of 300ns between the 12th and 13th filled bunch crossings in the train.
## Any of the filled bunch crossings can be chosen as the
## central bunch crossing, so the timing of the out-of-time will vary significantly
## from event to event.
## Cavern Background is independent of the bunch pattern. (Compatible with 25ns cavern background.)
####################################################################################

from Digitization.DigitizationFlags import digitizationFlags

digitizationFlags.bunchSpacing = 25 # This now sets the bunch slot length.
digitizationFlags.numberOfCavern = 1 #FIXME need to check on appropriate values
digitizationFlags.numberOfCollisions = 1.5
digitizationFlags.initialBunchCrossing = -32
digitizationFlags.finalBunchCrossing = 32
digitizationFlags.numberOfBeamHalo = 0.05 #FIXME need to check on appropriate values
digitizationFlags.numberOfBeamGas = 0.0003 #FIXME need to check on appropriate values
if hasattr(digitizationFlags, 'numberOfLowPtMinBias'): digitizationFlags.numberOfLowPtMinBias = 2.2 
if hasattr(digitizationFlags, 'numberOfHighPtMinBias'): digitizationFlags.numberOfHighPtMinBias = 0.0022 
if hasattr(digitizationFlags, 'numberOfNDMinBias'): digitizationFlags.numberOfNDMinBias = 1.5
if hasattr(digitizationFlags, 'numberOfSDMinBias'): digitizationFlags.numberOfSDMinBias = 0.05
if hasattr(digitizationFlags, 'numberOfDDMinBias'): digitizationFlags.numberOfDDMinBias = 0.05
digitizationFlags.BeamIntensityPattern = [
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing  1 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing  2 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing  3 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing  4 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing  5 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing  6 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing  7 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing  8 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing  9 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 10 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 11 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 12 (300ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 13 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 14 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 15 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 16 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 17 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 18 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 19 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 20 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 21 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 22 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 23 (150ns gap)
    1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,# filled bunch crossing 24 (900ns gap i.e. beyond atlas exposure time)
    0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,
    0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]

from AthenaCommon.BeamFlags import jobproperties
jobproperties.Beam.bunchSpacing = 150 # Set this to the spacing between filled bunch-crossings within the train.
