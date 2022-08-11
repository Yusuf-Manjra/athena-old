# $Id: PrintConfFragment.py 279081 2010-02-13 05:21:53Z beringer $
#
# Job option fragment for JobRunner templates to print and save the final job's
# jobConfig parameters. It also saves actually used top-level job options file.
#
# Written by Juerg Beringer in November 2009.
#
printfunc ("InDetBeamSpotExample INFO Using $Id: PrintConfFragment.py 279081 2010-02-13 05:21:53Z beringer $")


import pprint
import os, sys
import string
import shutil

if not 'jobConfig' in dir():
    printfunc ("InDetBeamSpotExample WARNING No jobConfig dictionary found")
    jobConfig = {}

else:

    if not 'outputfileprefix' in jobConfig:              jobConfig['outputfileprefix'] = ''
    if not 'jobname' in jobConfig:                       jobConfig['jobname'] = 'UNKNOWN'
    if not 'configfile' in jobConfig:                    jobConfig['configfile'] = 'noconfig.py'
    if not 'finalconfigfile' in jobConfig:               jobConfig['finalconfigfile'] = jobConfig['configfile']+'.final.py'
    if not 'joboptionpath' in jobConfig:                 jobConfig['joboptionpath'] = 'UNKNOWN'
    if not 'savedjoboptionpath' in jobConfig:            jobConfig['savedjoboptionpath'] = jobConfig['configfile']+'.template.py'
    if not 'doPrintFullConfig' in jobConfig:             jobConfig['doPrintFullConfig'] = False
    if not 'doSaveFullConfig' in jobConfig:              jobConfig['doSaveFullConfig'] = True
    if not 'doSaveTemplate' in jobConfig:                jobConfig['doSaveTemplate'] = True    

    printfunc ("InDetBeamSpotExample INFO Job configuration parameters (jobConfig) used by this job:")
    printfunc (pprint.pformat(jobConfig))

    if jobConfig['doSaveFullConfig']:
        try:
            f = open(jobConfig['finalconfigfile'],'w')
            f.write('# Final job configuration data for job %(jobname)s\n' % jobConfig)
            f.write('# Generated by the actual athena job\n\n')
            if pprint.isreadable(jobConfig):
                f.write('jobConfig = '+pprint.pformat(jobConfig))
            else:
                f.write('jobConfig = '+repr(jobConfig))
            f.write('\n')
            f.close()
        except:
            printfunc ("InDetBeamSpotExample WARNING Unable to save final full jobConfig dictionary")

    if jobConfig['doPrintFullConfig']:

        printfunc ("\n================================================================================")
        printfunc ("topSequence:")
        printfunc ("================================================================================")
        printfunc (topSequence)


        printfunc ("\n================================================================================")
        printfunc ("ToolSvc:")
        printfunc ("================================================================================")
        printfunc (ToolSvc)
        printfunc ("================================================================================\n")

        printfunc ("\n================================================================================")
        printfunc ("IOVDbSvc and THistSvc (if present):")
        printfunc ("================================================================================")
        if hasattr(ServiceMgr, 'IOVDbSvc'):
            printfunc (ServiceMgr.IOVDbSvc)
        if hasattr(ServiceMgr, 'THistSvc'):
            printfunc (ServiceMgr.THistSvc)
        printfunc ("================================================================================\n")

    if jobConfig['doSaveTemplate']:
        try:
            name = jobConfig['joboptionpath']
            paths = string.split(os.environ["JOBOPTSEARCHPATH"], os.pathsep)
            templatepath = filter(lambda p: os.path.exists(os.path.join(p, name)), paths)
            templatepath = templatepath and os.path.join( templatepath[0], name ) or ''
            shutil.copy(templatepath, jobConfig['savedjoboptionpath'])
        except:
            printfunc ("InDetBeamSpotExample WARNING Unable to save top-level job option file")
