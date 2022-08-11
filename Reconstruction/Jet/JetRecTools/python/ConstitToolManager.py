# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging, CfgMgr


# -----------------------------------
# ConstitModifier
# -----------------------------------
class ConstituentToolManager(object):
    """This class is modelled after JetToolManager.
    It is intended to be a central place where to
      * register constituent modifier tools
      * map them to shortcut
      * collect them in a mod sequencer tool
    This class is provided standalone, but could be merged with JetToolManager if needed.

    and example call could be :

    clustModSeq = ctm.buildConstitModifSequence( 'JetWeightedTopoClusters',
                                             inputName= 'CaloCalTopoClusters',
                                             modList = [  'clust_weight'] )


    """

    # map of tool names or alias to tool instance
    #  (there can be multiple entries pointing to the same instance) 
    modifiersMap = dict()

    # map of named standard list of modifiers
    standardModifierLists = dict()
    import cppyy
    try:
        cppyy.load_library('libxAODBaseObjectTypeDict')
    except Exception:
        pass
    from ROOT import xAODType
    xAODType.ObjectType

    # map of known input collections to their type
    inputContainerMap = dict( CaloCalTopoClusters = xAODType.CaloCluster, CaloTopoClusters = xAODType.CaloCluster,
                              EMOriginTopoClusters = xAODType.CaloCluster, LCOriginTopoClusters = xAODType.CaloCluster, 
                              InDetTrackParticles = xAODType.TrackParticle, JetETMiss = xAODType.FlowElement )

    log = Logging.logging.getLogger("ConstituentToolManager")

    def add(self, tool, alias=None):
        """Register a tool in the manager.  If alias is given, the tool is also registered under the alias key"""
        name = tool.name()
        if name in self.modifiersMap:
            self.log.warning("Tool named "+name+" already registered. Not adding a new one")
            return self.modifiersMap[name]
        if alias is not None and alias in self.modifiersMap:
            self.log.warning("Tool named "+alias+" already registered. Not adding a new one under this alias. Was "+name)            
            return self.modifiersMap[alias]

        self.modifiersMap[name] = tool
        if alias: self.modifiersMap[alias] = tool
        return tool

    
    def __iadd__(self, tool):
        """Enables the += syntax to add tool """
        self.add(tool)
        return self

    def buildConstitModifSequence(self, seqName, OutputContainer , InputContainer, modList, InputType=None):
        """Returns a configured JetConstituentModSequence instance.
        The tool is build according to
           seqName (str)    : name of the JetConstituentModSequence tool
           OutputContainer (str) : name of desired output constituents container (property of JetConstituentModSequence)
           InputContainer  (str) : name of input constituents container (property of JetConstituentModSequence)
           modList (str or list): if str, this is taken as a shortcut to a knonw, default list of modifier tools (from self.standardModifierLists)
                                  if list, entries are eihter configured modifier tools either strings in which case they are shortcut to known modifier tool (in self.modifiersMap).
          InputType : (int or None) the type of particles in the input container. If None attempt is made to guess it from InputContainer (from self.inputContainerMap).
          
        """
        
        #seqName = OutputContainer+'_modSeq'

        # Deal with input -----------------
        if InputType is None:
            # get it from the known inputs
            InputType = self.inputContainerMap[ InputContainer ]
        if InputType is None:
            self.log.error( seqName+'. Unknown input container : '+InputContainer )
            return 

        # deal with modifiers ---------------
        if isinstance(modList, str):
            modKey = modList
            # translate into a known list :
            modList = self.standardModifierLists.get( modKey , None)
            if modList is None :
                self.log.error( seqName+". Unknown shortcut for constit modifier list : "+modKey)
                return None
        # loop over modList
        finalList = []
        for t in modList:
            if isinstance(t,str):
                # translate into a real tool
                tool = self.modifiersMap.get(t,None)
                if tool is None:
                    self.log.error( seqName+". Unknown shortcut for constit modifier list : "+t)
                    return None
                t = tool
                t.InputType = InputType
            # append to the final list
            finalList.append( t )
            
        constModSeq = CfgMgr.JetConstituentModSequence( seqName, # the name of the tool 
                                                        InputContainer = InputContainer,
                                                        OutputContainer = OutputContainer,
                                                        InputType = InputType,
                                                        Modifiers = finalList, # pass the list of modifier we want 
                                                        )
        self += constModSeq
        
        return constModSeq


# -----------------------------------
# the main ConstituentToolManager instance
ctm = ConstituentToolManager()
# -----------------------------------
    
# -----------------------------------
# add standard tools to ctm
ctm.add( CfgMgr.SoftKillerWeightTool("JetConstit_SoftKiller", SKGridSize=0.6) ,
         alias = 'softkiller' )

ctm.add( CfgMgr.ClusterAtEMScaleTool("JetConstit_ClusEM") ,
         alias = 'clus_emscale' )

ctm.add( CfgMgr.CaloClusterConstituentsOrigin("JetConstit_ClusOrigin") ,
         alias = 'clus_origin' )
