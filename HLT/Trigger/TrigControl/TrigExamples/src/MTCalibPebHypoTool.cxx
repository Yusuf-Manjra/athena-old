/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// Trigger includes
#include "MTCalibPebHypoTool.h"
#include "TrigPartialEventBuilding/PEBInfoWriterToolBase.h" // Defines the PEBInfo keys expected by StreamTagMakerTool

// Athena includes
#include "AthenaKernel/Timeout.h"
#include "AthenaKernel/AthStatusCode.h"

// System includes
#include <random>
#include <thread>
#include <sstream>
#include <algorithm>

// Local implementation-specific helper methods
namespace {
  using rng_t = std::mt19937_64;
  using seed_t = rng_t::result_type;
  /// Calculate seed from EventID and tool name
  seed_t eventSeed(const EventIDBase& eventID, const std::string& name) {
    uint64_t evtNum = eventID.event_number();
    uint64_t runNum = eventID.run_number();
    uint64_t nameHash = std::hash<std::string>{}(name);
    uint64_t seed = evtNum ^ (runNum << 10) ^ nameHash;
    return static_cast<seed_t>(seed);
  }
  /// Returns a reference to static thread-local random number generator
  rng_t& threadLocalGenerator() {
    static thread_local std::random_device rd; // used only to ensure different seeds for mt19937
    static thread_local rng_t generator(rd());
    return generator;
  }
  /// Basic random real number generation
  template<typename T> T randomRealNumber(const T min, const T max) {
    std::uniform_real_distribution<T> distribution(min, max);
    return distribution(threadLocalGenerator());
  }
  /// Basic random integer generation
  template<typename T> T randomInteger(const T min, const T max) {
    std::uniform_int_distribution<T> distribution(min, max);
    return distribution(threadLocalGenerator());
  }
  /// Random bool with a given true rate
  bool randomAccept(const double acceptRate) {
      return (randomRealNumber(0.0, 1.0) < acceptRate);
  }
  /// ROBFragments vector print helper
  /// Can't do it via operator<< as this is in an anonymous namespace.
  std::string format(const IROBDataProviderSvc::VROBFRAG& robFragments) {
    std::ostringstream ss;
    for (const IROBDataProviderSvc::ROBF* robf : robFragments) {
      ss << "---> ROB ID = 0x" << std::hex << robf->rob_source_id() << std::dec << std::endl
          << "     ROD ID = 0x" << std::hex << robf->rod_source_id() << std::dec << std::endl
          << "     ROD Level-1 ID = " << robf->rod_lvl1_id() << std::endl;
    }
    return ss.str();
  }
  /// Print helper for a container with ROB/SubDet IDs
  template<typename Container>
  const std::string idsToString(const Container& ids) {
    std::ostringstream str;
    for (const uint32_t id : ids)
      str << "0x" << std::hex << id << std::dec << " ";
    return str.str();
  }
  /// Append PEB information to the Decision object
  StatusCode appendPEBInfo(TrigCompositeUtils::Decision* decision,
                           std::set<uint32_t> robIDsToAdd,
                           std::set<uint32_t> subDetIDsToAdd) {
    // Merge with previous ROBs
    std::vector<uint32_t> previousRobs;
    if (decision->getDetail(PEBInfoWriterToolBase::robListKey(), previousRobs)) {
      robIDsToAdd.insert(previousRobs.begin(), previousRobs.end());
    }

    // Merge with previous SubDets
    std::vector<uint32_t> previousSubDets;
    if (decision->getDetail(PEBInfoWriterToolBase::subDetListKey(), previousSubDets)) {
      subDetIDsToAdd.insert(previousSubDets.begin(), previousSubDets.end());
    }

    // Attach the PEB Info to the decision
    std::vector<uint32_t> robVec(robIDsToAdd.begin(), robIDsToAdd.end());
    if (not decision->setDetail(PEBInfoWriterToolBase::robListKey(), robVec)) {
      return StatusCode::FAILURE;
    }
    std::vector<uint32_t> subDetVec(subDetIDsToAdd.begin(), subDetIDsToAdd.end());
    if (not decision->setDetail(PEBInfoWriterToolBase::subDetListKey(), subDetVec)) {
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }
  /// Fill a TrigCompositeContainer with random data
  void fillRandomData(xAOD::TrigCompositeContainer& data, unsigned int maxElements) {
    unsigned int nObjects = randomInteger<unsigned int>(0, maxElements);
    for (unsigned int iObj=0; iObj<nObjects; ++iObj) {
      xAOD::TrigComposite* object = new xAOD::TrigComposite;
      data.push_back(object);
      object->setName(std::string("object_")+std::to_string(iObj));
      unsigned int nAuxDataVec = randomInteger<unsigned int>(0, maxElements);
      for (unsigned int iAuxDataVec=0; iAuxDataVec<nAuxDataVec; ++iAuxDataVec) {
        xAOD::TrigComposite::Accessor<std::vector<float>> floatVec(std::string("floatVec_")+std::to_string(iAuxDataVec));
        unsigned int nValues = randomInteger<unsigned int>(0, maxElements);
        std::vector<float> values;
        for (unsigned int iValue=0; iValue<nValues; ++iValue) values.push_back( randomRealNumber<float>(0,1) );
        floatVec(*object) = values;
      }
    }
  }
}

// =============================================================================
// Standard constructor
// =============================================================================
MTCalibPebHypoTool::MTCalibPebHypoTool(const std::string& type, const std::string& name, const IInterface* parent)
: AthAlgTool(type,name,parent),
  m_decisionId (HLT::Identifier::fromToolName(name)) {}

// =============================================================================
// Standard destructor
// =============================================================================
MTCalibPebHypoTool::~MTCalibPebHypoTool() {}

// =============================================================================
// Implementation of AthAlgTool::initialize
// =============================================================================
StatusCode MTCalibPebHypoTool::initialize() {
  ATH_MSG_INFO("Initialising " << name());
  ATH_CHECK(m_robDataProviderSvc.retrieve());
  if (m_doCrunch) ATH_CHECK(m_cpuCrunchSvc.retrieve());

  // Copy keys from map<string,uint> to WriteHandleKeyArray
  std::transform(m_createRandomData.begin(),
                 m_createRandomData.end(),
                 std::back_inserter(m_randomDataWHK),
                 [](const auto& p){return p.first;});
  ATH_CHECK(m_randomDataWHK.initialize());

  // Parse and print the ROB request dictionary
  for (const auto& [instrString,robVec] : m_robAccessDictProp.value()) {
    m_robAccessDict.emplace_back(ROBRequestInstruction(instrString),robVec);
    if (m_robAccessDict.back().first.type==ROBRequestInstruction::Type::INVALID) {
      ATH_MSG_ERROR("Invalid instruction " << instrString);
      return StatusCode::FAILURE;
    }
  }
  if (msgLvl(MSG::DEBUG)) {
    if (!m_robAccessDict.empty()) {
      ATH_MSG_DEBUG(name() << " will execute the following ROB request instructions:");
      for (const auto& [instr,robVec] : m_robAccessDict) {
        ATH_MSG_DEBUG("---> Instruction : " << instr.toString());
        ATH_MSG_DEBUG("     ROB list    : " << idsToString(robVec));
      }
    }
    ATH_MSG_DEBUG(name() << " PEBROBList = [" << idsToString(m_pebRobList) << "]");
    ATH_MSG_DEBUG(name() << " PEBSubDetList = [" << idsToString(m_pebSubDetList) << "]");
  }

  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of AthAlgTool::finalize
// =============================================================================
StatusCode MTCalibPebHypoTool::finalize() {
  ATH_MSG_INFO("Finalising " << name());
  ATH_CHECK(m_robDataProviderSvc.release());
  ATH_CHECK(m_randomDataWHK.initialize());
  return StatusCode::SUCCESS;
}

// =============================================================================
StatusCode MTCalibPebHypoTool::decide(const MTCalibPebHypoTool::Input& input) const {
  // Re-seed the static thread-local RNG
  if (not m_useRandomSeed.value()) {
    const seed_t seed = eventSeed(input.eventContext.eventID(), name());
    ATH_MSG_DEBUG("Using seed " << seed << " for event " << input.eventContext.eventID());
    threadLocalGenerator().seed(seed);
  }

  // ---------------------------------------------------------------------------
  // Burn CPU time
  // ---------------------------------------------------------------------------
  for (unsigned int iCycle = 0; iCycle < m_numBurnCycles; ++iCycle) {
    if (Athena::Timeout::instance(input.eventContext).reached()) {
      ATH_MSG_ERROR("Timeout reached in CPU time burning cycle # " << iCycle+1);
      return Athena::Status::TIMEOUT;
    }
    unsigned int burnTime = m_burnTimeRandomly
                            ? randomInteger<unsigned int>(0, m_burnTimePerCycleMillisec)
                            : m_burnTimePerCycleMillisec.value();
    ATH_MSG_VERBOSE("CPU time burning cycle # " << iCycle+1 << ", burn time [ms] = " << burnTime);
    if (m_doCrunch) m_cpuCrunchSvc->crunch_for(std::chrono::milliseconds(burnTime));
    else std::this_thread::sleep_for(std::chrono::milliseconds(burnTime));
  }

  // ---------------------------------------------------------------------------
  // Prefetch or retrieve ROBs
  // ---------------------------------------------------------------------------
  for (const auto& [instr,robVec] : m_robAccessDict) {
    // Check for timeout
    if (Athena::Timeout::instance(input.eventContext).reached()) {
      ATH_MSG_ERROR("Timeout reached in ROB retrieval loop");
      return Athena::Status::TIMEOUT;
    }

    // Select a random sample of ROBs from the list, if needed
    ATH_MSG_DEBUG("Processing instruction " << instr.toString());
    std::vector<uint32_t> robs;
    if (instr.isRandom && instr.nRandom < robVec.size()) {
      std::sample(robVec.begin(),robVec.end(),
                  std::back_inserter(robs),
                  instr.nRandom,
                  threadLocalGenerator());
    }
    else robs = robVec;

    // Execute the ROB requests
    using ReqType = ROBRequestInstruction::Type;
    if (instr.type == ReqType::ADD || instr.type == ReqType::ADDGET) {
      // Prefetch ROBs
      ATH_MSG_DEBUG("Preloading ROBs: " << idsToString(robs));
      m_robDataProviderSvc->addROBData(input.eventContext, robs, name()+"-ADD");
    }
    if (instr.type == ReqType::GET || instr.type == ReqType::ADDGET) {
      // Retrieve ROBs
      ATH_MSG_DEBUG("Retrieving ROBs: " << idsToString(robs));
      // VROBFRAG is a typedef for std::vector<const eformat::ROBFragment<const uint32_t*>*>
      IROBDataProviderSvc::VROBFRAG robFragments;
      m_robDataProviderSvc->getROBData(input.eventContext, robs, robFragments, name()+"-GET");
      ATH_MSG_DEBUG("Number of ROBs retrieved: " << robFragments.size());
      if (!robFragments.empty()) {
        ATH_MSG_DEBUG("List of ROBs found: " << std::endl << format(robFragments));
      }
      if (m_checkDataConsistency.value()) {
        for (const IROBDataProviderSvc::ROBF* rob : robFragments) {
          try {
            if (!rob->check_rob() || !rob->check_rod()) {
              ATH_MSG_ERROR("Data consistency check failed");
            }
          }
          catch (const std::exception& ex) {
            ATH_MSG_ERROR("Data consistency check failed: " << ex.what());
          }
          ATH_MSG_DEBUG("Data consistency check passed for ROB 0x" << std::hex << rob->rob_source_id() << std::dec);
        }
      }
    }
    if (instr.type == ReqType::COL) {
      // Event building
      ATH_MSG_DEBUG("Requesting full event ROBs");
      int nrobs = m_robDataProviderSvc->collectCompleteEventData(input.eventContext, name()+"-COL");
      ATH_MSG_DEBUG("Number of ROBs retrieved: " << nrobs);
    }
    if (instr.type == ReqType::INVALID) {
      ATH_MSG_ERROR("Invalid ROB request instruction " << instr.toString());
      return StatusCode::FAILURE;
    }

    // Sleep between ROB requests
    std::this_thread::sleep_for(std::chrono::milliseconds(m_timeBetweenRobReqMillisec));
  }

  // ---------------------------------------------------------------------------
  // Produce random data
  // ---------------------------------------------------------------------------
  {
    using DataCont = xAOD::TrigCompositeContainer;
    using AuxCont = xAOD::TrigCompositeAuxContainer;
    for (const SG::WriteHandleKey<DataCont>& handleKey : m_randomDataWHK) {
      // Create data and aux container
      std::unique_ptr<DataCont> data = std::make_unique<DataCont>();
      std::unique_ptr<AuxCont> aux = std::make_unique<AuxCont>();
      data->setStore(aux.get());
      // Record the container in event store
      SG::WriteHandle<DataCont> handle(handleKey,input.eventContext);
      ATH_CHECK( handle.record(std::move(data),std::move(aux)) );
      ATH_MSG_DEBUG("Recorded TrigCompositeContainer " << handleKey.key() << " in event store");
      // Fill the container with random data
      unsigned int maxElements = m_createRandomData.value().at(handleKey.key());
      fillRandomData(*handle,maxElements);
    }
  }

  // ---------------------------------------------------------------------------
  // Random accept decision with PEB information
  // ---------------------------------------------------------------------------
  bool accept = randomAccept(m_acceptRate);
  if(!accept) {
    ATH_MSG_DEBUG("Decision " << m_decisionId << " is reject");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("Decision " << m_decisionId << " is accept");
  TrigCompositeUtils::addDecisionID(m_decisionId, input.decision);

  if (m_pebRobList.empty() && m_pebSubDetList.empty()) {
    ATH_MSG_VERBOSE("Not configured to write any PEBInfo, nothing will be attached to the decision");
  }
  else {
    ATH_MSG_DEBUG("Attaching ROBs=[" << idsToString(m_pebRobList) << "] and SubDets=["
                  << idsToString(m_pebSubDetList) << "] to the decision object");
    ATH_CHECK(appendPEBInfo(input.decision,m_pebRobList,m_pebSubDetList));
  }

  return StatusCode::SUCCESS;
}

// =============================================================================
MTCalibPebHypoTool::ROBRequestInstruction::ROBRequestInstruction(std::string_view strv) {
  // Work around a bug in clang 9.
#if __clang_major__ == 9
  std::string str (strv.begin(), strv.end());
#else
  const std::string_view& str = strv;
#endif
  if (str.find(":ADD:")!=std::string_view::npos) type = ROBRequestInstruction::ADD;
  else if (str.find(":GET:")!=std::string_view::npos) type = ROBRequestInstruction::GET;
  else if (str.find(":ADDGET:")!=std::string_view::npos) type = ROBRequestInstruction::ADDGET;
  else if (str.find(":COL:")!=std::string_view::npos) type = ROBRequestInstruction::COL;
  if (size_t pos=str.find(":RND"); pos!=std::string_view::npos) {
    size_t firstDigit=pos+4;
    size_t lastDigit=str.find_first_of(":",firstDigit);
    size_t num = std::stoul(str.substr(firstDigit,lastDigit).data());
    isRandom = true;
    nRandom = num;
  }
}

// =============================================================================
const std::string MTCalibPebHypoTool::ROBRequestInstruction::toString() const {
  std::string s;
  s += "type=";
  if (type==INVALID) s+="INVALID";
  else if (type==ADD) s+="ADD";
  else if (type==GET) s+="GET";
  else if (type==ADDGET) s+="ADDGET";
  else if (type==COL) s+="COL";
  s += ", isRandom=";
  s += isRandom ? "true" : "false";
  s += ", nRandom=";
  s += std::to_string(nRandom);
  return s;
}
