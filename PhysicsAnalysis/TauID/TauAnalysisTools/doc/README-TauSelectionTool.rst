================
TauSelectionTool
================

:authors: Dirk Duschinger, David Kirchmeier
:contact: guillermo.nicolas.hamity@cern.ch, dirk.duschinger@cern.ch, david.kirchmeier@cern.ch

.. contents:: Table of contents 

------------
Introduction
------------


This tool intends to perform simple selections on a set of tau properties. By
default a set of recommended cuts are applied. To use it you first need to
include the corresponding header file::

  #include "TauAnalysisTools/TauSelectionTool.h"
  
The tool at least needs to be created and initialized like::

  TauAnalysisTools::TauSelectionTool TauSelTool( "TauSelectionTool" );
  TauSelTool->initialize();
  
This creates the tool with the recommended cuts, which are defined in the
default config file
/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/TauAnalysisTools/00-04-00/Selection/recommended_selection_r22.conf
(or in newer versions).

------------------
Tool configuration
------------------

The default config file looks like this::

  SelectionCuts: PtMin AbsEtaRegion AbsCharge NTracks JetIDWP EleIDWP

  PtMin: 20
  AbsEtaRegion: 0; 1.37; 1.52; 2.5
  AbsCharge: 1
  NTracks: 1; 3
  JetIDWP: JETIDRNNMEDIUM
  EleIDWP: ELEIDRNNLOOSE

The top line lists the cuts to be applied. Below are the configurations on the
cuts, like the pt threshold of 20 GeV. If there is a cut specified, e.g. `PtMin:
20` but `PtMin` is not listed in `SelectionCuts`, the cut will not be made.

Entries with semicolons are treated as list of values, which only makes sense
for setups of vector type (e.g. ``AbsEtaRegion`` or ``NTracks``).

