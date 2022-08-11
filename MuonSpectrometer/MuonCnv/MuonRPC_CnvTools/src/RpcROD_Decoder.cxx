/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcROD_Decoder.h"

#include <algorithm>
#include <deque>
#include <utility>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamData/RawEvent.h"
#include "eformat/Version.h"

Muon::RpcROD_Decoder::RpcROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type, name, parent) {
    declareProperty("SpecialROBNumber", m_specialROBNumber = -1);
    declareProperty("Sector13Data", m_sector13Data = false);
    declareProperty("DataErrorPrintLimit", m_maxprinterror = 1000);
}

StatusCode Muon::RpcROD_Decoder::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());

    ATH_CHECK(m_rpcReadKey.initialize());

    if (m_specialROBNumber > 0) { ATH_MSG_DEBUG("Setting the special ROB Number to: 0x" << MSG::hex << m_specialROBNumber << MSG::dec); }

    //==LBTAG initialize vector and variables for format failure check
    for (int i = 0; i < 13; i++) m_RPCcheckfail[i] = 0;
    m_printerror = 0;

    return StatusCode::SUCCESS;
}

StatusCode Muon::RpcROD_Decoder::finalize() {
    //==LBTAG print format failure final statistics
    printcheckformat();
    return StatusCode::SUCCESS;
}

#include "RpcROD_Decoder_v302.C"
