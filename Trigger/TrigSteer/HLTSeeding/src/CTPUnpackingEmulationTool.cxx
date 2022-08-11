/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#include "CTPUnpackingEmulationTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include <fstream>


CTPUnpackingEmulationTool::CTPUnpackingEmulationTool( const std::string& type,
                                                      const std::string& name,
                                                      const IInterface* parent )
  : CTPUnpackingToolBase( type, name, parent ) {}


StatusCode CTPUnpackingEmulationTool::parseInputFile() {
  std::ifstream inputFile( m_inputFileName );
  if ( not inputFile.good() ) {
    ATH_MSG_ERROR( "Input file " << m_inputFileName << " inaccessible" );
    return StatusCode::FAILURE;
  }

  std::string line;
  while ( std::getline( inputFile, line ) ) {
    if ( line[0] == '#' ) {continue;}
    std::istringstream allChains( line );
    HLT::IDVec ids;
    while ( allChains ) {
      std::string chainName;
      allChains >> chainName;
      if ( not chainName.empty() ) {
        ids.push_back( HLT::Identifier( chainName ).numeric() );
      }
    }

    if ( not ids.empty() ) {
      m_events.push_back( ids ); // new event
    }
  }
  inputFile.close();

  ATH_MSG_DEBUG( "In input file "<<m_inputFileName.value()<<" found "<<m_events.size()<<" chain sets" );
  return StatusCode::SUCCESS;
}


StatusCode CTPUnpackingEmulationTool::initialize() {
  ATH_CHECK( CTPUnpackingToolBase::initialize() );
  return parseInputFile();
}


StatusCode CTPUnpackingEmulationTool::decode( const ROIB::RoIBResult& /*roib*/,  HLT::IDVec& enabledChains ) const {
  if ( m_events.empty() ){
    ATH_MSG_ERROR( "No chain set found. Cannot decode CTP emulation" );
    return StatusCode::FAILURE;
  }

  const EventContext& context = Gaudi::Hive::currentContext();
  size_t line = context.evt() % m_events.size();
  enabledChains =  m_events[line];

  return StatusCode::SUCCESS;
}
