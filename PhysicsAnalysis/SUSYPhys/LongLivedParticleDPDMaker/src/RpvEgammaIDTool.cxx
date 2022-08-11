/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RpvEgammaIDTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Straight copy of DerivationFrameworkEGamma/EGammaPassSelectionWrapper.cxx
// because DerivationFrameworkEGamma is not in AtlasProduction
// From DerivationFrameworkEGamma/EGammaPassSelectionWrapper.cxx:
// Author: James Catmore (James.Catmore@cern.ch)
// Wrapper around the passSelection() method of xAOD egamma
// Writes result to SG for later selection by string parser

#include "LongLivedParticleDPDMaker/RpvEgammaIDTool.h"
#include "xAODEgamma/EgammaContainer.h"
#include <vector>
#include <string>

// Constructor
DerivationFramework::RpvEgammaIDTool::RpvEgammaIDTool( const std::string& t,
						       const std::string& n,
						       const IInterface* p ) :
  AthAlgTool(t,n,p)
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    declareProperty("SelectionVariable",m_selectionString);
  }
 
// Destructor
DerivationFramework::RpvEgammaIDTool::~RpvEgammaIDTool() {
}

// Athena initialize and finalize
StatusCode DerivationFramework::RpvEgammaIDTool::initialize()
{
     ATH_MSG_VERBOSE("initialize() ...");
     ATH_CHECK(m_collNameKey.initialize());
     ATH_CHECK(m_egammaSelectionKey.initialize());
     return StatusCode::SUCCESS;
}
StatusCode DerivationFramework::RpvEgammaIDTool::finalize()
{
     ATH_MSG_VERBOSE("finalize() ...");
     return StatusCode::SUCCESS;
}

// Augmentation
StatusCode DerivationFramework::RpvEgammaIDTool::addBranches() const
{

     // Retrieve data
     SG::ReadHandle<xAOD::EgammaContainer> egammas(m_collNameKey);
     if( !egammas.isValid() ) {
        ATH_MSG_ERROR("Couldn't retrieve e-gamma container with key: " << m_collNameKey);
        return StatusCode::FAILURE;
     }
       
     SG::WriteHandle< std::vector<int> > egammaSelection(m_egammaSelectionKey);
     ATH_CHECK(egammaSelection.record(std::make_unique< std::vector<int> >()));

     // Loop over egammas, set decisions   
     for (xAOD::EgammaContainer::const_iterator eIt = egammas->begin(); eIt!=egammas->end(); ++eIt) {
        bool val(0);
        if ( (*eIt)->passSelection(val,m_selectionString) ) {
          if (val) {egammaSelection->push_back(1);}
          else {egammaSelection->push_back(0);}
        }
        else{
          ATH_MSG_WARNING("Evident problem with quality flag " << m_selectionString << " so setting to false!");
          egammaSelection->push_back(0);
        }
     }     

     return StatusCode::SUCCESS;

}
