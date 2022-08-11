/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "HLTEDMCreatorAlg.h"

HLTEDMCreatorAlg::HLTEDMCreatorAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode HLTEDMCreatorAlg::initialize()
{
  ATH_CHECK( m_tools.retrieve() );
  return StatusCode::SUCCESS;
}

StatusCode HLTEDMCreatorAlg::execute(const EventContext& context) const
{
  for ( const auto& t: m_tools )  {
    ATH_CHECK( t->createOutput( context ) );
  }
  return StatusCode::SUCCESS;
}

