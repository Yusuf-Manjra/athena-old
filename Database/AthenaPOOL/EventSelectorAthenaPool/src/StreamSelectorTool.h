/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STREAMSELECTORTOOL_H
#define STREAMSELECTORTOOL_H

/** @file StreamSelectorTool.h
 *  @brief This file contains the class definition for the StreamSelectorTool class.
 **/

#include "AthenaKernel/IAthenaSelectorTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include <string>

/** @class StreamSelectorTool
 *  @brief This class provides an example for reading with a ISelectorTool to veto events on AttributeList.
 **/
class StreamSelectorTool : public AthAlgTool, virtual public IAthenaSelectorTool {
public: // Constructor and Destructor
   /// Standard Service Constructor
   StreamSelectorTool(const std::string& type, const std::string& name, const IInterface* parent);
   /// Destructor
   virtual ~StreamSelectorTool();

public:
   /// IAthenaSelectorTool Interface method implementations:
   virtual StatusCode initialize() override;
   virtual StatusCode postInitialize() override;
   virtual StatusCode preNext() const override;
   virtual StatusCode postNext() const override;
   virtual StatusCode preFinalize() override;
   virtual StatusCode finalize() override;
private:
   StringProperty m_attrListKey{this,"AttributeListKey","Input","Key for attribute list input to be used"};
   StringProperty m_streamName{this,"AcceptStreams","","Name of stream to be used as a ACCEPT"};
};

inline StatusCode StreamSelectorTool::initialize() {return StatusCode::SUCCESS;}
inline StatusCode StreamSelectorTool::postInitialize() {return StatusCode::SUCCESS;}
inline StatusCode StreamSelectorTool::preNext() const {return StatusCode::SUCCESS;}
inline StatusCode StreamSelectorTool::preFinalize() {return StatusCode::SUCCESS;}
inline StatusCode StreamSelectorTool::finalize() {return StatusCode::SUCCESS;}

#endif
