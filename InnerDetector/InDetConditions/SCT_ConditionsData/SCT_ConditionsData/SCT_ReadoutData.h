// -*- C++ -*-

/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_ReadoutData.h
 * Header file for the SCT_ReadoutData class
 * @author Susumu Oda <Susumu.Oda@cern.ch>
 */

#ifndef SCT_ConditionData_SCT_ReadoutData_h
#define SCT_ConditionData_SCT_ReadoutData_h

// Athena
#include "AthenaBaseComps/AthMessaging.h"
#include "SCT_ConditionsData/SCT_Chip.h"
#include "SCT_ConditionsData/SCT_PortMap.h"

// C++ STL
#include <bitset>
#include <memory>
#include <vector> 

class Identifier;

/** 
 * @enum ModuleType
 * @brief Enums for module and chip type
 **/
namespace SCT_Parameters {
  enum ModuleType {BARREL, MODIFIED_0, MODIFIED_1, ENDCAP};
}

/**
 * @class SCT_ReadoutTool
 * Class to represent the SCT module readout
 **/
class SCT_ReadoutData
  : public AthMessaging
{
public:

  SCT_ReadoutData(IMessageSvc* msgSvc=nullptr);
  ~SCT_ReadoutData() = default;

  /** No copy ctor*/
  SCT_ReadoutData(const SCT_ReadoutData&) = delete;
  /** No assignment operator */
  SCT_ReadoutData& operator=(const SCT_ReadoutData&) = delete;
  
  /** Find the ID of the input chip for chip*/ 
  SCT_Parameters::ChipType inputChip(const SCT_Chip& chip) const {
    return m_chipMap[chip.id()].input(chip.inPort());
  }

  /** Find the ID of the output chip for chip*/
  SCT_Parameters::ChipType outputChip(const SCT_Chip& chip) const {
    return m_chipMap[chip.id()].output(chip.outPort());
  }

  /** Set chip in readout and which link it is on*/
  void setChipIn(const SCT_Chip& chip, int link) {
    m_chipInReadout.set(chip.id());
    if (link==0) m_chipsOnLink0.push_back(chip.id());
    if (link==1) m_chipsOnLink1.push_back(chip.id());
  }

  /** Set chip out of readout and reset link*/
  void setChipOut(const SCT_Chip& chip) {
    m_chipInReadout.reset(chip.id());
  }

  /** Test if chip is in readout or not*/
  bool isChipReadOut(const SCT_Chip& chip) const {
    return m_chipInReadout.test(chip.id());
  }

  /** Set the module type */
  void setModuleType(const Identifier& moduleId, int bec);

  /** Fill the chip mapping*/
  void setChipMap();

  /** Check which chips are in the readout for a particular link and if the readout is sane*/
  void checkLink(int link);

  /** Chip has a correctly connected input*/
  bool hasConnectedInput(const SCT_Chip& chip) const;

  /** Chip is an end but is being talked to*/
  bool isEndBeingTalkedTo(const SCT_Chip& chip) const;

  /** Mask the chips that are not in the readout*/
  void maskChipsNotInReadout();

  /** Follow the readout to the input side*/
  bool followReadoutUpstream(int link, const SCT_Chip& chip, int remainingDepth = 12);

  /** is the readout for a particular link sane*/
  bool isLinkStandard(int link) const;

  /** Print readout status*/
  void printStatus(const Identifier& moduleId) const;

  /** Set SCT_Chip vectors */
  void setChips(std::vector<SCT_Chip>& chips);

  /** Set link status */
  void setLinkStatus(bool link0ok, bool link1ok);

  /** Set all chips out of readout and clear both links to start */
  void clearChipReadout();

private:

  /** Private data*/
  std::vector<SCT_Chip>*              m_chips{nullptr}; //!< Vector of actual SCT Chips for that module
  std::vector<SCT_PortMap>            m_chipMap{}; //!< Vector of port mapping from the chips in an SCT module 
  bool                                m_linkActive[2]{false, false}; //!< Links status for link 0 and 1
  std::bitset<SCT_Parameters::NChips> m_chipInReadout{}; //!< Bitset indicating whether a chip is readout or not
  SCT_Parameters::ModuleType          m_type{SCT_Parameters::BARREL}; //!< The type of this module (Barrel, Modified Barrel (0 or 1), Endcap)
  std::vector<int>                    m_chipsOnLink0{}; //!< The chips read out on link 0
  std::vector<int>                    m_chipsOnLink1{}; //! <The chips read out on link 1
};

#endif // SCT_ConditionData_SCT_ReadoutData_h
