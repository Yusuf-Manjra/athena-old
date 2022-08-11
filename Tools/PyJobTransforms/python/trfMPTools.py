# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

## @package PyJobTransforms.trfMPTools
#
# @brief Utilities for handling AthenaMP jobs
# @author atlas-comp-transforms-dev@cern.ch
#

__version__ = '$Revision'

import os
import os.path as path

import logging
msg = logging.getLogger(__name__)

from xml.etree import ElementTree

from PyJobTransforms.trfExeStepTools import commonExecutorStepName
from PyJobTransforms.trfExitCodes import trfExit

import PyJobTransforms.trfExceptions as trfExceptions

## @brief Detect if AthenaMP has been requested
#  @param argdict Argument dictionary, used to access athenaopts for the job
#  @return Integer with the number of processes, N.B. 0 means non-MP serial mode
def detectAthenaMPProcs(argdict = {}, currentSubstep = '', legacyThreadingRelease = False):
    athenaMPProcs = 0
    currentSubstep = commonExecutorStepName(currentSubstep)
    
    # Try and detect if any AthenaMP has been enabled 
    try:
        if 'athenaopts' in argdict:
            for substep in argdict['athenaopts'].value:
                if substep == 'all' or substep == currentSubstep:
                    procArg = [opt.replace("--nprocs=", "") for opt in argdict['athenaopts'].value[substep] if '--nprocs' in opt]
                    if len(procArg) == 0:
                        athenaMPProcs = 0
                    elif len(procArg) == 1:
                        if 'multiprocess' in argdict and substep == 'all':
                            raise ValueError("Detected conflicting methods to configure AthenaMP: --multiprocess and --nprocs=N (via athenaopts). Only one method must be used")
                        athenaMPProcs = int(procArg[0])
                        if athenaMPProcs < -1:
                            raise ValueError("--nprocs was set to a value less than -1")
                    else:
                        raise ValueError("--nprocs was set more than once in 'athenaopts'")
                    if athenaMPProcs > 0:
                        msg.info('AthenaMP detected from "nprocs" setting with {0} workers for substep {1}'.format(athenaMPProcs,substep))
        if (athenaMPProcs == 0 and
            'ATHENA_CORE_NUMBER' in os.environ and
            (('multiprocess' in argdict and argdict['multiprocess'].value) or legacyThreadingRelease)):
            athenaMPProcs = int(os.environ['ATHENA_CORE_NUMBER'])
            if athenaMPProcs < -1:
                raise ValueError("ATHENA_CORE_NUMBER value was less than -1")
            msg.info('AthenaMP detected from ATHENA_CORE_NUMBER with {0} workers'.format(athenaMPProcs))
    except ValueError as errMsg:
        myError = 'Problem discovering AthenaMP setup: {0}'.format(errMsg)
        raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_EXEC_SETUP_FAIL'), myError)

    return athenaMPProcs

