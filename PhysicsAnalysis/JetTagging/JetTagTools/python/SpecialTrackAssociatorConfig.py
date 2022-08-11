# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

# Import a reco TTVA tool
from TrackVertexAssociationTool.getTTVAToolForReco import getTTVAToolForReco

def SpecialTrackAssociatorCfg( name = 'SpecialTrackAssociator', PrimaryVertexCollectionName="", useBTagFlagsDefaults = True, **options ):
    """Sets up a SpecialTrackAssociator tool and returns it.

    The following options have BTaggingFlags defaults:

    input:             name: The name of the tool (should be unique).
          useBTagFlagsDefaults : Whether to use BTaggingFlags defaults for options that are not specified.
                  **options: Python dictionary with options for the tool.
    output: The actual tool."""
    acc = ComponentAccumulator()
    if useBTagFlagsDefaults:
        defaults = { 'WorkingPoint': 'Loose' }
    for option in defaults:
        options.setdefault(option, defaults[option])
    options['name'] = name
    acc.setPrivateTools(getTTVAToolForReco(returnCompFactory=True, **options))

    return acc
