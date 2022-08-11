# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitChainDict
from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
from .StreamingChainConfiguration import StreamingChainConfiguration


from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)
log.info("Importing %s",__name__)



def generateChainConfigs( chainDict ):

    listOfChainDicts = splitChainDict(chainDict)
    log.debug("Will generate Config for streamer: %s", chainDict['chainName'])

    listOfChainDefs = []
        
    #streamers will never have more than one chainPart but this is still
    #needed to move to the correct format [{}]->{}
    for subChainDict in listOfChainDicts:
        
        Streaming = StreamingChainConfiguration(subChainDict).assembleChain() 

        listOfChainDefs += [Streaming]
        log.debug('length of chaindefs %s', len(listOfChainDefs) )

    if len(listOfChainDefs)>1:
        log.warning("This is a streamer with more than one chainPart, is this really intended?")
        theChainDef = mergeChainDefs(listOfChainDefs, chainDict)
    else:
        theChainDef = listOfChainDefs[0]

    log.debug("theChainDef %s" , theChainDef)

    return theChainDef