## @brief Handle AthenaMP outputs, updating argFile instances to real 
#  @param athenaMPFileReport XML file with outputs that AthenaMP knew about
#  @param athenaMPWorkerTopDir Subdirectory with AthenaMP worker run directories
#  @param dataDictionary This substep's data dictionary, allowing all files to be
#  updated to the appropriate AthenaMP worker files
#  @param athenaMPworkers Number of AthenaMP workers
#  @param skipFileChecks Switches off checks on output files
#  @return @c None; side effect is the update of the @c dataDictionary
def athenaMPOutputHandler(athenaMPFileReport, athenaMPWorkerTopDir, dataDictionary, athenaMPworkers, skipFileChecks = False, argdict = {}):
    msg.debug("MP output handler called for report {0} and workers in {1}, data types {2}".format(athenaMPFileReport, athenaMPWorkerTopDir, list(dataDictionary)))
    outputHasBeenHandled = dict([ (dataType, False) for dataType in dataDictionary if dataDictionary[dataType] ])

    # if sharedWriter mode is active ignore athenaMPFileReport
    sharedWriter=False
    if 'sharedWriter' in argdict and argdict['sharedWriter'].value:
        sharedWriter=True
        skipFileChecks=True

    if not sharedWriter:
        # First, see what AthenaMP told us
        mpOutputs = ElementTree.ElementTree()
        try:
            mpOutputs.parse(athenaMPFileReport)
        except IOError:
            raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Missing AthenaMP outputs file {0} (probably athena crashed)".format(athenaMPFileReport))
        for filesElement in mpOutputs.getroot().iter(tag='Files'):
            msg.debug('Examining element {0} with attributes {1}'.format(filesElement, filesElement.attrib))
            originalArg = None 
            startName = filesElement.attrib['OriginalName']
            for dataType, fileArg in dataDictionary.items():
                if fileArg.value[0] == startName:
                    originalArg = fileArg
                    outputHasBeenHandled[dataType] = True
                    break
            if originalArg is None:
                msg.warning('Found AthenaMP output with name {0}, but no matching transform argument'.format(startName))
                continue
        
            msg.debug('Found matching argument {0}'.format(originalArg))
            fileNameList = []
            for fileElement in filesElement.iter(tag='File'):
                msg.debug('Examining element {0} with attributes {1}'.format(fileElement, fileElement.attrib))
                fileNameList.append(path.relpath(fileElement.attrib['name']))

            athenaMPoutputsLinkAndUpdate(fileNameList, fileArg)

    # Now look for additional outputs that have not yet been handled
    if len([ dataType for dataType in outputHasBeenHandled if outputHasBeenHandled[dataType] is False]):
        # OK, we have something we need to search for; cache the dirwalk here
        MPdirWalk = [ dirEntry for dirEntry in os.walk(athenaMPWorkerTopDir) ]

        for dataType, fileArg in dataDictionary.items():
            if outputHasBeenHandled[dataType]:
                continue
            if fileArg.io == "input":
                continue
            msg.info("Searching MP worker directories for {0}".format(dataType))
            startName = fileArg.value[0]
            fileNameList = []
            for entry in MPdirWalk:
                if "evt_count" in entry[0]:
                    continue
                if "range_scatterer" in entry[0]:
                    continue
                # N.B. AthenaMP may have made the output name unique for us, so 
                # we need to treat the original name as a prefix
                possibleOutputs = [ fname for fname in entry[2] if fname.startswith(startName) ]
                if len(possibleOutputs) == 0:
                    continue
                elif len(possibleOutputs) == 1:
                    fileNameList.append(path.join(entry[0], possibleOutputs[0]))
                elif skipFileChecks:
                    pass
                else:
                    raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Found multiple matching outputs for datatype {0} in {1}: {2}".format(dataType, entry[0], possibleOutputs))
            if skipFileChecks:
                pass
            elif len(fileNameList) != athenaMPworkers:
                raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Found {0} output files for {1}, expected {2} (found: {3})".format(len(fileNameList), dataType, athenaMPworkers, fileNameList))

            # Found expected number of files - good!
            athenaMPoutputsLinkAndUpdate(fileNameList, fileArg) 


def athenaMPoutputsLinkAndUpdate(newFullFilenames, fileArg):
    # Any files we link are numbered from 1, because we always set
    # the filename given to athena has _000 as a suffix so that the
    # mother process' file can be used without linking
    fileIndex = 1
    linkedNameList = []
    newFilenameValue = []
    for fname in newFullFilenames:
        if path.dirname(fname) == "":
            linkedNameList.append(None)
            newFilenameValue.append(fname)
        else:
            linkName = "{0}{1:03d}".format(path.basename(fname).rstrip('0'), fileIndex)
            linkedNameList.append(linkName)
            newFilenameValue.append(linkName)
            fileIndex += 1
            
    for linkname, fname in zip(linkedNameList, newFullFilenames):
        if linkname:
            if len(newFullFilenames) == 1:
                try:
                    os.rename(fname,fileArg.originalName)
                    newFilenameValue[0]=fileArg.originalName
                except OSError as e:
                    raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Failed to move {0} to {1}: {2}".format(fname, linkname, e))
            else:
                 try:
                     if path.lexists(linkname):
                         os.unlink(linkname)
                     os.symlink(fname, linkname)
                 except OSError as e:  
                     raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_OUTPUT_FILE_ERROR"), "Failed to link {0} to {1}: {2}".format(fname, linkname, e))

    fileArg.multipleOK = True
    fileArg.value = newFilenameValue
    msg.debug('MP output argument updated to {0}'.format(fileArg))
    
