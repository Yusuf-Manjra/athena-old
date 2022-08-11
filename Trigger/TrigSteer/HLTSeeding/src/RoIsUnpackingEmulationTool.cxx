/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#include "RoIsUnpackingEmulationTool.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"
#include <fstream>


RoIsUnpackingEmulationTool::RoIsUnpackingEmulationTool(const std::string& type,
                                                       const std::string& name,
                                                       const IInterface* parent)
  : RoIsUnpackingToolBase ( type, name, parent ) {}


StatusCode RoIsUnpackingEmulationTool::initialize() {
  ATH_CHECK( RoIsUnpackingToolBase::initialize() );

  if (readEmulatedData().isFailure() ) {
    ATH_MSG_ERROR( "Failed to read emulated data" );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


StatusCode RoIsUnpackingEmulationTool::start() {
  ATH_CHECK(decodeMapping([&](const std::string& name){
    return name.find(m_thresholdPrefix) == 0;
  }));
  return StatusCode::SUCCESS;
}


StatusCode RoIsUnpackingEmulationTool::readEmulatedData(){
  if ( m_inputFilename.empty() ) {
    ATH_MSG_WARNING("Provided InputFilename parameter is empty");
    return StatusCode::RECOVERABLE;
  }
  try {
    m_currentRowNumber = 0;
    m_inputData = parseInputFile();
  }
  catch (const std::exception& ex) {
    ATH_MSG_WARNING(ex.what());
    return StatusCode::RECOVERABLE;
  }

  ATH_MSG_DEBUG("Input file parsed successfully. Data:");

  for (auto& fragment : m_inputData) {
    ATH_MSG_DEBUG("Event");
    for (auto& fakeRoI : fragment) {
      const char* const delim = ", ";
      std::ostringstream debugPassedTresholdIDs;
      std::copy(fakeRoI.passedThresholdIDs.begin(),
                fakeRoI.passedThresholdIDs.end(),
                std::ostream_iterator<std::string>(debugPassedTresholdIDs, delim));

      ATH_MSG_DEBUG("Eta: " + std::to_string(fakeRoI.eta) +
                    ", Phi: " + std::to_string(fakeRoI.phi) +
                    ", Word: " + std::to_string(fakeRoI.roIWord) +
                    ", PassedThresholdIDs: " + debugPassedTresholdIDs.str());
    }
  }

  return StatusCode::SUCCESS;
}


std::vector<std::vector<RoIsUnpackingEmulationTool::FakeRoI>> RoIsUnpackingEmulationTool::parseInputFile() {
  const char* whiteSpaceChars = " \t\n\r\f\v";
  auto rtrim = [whiteSpaceChars](std::string& s) { s.erase(s.find_last_not_of(whiteSpaceChars) + 1); };
  auto ltrim = [whiteSpaceChars](std::string& s) { s.erase(0, s.find_first_not_of(whiteSpaceChars)); };
  auto trim = [&rtrim, &ltrim](std::string& s) { ltrim(s); rtrim(s); };

  std::vector<std::vector<RoIsUnpackingEmulationTool::FakeRoI>> result;
  unsigned lineNumber = 0;
  std::string line;

  std::ifstream inputFile(m_inputFilename);
  if (!inputFile.good())  {
    throw std::invalid_argument("File " + m_inputFilename + "does not exists or is corrupted");
  }

  while (getline(inputFile, line)) {
    trim(line);
    if (line.empty()) {continue;}
    result.push_back(parseInputFileLine(line, ++lineNumber));
  }

  if (result.empty()) {
    throw std::invalid_argument("File " + m_inputFilename + " does not contain any RoI");
  }
  ATH_MSG_DEBUG("Read in " << result.size() << " pseudo events from " << m_inputFilename );
  return result;
}


std::vector<RoIsUnpackingEmulationTool::FakeRoI>
RoIsUnpackingEmulationTool::parseInputFileLine(const std::string& line, unsigned lineNumber) {
  std::vector<RoIsUnpackingEmulationTool::FakeRoI> result;
  unsigned roiNumber = 0;
  std::stringstream inputLine(line);
  std::string roi;

  while (getline(inputLine, roi, ';'))  {
    // FIXME: If there aren't as many input fields in roi as parseInputRoI expects,
    // it'll return uninitialized data (which can lead to a practically infinite
    // loop when we try to normalize the phi). Add an elementary check here to
    // fix such a loop observed from creatingEVTest in ViewAlgsTest.
    if (roi.find (',') != std::string::npos) {
      result.push_back(parseInputRoI(roi, lineNumber, ++roiNumber));
    }
  }
  return result;
}


RoIsUnpackingEmulationTool::FakeRoI
RoIsUnpackingEmulationTool::parseInputRoI(const std::string& roi, unsigned lineNumber, unsigned roiNumber) {
  RoIsUnpackingEmulationTool::FakeRoI result;
  std::stringstream inputRoi(roi);
  std::string roiElement;
  unsigned roiElementNumber = 0;

  std::vector<std::function<void(const std::string&)>> fillResultFunctors;
  fillResultFunctors.emplace_back([&](const std::string& eta) { result.eta = stod(eta); });
  fillResultFunctors.emplace_back([&](const std::string& phi) { result.phi = stod(phi); });
  fillResultFunctors.emplace_back([&](const std::string& word) { result.roIWord = stoul(word); });
  fillResultFunctors.emplace_back([&](const std::string& passedThresholdID) {
    result.passedThresholdIDs.push_back(passedThresholdID); });

  while (getline(inputRoi, roiElement, ','))        {
    if (roiElementNumber > 2) { roiElementNumber = 3; }
    try{
      fillResultFunctors[roiElementNumber++](roiElement);
    }
    catch (const std::exception& ex) {
      throw std::invalid_argument("Exception encountered while parsing line " + std::to_string(lineNumber) +
                                  ", roi " + std::to_string(roiNumber) + ". Exception: " + ex.what());
    }
  }

  return result;
}


StatusCode RoIsUnpackingEmulationTool::unpack(const EventContext& ctx,
                                              const ROIB::RoIBResult& /*roib*/,
                                              const HLT::IDSet& activeChains ) const {
  using namespace TrigCompositeUtils;

  // create and record the collections needed
  SG::WriteHandle<TrigRoiDescriptorCollection> trigRoIs = createAndStoreNoAux(m_trigRoIsKey, ctx );
  SG::WriteHandle<DecisionContainer> decisionOutput = createAndStore(m_decisionsKey, ctx );

  // retrieve fake data for this event
  if (m_inputData.empty()){
    ATH_MSG_ERROR("No input dataset found. Cannot decode RoI emulation");
    return StatusCode::FAILURE;
  }
  const size_t line = ctx.evt() % m_inputData.size();
  ATH_MSG_DEBUG("Getting RoIs for event "<< line);
  auto FakeRoIs = m_inputData[line];

  for (auto& roi : FakeRoIs) {
    uint32_t roIWord = roi.roIWord;
    trigRoIs->push_back( std::make_unique<TrigRoiDescriptor>(
      roIWord, 0u ,0u,
      roi.eta, roi.eta-m_roIWidth, roi.eta+m_roIWidth,
      roi.phi, roi.phi-m_roIWidth, roi.phi+m_roIWidth) );

    ATH_MSG_DEBUG( "RoI word: 0x" << MSG::hex << std::setw(8) << roIWord << MSG::dec );

    // This hltSeedingNodeName() denotes an initial node with no parents
    auto *decision  = TrigCompositeUtils::newDecisionIn( decisionOutput.ptr(), hltSeedingNodeName() );
    std::vector<TrigCompositeUtils::DecisionID> passedThresholdIDs;

    for ( const auto& th: roi.passedThresholdIDs ) {
      ATH_MSG_DEBUG( "Passed Threshold " << th << " enabling respective chains " );
      passedThresholdIDs.push_back( HLT::Identifier(th) );
      addChainsToDecision( HLT::Identifier( th ), decision, activeChains );
      decision->setObjectLink( initialRoIString(),
                               ElementLink<TrigRoiDescriptorCollection>(m_trigRoIsKey.key(), trigRoIs->size()-1) );
    }

    decision->setDetail( "thresholds", passedThresholdIDs );
  }

  for ( auto roi: *trigRoIs ) {
    ATH_MSG_DEBUG("RoI Eta: " << roi->eta() << " Phi: " << roi->phi() << " RoIWord: " << roi->roiWord());
  }

  return StatusCode::SUCCESS;
}
