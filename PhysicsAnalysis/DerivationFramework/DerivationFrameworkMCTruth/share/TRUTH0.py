#==============================================================================
# Set up common services and job object.
# This should appear in ALL derivation job options
#==============================================================================
# Add translator from EVGEN input to xAOD-like truth here
from DerivationFrameworkCore.DerivationFrameworkMaster import *
from RecExConfig.ObjKeyStore import objKeyStore
from xAODTruthCnv.xAODTruthCnvConf import xAODMaker__xAODTruthCnvAlg
from RecExConfig.InputFilePeeker import inputFileSummary

#ensure EventInfoCnvAlg is scheduled in the main algsequence, if not already, and is needed
from RecExConfig.InputFilePeeker import inputFileSummary
if ("EventInfo#McEventInfo" not in inputFileSummary['eventdata_itemsList']) and not any(isinstance(x,CfgMgr.xAODMaker__EventInfoCnvAlg) for x in DerivationFrameworkJob):
    DerivationFrameworkJob += CfgMgr.xAODMaker__EventInfoCnvAlg()

# Decide what kind of input HepMC container we are dealing with
if ("McEventCollection#GEN_EVENT" in inputFileSummary['eventdata_itemsList']):
    if not hasattr(DerivationFrameworkJob,'GEN_EVNT2xAOD'):
        DerivationFrameworkJob += xAODMaker__xAODTruthCnvAlg("GEN_EVNT2xAOD",AODContainerName="GEN_EVENT")
elif ("McEventCollection#TruthEvent" in inputFileSummary['eventdata_itemsList']):
    if not hasattr(DerivationFrameworkJob,'GEN_AOD2xAOD'):
        DerivationFrameworkJob += xAODMaker__xAODTruthCnvAlg("GEN_EVNT2xAOD",AODContainerName="TruthEvent")

#==============================================================================
# Create the derivation kernel algorithm
#==============================================================================
from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
DerivationFrameworkJob += CfgMgr.DerivationFramework__DerivationKernel("TRUTH0Kernel")

#==============================================================================
# Set up stream
#==============================================================================
streamName = derivationFlags.WriteDAOD_TRUTH0Stream.StreamName
fileName = buildFileName( derivationFlags.WriteDAOD_TRUTH0Stream )
TRUTH0Stream = MSMgr.NewPoolRootStream( streamName, fileName )

# Only events that pass the filters listed are written out
# AcceptAlgs  = logical OR of filters
# RequireAlgs = logical AND of filters
TRUTH0Stream.AcceptAlgs(['TRUTH0Kernel'])

#==============================================================================
# Set up slimming content list here
#==============================================================================
TRUTH0Stream.AddItem("xAOD::EventInfo#*")
TRUTH0Stream.AddItem("xAOD::EventAuxInfo#*")
TRUTH0Stream.AddItem( "xAOD::TruthEventContainer#*" )
TRUTH0Stream.AddItem( "xAOD::TruthEventAuxContainer#*" )
TRUTH0Stream.AddItem( "xAOD::TruthVertexContainer#*" )
TRUTH0Stream.AddItem( "xAOD::TruthVertexAuxContainer#*" )
TRUTH0Stream.AddItem( "xAOD::TruthParticleContainer#*" )
TRUTH0Stream.AddItem( "xAOD::TruthParticleAuxContainer#*" )
# Keep the metadata of course!
TRUTH0Stream.AddMetaDataItem( [ "xAOD::TruthMetaDataContainer#TruthMetaData", "xAOD::TruthMetaDataAuxContainer#TruthMetaDataAux." ] )

# If we don't have a conditions tag set by now, then assume this job isn't going to have one and kill the conditions service
if len(globalflags.ConditionsTag())==0:
    svcMgr.PoolSvc.ReadCatalog = []
