# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil
import sys


class AtlCoolTriggerTool:

    def __init__(self, options={}):
        self.opt = options
        if not self.check_options():
            sys.exit(0)
        self.init()



    def check_options(self):
        opt = self.opt
        if opt.l1 and not opt.menu:
            print ("Option l1 requires option 'menu' to be set")
            return False
            
        if opt.l2 and not opt.menu:
            print ("Option l2 requires option 'menu' to be set")
            return False
                
        if opt.ef and not opt.menu:
            print ("Option ef requires option 'menu' to be set")
            return False

        if opt.processing and opt.processing!='hlt':
            print ("Option p|processing must be set to 'hlt'")
            return False

        if opt.menu and not (opt.l1 or opt.l2 or opt.ef):
            opt.l1 = opt.l2 = opt.ef = True

        return True

    def init(self):
        self.runlist = TriggerCoolUtil.GetRunRanges(self.opt.runlist)
        self.dbconn  = TriggerCoolUtil.GetConnection(self.opt.db,self.opt.verbosity)


    def getConfigKeys(self):
        hltkeys   = TriggerCoolUtil.getHLTConfigKeys(self.dbconn, self.runlist)
        hltpskeys = TriggerCoolUtil.getHLTPrescaleKeys(self.dbconn, self.runlist)
        l1keys    = TriggerCoolUtil.getL1ConfigKeys(self.dbconn, self.runlist)

        runs = list(set(list(hltkeys)+list(hltpskeys)+list(l1keys)))
        runs.sort()
        runStartTimes = None
        if self.opt.time:
            runStartTimes = TriggerCoolUtil.getRunStartTime(self.dbconn, self.runlist, runs)

        allkeys = dict([(r,{}) for r in runs])
        for r in runs:
            if r in hltkeys:   allkeys[r].update(hltkeys[r])
            if r in hltpskeys: allkeys[r].update(hltpskeys[r])
            if r in l1keys:    allkeys[r].update(l1keys[r])
            if runStartTimes and r in runStartTimes: allkeys[r].update(runStartTimes[r])

        return (runs, allkeys)

    def printHLTProcessingString(self,runKeys):
        runs = runKeys.keys()
        runs.sort()


        for r in runs:
            keys = runKeys[r]
            print ('c'.join(["%su%s" % (x[1],x[0]) for x in keys["HLTPSK2"]]))

    def printConfigKeys(self, runKeys):
        #t = string.Template('run $RUN ($STARTTIME) release %10')
        runs = list(runKeys)
        runs.sort()

        for r in runs:
            keys = runKeys[r]
            timestr=""
            if self.opt.time:
                try:             timestr = "(%s)  " % keys["STARTTIME"]
                except KeyError: timestr = "(--------unknown---------)  "
            try:
                rel = keys["REL"]
                smk = keys["SMK"]
                hltpsk = "%4i (1-)" % keys["HLTPSK"]
            except KeyError:
                rel = "unknown"
                smk = "0"
                hltpsk = "unknown"

            hltpsknew = hltpsk
            if r > 127453:
                hltpsknew = ""
                if "HLTPSK2" in keys:
                    for x in keys["HLTPSK2"]:
                        if x[2]<0:       hltpsknew += "%4i (%i-)" % (x[0],x[1])
                        elif x[2]==x[1]: hltpsknew += "%4i (%i)   " % (x[0],x[1])
                        else:            hltpsknew += "%4i (%i-%i)   " % (x[0],x[1],x[2])
                if not hltpsknew: hltpsknew = "unknown"

            lvl1psk = ""
            if "LVL1PSK" in keys:
                for x in keys["LVL1PSK"]:
                    if x[2]<0:       lvl1psk += "%4i (%i-)" % (x[0],x[1])
                    elif x[2]==x[1]: lvl1psk += "%4i (%i) " % (x[0],x[1])
                    else:            lvl1psk += "%4i (%i-%i) " % (x[0],x[1],x[2])
            if not lvl1psk: lvl1psk = "unknown"

            print ("run  %6i %srelease  %9s smk %4s hltps %s lvl1ps %s" % (r, timestr, rel, smk, hltpsknew, lvl1psk),)

            if r > 127453 and "HLTPSK2" in keys and "HLTPSK" in keys and keys["HLTPSK"] != keys["HLTPSK2"][0][0]:
                msg = "WARNING: Menu folder shows different HLT prescale for SOR: %i" % keys["HLTPSK"]
                print (msg)
            else: print()

    def printMenu(self, run):
        if not self.opt.menu: return
        if self.opt.l1:
            TriggerCoolUtil.printL1Menu(self.dbconn, run, self.opt.verbosity)
        if self.opt.l2 or self.opt.ef:
            TriggerCoolUtil.printHLTMenu(self.dbconn, run, self.opt.verbosity, printL2=self.opt.l2, printEF=self.opt.ef)

    def printStreams(self,run):
        if not self.opt.streams: return
        TriggerCoolUtil.printStreams(self.dbconn,run,self.opt.verbosity)
            
    def execute(self):
        (runs, configKeys) = self.getConfigKeys()

        if self.opt.processing=='hlt':
            self.printHLTProcessingString(configKeys)
            return
        else:
            self.printConfigKeys(configKeys)

        self.opt.isSingleRun = len(runs)==1

        if self.opt.menu and not self.opt.isSingleRun:
            print ("Error: option 'menu' works only for single run")
            sys.exit(0)

        if self.opt.streams and not self.opt.isSingleRun:
            print ("Error: option 'streams' works only for single run")
            sys.exit(0)

        if self.opt.isSingleRun:
            self.printMenu(runs[0])
            self.printStreams(runs[0])
