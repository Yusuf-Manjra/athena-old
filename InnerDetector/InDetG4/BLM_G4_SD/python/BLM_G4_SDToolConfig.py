# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg


def BLMSensorSDCfg(ConfigFlags, name="BLMSensorSD", **kwargs):
    bare_collection_name = "BLMHits"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "BLMHits"
    region = "ID"

    acc, hits_collection_name = CollectionMergerCfg(ConfigFlags,
                                                    bare_collection_name,
                                                    mergeable_collection_suffix,
                                                    merger_input_property,
                                                    region)
    kwargs.setdefault("LogicalVolumeNames", ["Pixel::blmDiamondLog"])
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    result = ComponentAccumulator()
    result.merge(acc)
    result.setPrivateTools(CompFactory.BLMSensorSDTool(name, **kwargs))
    return result
