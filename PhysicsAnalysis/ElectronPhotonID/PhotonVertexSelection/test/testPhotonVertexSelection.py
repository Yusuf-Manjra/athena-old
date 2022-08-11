#!/usr/bin/env python

# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

__doc__ = """Script / jobOptions to test PhotonVertexSelectionTool using an AOD from
mc15_13TeV:mc15_13TeV.341000.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_gamgam.merge.AOD.e3806_s2608_r7772_r7676"""
__author__ = "Bruno Lenzi"

import os
import sys

defaultFile = os.environ.get("ASG_TEST_FILE_MC", None)
defaultNevents = 5

def printMethod(x):
  print (x)

def getViewContainer(container):
  """getViewContainer(container) --> return a view container with at most 2 egamma
  objects from the given container, ignoring topo-seeded photons and fwd electrons"""
  import ROOT
  from xAODEgamma.xAODEgammaParameters import xAOD
  def filterAuthor(x):
    return x.author() not in [xAOD.EgammaParameters.AuthorCaloTopo35, xAOD.EgammaParameters.AuthorFwdElectron]

  egammas = container.__class__(ROOT.SG.VIEW_ELEMENTS)
  for eg in list(filter(filterAuthor, container))[:2]:
    egammas.push_back( eg )
  return egammas

def printOutput(container, tool, printMethod = printMethod):
  "printOutput(egammas, tool) -> print case and MVA output"
  # TODO: pointing, HPV, zcommon
  if len(container) < 2: return
  # ignore conversions if running on electrons
  result = tool.getVertex(container, 'Electron' in container.__class__.__name__ )
  printMethod('Case: %s' % tool.getCase() )
  for vertex, mva in result:
    printMethod('  Vertex %s: %s' % (vertex.index(), mva) )


def setupAthenaJob(algoClass, inputfile = defaultFile, EvtMax = None):
  "Setup athena job"
  import AthenaPoolCnvSvc.ReadAthenaPool # EventSelector
  from AthenaCommon.AppMgr import ServiceMgr as svcMgr, theApp

  svcMgr.EventSelector.InputCollections = [inputfile]

  # Redefine the function InputFileNames to make autoconfiguration work
  # outside RecExCommon
  from RecExConfig import RecoFunctions
  RecoFunctions.InputFileNames = lambda : svcMgr.EventSelector.InputCollections
  from RecExConfig.AutoConfiguration import ConfigureFromListOfKeys
  ConfigureFromListOfKeys(['everything'])

  ###################

  from egammaRec.Factories import ToolFactory, AlgFactory
  from RecExConfig.RecFlags  import rec
  import PhotonVertexSelection.PhotonVertexSelectionConf as PVS

  # Configure and PhotonVertexSelectionTool
  PhotonVertexSelectionTool = ToolFactory(PVS.CP__PhotonVertexSelectionTool)

  # Configure the test algorithm and add it to topSequence
  testAlg = AlgFactory(algoClass,
    PhotonVertexSelectionTool = PhotonVertexSelectionTool)()

  from AthenaCommon.Constants import INFO
  theApp.setOutputLevel(INFO)
  theApp.run( EvtMax )

# --------------------------------
# Athena algorithm and setup
# --------------------------------
import os
if 'ROOTCOREBIN' not in os.environ:
  from AthenaPython import PyAthena
  from AthenaPython.PyAthena import StatusCode

  class TestPhotonVertexSelection( PyAthena.Alg ):
    def __init__(self, name = 'TestPhotonVertexSelection',
                       containerName = 'Photons', **kw):
      super(TestPhotonVertexSelection,self).__init__(name = name, containerName = containerName, **kw)

    def initialize(self):
      self.msg.info("initializing [%s]", self.name)
      self.vertexTool = PyAthena.py_tool(self.PhotonVertexSelectionTool.getFullName(),
        iface='CP::IPhotonVertexSelectionTool')
      if not self.vertexTool:
          self.msg.error("Problem retrieving PhotonVertexSelectionTool !!")
          return PyAthena.StatusCode.Failure

      return StatusCode.Success

    def execute(self):
      viewContainer = getViewContainer( self.evtStore[self.containerName] )
      printOutput(viewContainer, self.vertexTool, self.msg.info)
      return StatusCode.Success

    def finalize(self):
      return StatusCode.Success

  setupAthenaJob( TestPhotonVertexSelection,
                  locals().get('inputfile', defaultFile),
                  locals().get('EvtMax', defaultNevents ) )


# --------------------------------
# AnalysisRelease script
# --------------------------------
else:
  from optparse import OptionParser
  parser = OptionParser("%prog [options]")
  parser.description = __doc__
  parser.add_option("-N", "--nEvents", help="Events to run (default: %default)",
    default=defaultNevents, type=int)
  parser.add_option("-i", "--inputfile", help="Inputfile",
    default=defaultFile)
  parser.add_option("-c", "--container", help="Container to use (default: %default)",
    default='Photons')
  parser.epilog = "\n"

  (options, _ ) = parser.parse_args()
  if len( _ ):
    raise ValueError('Only named options are allowed, got %s' % _ )
  print ('Analysing %s from %s' % (options.container, options.inputfile))

  import ROOT

  # Initialize the xAOD infrastructure:
  ROOT.xAOD.Init().ignore()
  ROOT.xAOD.L2CombinedMuonContainer()
  ROOT.xAOD.TrigElectronContainer()
  ROOT.xAOD.MuonContainer()
  ROOT.xAOD.ParticleContainer()

  # Setup the tools
  vertexTool = ROOT.CP.PhotonVertexSelectionTool("PhotonVertexSelectionTool")

  # Create transient tree: has to be done before initialising the pointing tool
  f = ROOT.TFile.Open( options.inputfile, "READ" )
  t = ROOT.xAOD.MakeTransientTree( f, "CollectionTree", ROOT.xAOD.TEvent.kAthenaAccess )
  import xAODRootAccess.GenerateDVIterators

  # Initialise tools
  if not vertexTool.initialize().isSuccess():
    print( "Couldn't initialise the vertex tool" )
    sys.exit( 1 )
    pass

  for entry in range(options.nEvents):
    print ('*** Analysing entry %s ***' % entry)
    _ = t.GetEntry(entry)
    container = getattr(t, options.container)
    viewContainer = getViewContainer( container )
    printOutput(viewContainer, vertexTool)
    print ('')