It is possible to use a hashtag (``#``) as the first character of the line, to
ignore the line.

The following table gives an overview of all currently available cuts and their
setup:

.. list-table:: 
   :header-rows: 1
   :widths: 5 5 5 55 30
   
   * - Cut
     - Cut setup
     - Type
     - Description
     - Note
     
   * - ``CutPt``
     - ``PtRegion``
     - ``std::vector<double>``
     - accepting taus within pt regions (in GeV), each `odd` in the vector is a lower bound, each `even` is an upper bound
     -
     
   * -
     - ``PtMin``
     - ``double``
     - accepting taus with a pt above a lower bound (in GeV)
     - if ``PtMin`` is configured, ``PtRegion`` configuration wont be considered

   * -
     - ``PtMax``
     - ``double``
     - accepting taus with a pt below an upper bound (in GeV)
     - if ``PtMax`` is configured, ``PtRegion`` configuration wont be considered

   * - ``CutAbsEta``
     - ``AbsEtaRegion``
     - ``std::vector<double>``
     - accepting taus within absolute eta regions, each `odd` in the vector is a lower bound, each `even` is an upper bound
     -
     
   * -
     - ``AbsEtaMin``
     - ``double``
     - accepting taus with an absolute eta above a lower bound
     - if ``AbsEtaMin`` is configured, ``AbsEtaRegion`` configuration wont be considered

   * -
     - ``AbsEtaMax``
     - ``double``
     - accepting taus with an absolute eta below an upper bound
     - if ``AbsEtaMax`` is configured, ``AbsEtaRegion`` configuration wont be considered

   * - ``CutAbsCharge``
     - ``AbsCharges``
     - ``std::vector<int>``
     - accepting taus with a set of absolute charges, each value in the vector will be accepted
     - 

   * - 
     - ``AbsCharge``
     - ``double``
     - accepting taus with the given absolute charge
     - if ``AbsCharge`` is configured, ``AbsCharges`` configuration wont be considered

   * - ``CutNTrack``
     - ``NTracks``
     - ``std::vector<unsigned int>``
     - accepting taus with a set of track multiplicities, each value in the vector will be accepted
     -

   * -
     - ``NTrack``
     - ``double``
     - accepting taus with the given track multiplicity
     - if ``NTrack`` is configured, ``NTracks`` configuration wont be considered

   * - ``CutJetRNNScoreSigTrans``
     - ``JetRNNSigTransRegion``
     - ``std::vector<double>``
     - accepting taus within jet RNN score regions, each `odd` in the vector is a lower bound, each `even` is an upper bound
     - ``JetRNNSigTrans`` is a transformed RNN score and provides flat ID efficiencies with respect to pT and pile-up. 

   * -
     - ``JetRNNSigTransMin``
     - ``double``
     - accepting taus with a jet RNN score above a lower bound
     - if ``JetRNNMin`` is configured, ``JetRNNRegion`` configuration wont be considered. ``JetRNNSigTrans`` is a transformed RNN score and provides flat ID efficiencies with respect to pT and pile-up. 

   * - 
     - ``JetRNNSigTransMax``
     - ``double``
     - accepting taus with a jet RNN score below an upper bound
     - if ``JetRNNMax`` is configured, ``JetRNNRegion`` configuration wont be considered. ``JetRNNSigTrans`` is a transformed RNN score and provides flat ID efficiencies with respect to pT and pile-up. 

   * - ``CutJetIDWP``
     - ``JetIDWP``
     - ``int``
     - accepting taus passing the given working point
     -

   * - ``CutEleRNNScore``
     - ``EleRNNRegion``
     - ``std::vector<double>``
     - accepting taus within electron RNN score regions, each `odd` in the vector is a lower bound, each `even` is an upper bound
     - 

   * -
     - ``EleRNNMin``
     - ``double``
     - accepting taus with a electron RNN score above a lower bound
     - if ``EleRNNMin`` is configured, ``EleRNNRegion`` configuration wont be considered

   * -
     - ``EleRNNMax``
     - ``double``
     - accepting taus with a electron RNN score below an upper bound
     - if ``EleRNNMax`` is configured, ``EleRNNRegion`` configuration wont be considered

   * - ``CutEleIDWP``
     - ``EleIDWP``
     - ``int``
     - accepting taus passing the given working point
     - 

   * - ``CutMuonOLR``
     - ``MuonOLR``
     - ``bool``
     - if ``MuonOLR == true``, removing tau overlapped with muon satisfying pt>2GeV and not calo-tagged
     - should only be used for run 2 analysis

Currently implemented working points for ``CutJetIDWP`` are:

.. list-table::
   :header-rows: 1

   * - Jet ID working points
     - description
     
   * - JETIDNONE
     - no cut at all

   * - JETIDRNNVERYLOOSE
     - passing RNN very loose working point, ID efficiency 95%

   * - JETIDRNNLOOSE
     - passing RNN loose working point, ID efficiency 85% (75%) for 1-prong (3-prong)
     
   * - JETIDRNNMEDIUM
     - passing RNN medium working point, ID efficiency 75% (60%) for 1-prong (3-prong)
     
   * - JETIDRNNTIGHT
     - passing RNN tight working point, ID efficiency 60% (45%) for 1-prong (3-prong)
     

and for ``CutEleIDWP``:

.. list-table::
   :header-rows: 1
   :widths: 5 30

   * - Electron veto working points
     - description
     
   * - ELEIDRNNLOOSE
     - passing electron RNN loose working point. Electron ID efficiency 95% (98%) for 1-prong (3-prong)
     
   * - ELEIDRNNMEDIUM
     - passing electron RNN medium working point. Electron ID efficiency 90% (95%) for 1-prong (3-prong)
     
   * - ELEIDRNNTIGHT
     - passing electron RNN tight working point. Electron ID efficiency 85% (90%) for 1-prong (3-prong)

If one wants to use a different setup one has three options:

1. Using an own config file
===========================

One needs to create a new file like the recommended_selection_r22.conf and
modify it as needed. You then have to tell the tool where it can find your
configuration file via::

  TauSelTool.setProperty( "ConfigPath", "/PATH/TO/CONFIG/FILE"); 

**IMPORTANT:** the last line of the file needs to be an empty line or should
only contain a comment (starting with the number sign #). Otherwise the tool
might be not properly configured (the last line is ignored by the file parser).

2. Overwrite particular cut setups or the list of cuts to be executed
=====================================================================

If particular cuts are modified, e.g. if one wants to select only taus above pT
> 100 GeV one would do::

  TauSelTool.setProperty("PtMin", 100.);

only the property will be overwritten, but all other cuts in the config file
will be applied as they are defined in the file.

Notes:

#. If one wants to specify the list of cuts to be applied, one can set the
   property ``SelectionCuts`` to a combination of enums defined in
   `TauSelectionTool.h <../TauAnalysisTools/TauSelectionTool.h>`_, which need to
   be casted to int, e.g.::

     TauSelTool.setProperty("SelectionCuts", int(TauAnalysisTools::CutPt |
                                                 TauAnalysisTools::CutAbsEta |
                                                 TauAnalysisTools::CutNTracks);

#. If one wants to use a different working point, e.g. for ``CutJetIDWP`` one
   needs to pass an enum, defined in `Enums.h <../TauAnalysisTools/Enums.h>`_,
   which need to be casted to int, e.g.::

     TauSelTool.setProperty("JetIDWP", int(TauAnalysisTools::JETIDRNNTIGHT));

#. Vector based variables need to get a vector of the correct type. I.e. to
   achieve the same configuration as in the config file::
     
     AbsEtaRegion: 0; 1.37; 1.52; 2.5

   one needs the following code lines::

     std::vector<double> vAbsEtaRegion = {0, 1.37, 1.52, 2.5};
     TauSelTool.setProperty("AbsEtaRegion", vAbsEtaRegion);
   
3. Don't load any config file
=============================

If the property ``ConfigPath`` is set to an empty string::

  TauSelTool.setProperty( "ConfigPath", "");

no config file will be loaded. In this case, if no other properties are
configured, the tool will accept any tau. However, configuration can be achieved
as described in the `previous section
<README-TauSelectionTool.rst#overwrite-particular-cut-setups-or-the-list-of-cuts-to-be-executed>`_.

     
----------------
Tool application
----------------

To test if a tau has passed all selection requirements just ask::

  TauSelTool.accept(xTau);

where xTau needs to be of type ``xAOD::TauJet`` or ``xAOD::IParticle*``. The
function returns a Root::TAccept value, equivalent to ``true``, in case all cuts
defined in the property ``"SelectionCuts"`` are passed, and equivalent to
``false`` otherwise. I.e. most users might make use of the following line in
their analyses::

  if (TauSelTool.accept(xTau))
  {
    // do stuff with accepted taus
    // ...
  }


------------------
Control histograms
------------------
     
This tool has the ability to create control histograms (currently it work not in
EventLoop). Therefore the `option` "CreateControlPlots" must be set to true::
     
  TauSelTool.setProperty("CreateControlPlots", true );

Also the tool needs to know where to write the histograms which is configured by
passing a pointer to the output file::

  TauSelTool.setOutFile( fOutputFile );

After all wanted selections have been made the histograms are written to the
file, via::

  TauSelTool.writeControlHistograms();
  
This adds a folder to the output file named by concatenating the tool name with
the prefix "_control". This folder contains a cutflow histogram showing the
number of processed tau objects before all cuts, and after each applied
cut. Additional control distributions before and after after all cuts are
stored in this folder.

---
FAQ
---

#. **Question:** How can I explicitly not perform a specific cut?

   **Answer:** This can be done by removing the cut name in the line starting
   with *SelectionCuts*.

#. **Question:** How can I find out, whether I correctly configured the tool and
   which cuts will be applied?

   **Answer:** If the tool is initialized with DEBUG message level
   (``TauSelTool->msg().setLevel( MSG::DEBUG );``) you will see for example such
   an output::

    TauSelectionTool          DEBUG Pt: 20 to inf
    TauSelectionTool          DEBUG AbsEta: 0 to 1.37
    TauSelectionTool          DEBUG AbsEta: 1.52 to 2.5
    TauSelectionTool          DEBUG AbsCharge: 1
    TauSelectionTool          DEBUG NTrack: 1
    TauSelectionTool          DEBUG NTrack: 3
    TauSelectionTool          DEBUG BDTJetScore: -inf to inf
    TauSelectionTool          DEBUG BDTEleScore: -inf to inf
    TauSelectionTool          DEBUG JetIDWP: JETIDNONE
    TauSelectionTool          DEBUG EleBDTDWP: ELEIDNONE
    TauSelectionTool          DEBUG cuts: Pt AbsEta AbsCharge NTrack JetIDWP

   **Note:** only the cuts in the last line will be processed

----------
Navigation
----------

* `TauAnalysisTools <../README.rst>`_

  * `TauSelectionTool <README-TauSelectionTool.rst>`_
  * `TauSmearingTool <README-TauSmearingTool.rst>`_
  * `TauEfficiencyCorrectionsTool <README-TauEfficiencyCorrectionsTool.rst>`_

    * `TauEfficiencyCorrectionsTool Trigger <README-TauEfficiencyCorrectionsTool_Trigger.rst>`_

  * `TauTruthMatchingTool <README-TauTruthMatchingTool.rst>`_
  * `TauTruthTrackMatchingTool <README-TauTruthTrackMatchingTool.rst>`_
