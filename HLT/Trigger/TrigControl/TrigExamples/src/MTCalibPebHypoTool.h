/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGEXPARTIALEB_MTCALIBPEBHYPOTOOL_H
#define TRIGEXPARTIALEB_MTCALIBPEBHYPOTOOL_H

// Trigger includes
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"

// Gaudi includes
#include "Gaudi/Parsers/Factory.h" // needed to declare less common Property types
#include "GaudiKernel/ICPUCrunchSvc.h"

/** @class MTCalibPebHypoTool
 *  @brief Base class for tools used by MTCalibPebHypoAlg
 **/
class MTCalibPebHypoTool : virtual public AthAlgTool {
public:
  /// Standard constructor
  MTCalibPebHypoTool(const std::string& type, const std::string& name, const IInterface* parent);
  /// Standard destructor
  virtual ~MTCalibPebHypoTool();

  // ------------------------- AthAlgTool methods ------------------------------
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  // ------------------------- Public types ------------------------------------
  struct Input {
    Input(TrigCompositeUtils::Decision* d, const EventContext& ctx)
    : decision(d), eventContext(ctx) {}
    TrigCompositeUtils::Decision* decision;
    const EventContext& eventContext;
  };

  // ------------------------- Specific methods of this class ------------------
  /// Decides whether to accept the event
  StatusCode decide(const Input& input) const;

private:
  // ------------------------- Types -------------------------------------------
  ///  ROB request instruction description
  struct ROBRequestInstruction {
    /// Constructor from string key in the ROBAccessDict property
    ROBRequestInstruction(std::string_view str);
    /// String form for debug print-outs
    const std::string toString() const;
    /// Type of instruction
    enum Type {INVALID, ADD, GET, ADDGET, COL} type = INVALID;
    /// Flag switching requests of a random sub-sample of the ROB list
    bool isRandom = false;
    /// Size of random request
    size_t nRandom = 0;
  };

  // ------------------------- Properties --------------------------------------
  Gaudi::Property<bool> m_useRandomSeed {
    this, "UseRandomSeed", false,
    "If true, use random seed for the internal RNG. If false, use a seed based on run/event number and tool name."
  };
  Gaudi::Property<double> m_acceptRate {
    this, "RandomAcceptRate", -1,
    "Rate of random accepts, <=0 is never, >=1 is always"
  };
  Gaudi::Property<unsigned int> m_burnTimePerCycleMillisec {
    this, "BurnTimePerCycleMillisec", 0,
    "Burn time per cycle in milliseconds"
  };
  Gaudi::Property<unsigned int> m_numBurnCycles {
    this, "NumBurnCycles", 0,
    "Number of time burning cycles"
  };
  Gaudi::Property<bool> m_burnTimeRandomly {
    this, "BurnTimeRandomly", true,
    "If true, burn time per cycle is a random value from uniform distribution between 0 and the given value"
  };
  Gaudi::Property<bool> m_doCrunch {
    this, "Crunch", false,
    "Crunch CPU instead of sleeping"
  };
  Gaudi::Property<bool> m_checkDataConsistency {
    this, "CheckDataConsistency", false,
    "Perform consistency checks for all retrieved ROB data"
  };
  Gaudi::Property<std::map<std::string,std::vector<uint32_t> > > m_robAccessDictProp {
    this, "ROBAccessDict", {},
    "Dictionary of prefetch/retrieve operations with given ROB IDs. The value is a vector of ROB IDs. "
    "The string key has to contain :ADD: (prefetch), :GET: (retrieve), :ADDGET: (prefetch+retrieve) or :COL: (full "
    "event building). :ADD:, :GET: and :ADDGET: may be also appended with :RNDX: where X is an integer. In this case, "
    "random X ROBs will be prefetched/retrieved from the provided list, e.g. :GET:RND10: retrieves 10 random ROBs from "
    "the list. Otherwise the full list is used. Note std::map is sorted by std::less<std::string>, so starting the key "
    "with a number may be needed to enforce ordering, e.g. '01 :ADD:RND10:'."
  };
  Gaudi::Property<unsigned int> m_timeBetweenRobReqMillisec {
    this, "TimeBetweenROBReqMillisec", 0,
    "Delay in milliseconds between subsequent ROB request operations from ROBAccessDict"
  };
  Gaudi::Property<std::set<uint32_t> > m_pebRobList {
    this, "PEBROBList", {},
    "The list of ROBs to add for partial event building in each passed event"
  };
  Gaudi::Property<std::set<uint32_t> > m_pebSubDetList {
    this, "PEBSubDetList", {},
    "The list of SubDets to add for partial event building in each passed event"
  };
  Gaudi::Property<std::map<std::string,unsigned int> > m_createRandomData {
    this, "CreateRandomData", {},
    "Create random data which can be serialised and saved to output. The property value should be a dictionary in the "
    "form {name: number} where name is the collection name and number is the maximum number of elements and "
    "their decorations"
  };

  // ------------------------- Service or tool handles -------------------------
  ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc{this, "ROBDataProviderSvc", "ROBDataProviderSvc", "Name of the ROB data provider"};
  ServiceHandle<ICPUCrunchSvc> m_cpuCrunchSvc{this, "CPUCrunchSvc", "CPUCrunchSvc", "Name of the CPU cruncher"};

  // ------------------------- Other private members ---------------------------
  /// The decision id of the tool instance
  HLT::Identifier m_decisionId;
  /// WriteHandleKey array for collections specified in the CreateRandomData property
  SG::WriteHandleKeyArray<xAOD::TrigCompositeContainer> m_randomDataWHK;
  /// Ordered map of ROB request instructions filled from ROBAccessDict property at initialisation
  std::vector<std::pair<ROBRequestInstruction,std::vector<uint32_t>>> m_robAccessDict;
};

#endif // TRIGEXPARTIALEB_MTCALIBPEBHYPOTOOL_H
