# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from .MonitorChainConfiguration import MonitorChainConfiguration
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs


from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
log.info("Importing %s",__name__)



def generateChainConfigs( chainDict ):

    listOfChainDicts = splitChainDict(chainDict)
    log.debug("Will generate Config for monitor chain: %s", chainDict['chainName'])

    listOfChainDefs = []
        
    #streamers will never have more than one chainPart but this is still
    #needed to move to the correct format [{}]->{}
    for subChainDict in listOfChainDicts:
        
        Monitor = MonitorChainConfiguration(subChainDict).assembleChain() 

        listOfChainDefs += [Monitor]
        log.debug('length of chaindefs %s', len(listOfChainDefs) )

    if len(listOfChainDefs)>1:
        log.warning("This is a streamer with more than one chainPart, is this really intended?")
        theChainDef = mergeChainDefs(listOfChainDefs, chainDict)
    else:
        theChainDef = listOfChainDefs[0]

    log.debug("theChainDef %s" , theChainDef)

    return theChainDef
