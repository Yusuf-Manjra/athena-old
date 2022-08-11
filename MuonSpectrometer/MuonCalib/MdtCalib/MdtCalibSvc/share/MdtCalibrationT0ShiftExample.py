# example configuration of MdtCalibrationT0ShiftTool and MdtCalibTMaxShiftTool
# in this example the 0ns shift maps are loaded, which means that the
# modification code runs, but it does not change anything


"""
NEVER recreate the shift maps when you're running multiple jobs.
If you want to recreate, run a single job to create the map and configure the
jobs to load the new map when submitting them.
It's all just random numbers and you cannot be sure that the tubes will get the
same shifts in all jobs.
(Yes, the RNGs are seeded, but what if someone changes this later?)
"""

from MdtCalibT0ShiftTool.MdtCalibT0ShiftToolConf import MdtCalibrationT0ShiftTool
ServiceMgr += MdtCalibrationT0ShiftTool()
ServiceMgr.MdtCalibrationT0ShiftTool.MapFile = 'shift_t0_0ns.dat'  # which shift map to load / how to save it if not existing (in the package's share folder)
ServiceMgr.MdtCalibrationT0ShiftTool.CentralValue = 0              # central value of Gaussian RNG - only relevant if you recreate the map
ServiceMgr.MdtCalibrationT0ShiftTool.Sigma = 0                     # sigma value of Gaussian RNG - only relevant if you recreate the map
ServiceMgr.MdtCalibrationT0ShiftTool.ForceMapRecreate = False      # force recreation of map - never do this when submitting multiple jobs
