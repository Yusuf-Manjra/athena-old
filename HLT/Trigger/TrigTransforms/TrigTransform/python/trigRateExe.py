#!/usr/bin/env python

# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# @brief: Simple executor to call RatesAnalysisFullMenu.py
# @details: Trig_reco_tf.py executor for rate processing
# @author: Mark Stockton

from PyJobTransforms.trfExe import scriptExecutor

# Setup logging here
import logging
msg = logging.getLogger("PyJobTransforms." + __name__)

class trigRateExecutor(scriptExecutor):

    def preExecute(self, input = set(), output = set()):

        # Build up the command line: RatesAnalysisFullMenu.py Input.Files=inputAODFile --outputHist=outputNTUP_TRIGRATEFile
        # All arguments have to be provided for step to be called
        # inputAODFile can be multiple files
        self._cmd = [self._exe]
        self._cmd.extend(['Input.Files=' + str(self.conf.dataDictionary['AOD'].value) ])
        self._cmd.extend(['--outputHist='+self.conf.argdict['outputNTUP_TRIGRATEFile'].value[0]])

        if 'rateopts' in self.conf.argdict:
            args = self.conf.argdict['rateopts'].value['all']
            if isinstance(args, list):
                self._cmd.extend([ str(v) for v in args])
            else:
                self._cmd.append(str(args))

        super(trigRateExecutor, self).preExecute()
