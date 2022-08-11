/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RPC_DIGITIZATION_RPC_DIGITIZER_H
#define RPC_DIGITIZATION_RPC_DIGITIZER_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

class IPileUpTool;

class RPC_Digitizer : public AthAlgorithm {
public:
    /** Constructor with parameters */
    RPC_Digitizer(const std::string& name, ISvcLocator* pSvcLocator);

    /** Destructor */
    virtual ~RPC_Digitizer() = default;

    /** Basic algorithm methods */
    virtual StatusCode initialize() override final;
    virtual StatusCode execute() override final;
    virtual bool isClonable() const override final { return true; }

private:
    ToolHandle<IPileUpTool> m_digTool{this, "DigitizationTool", "RpcDigitizationTool", "RpcDigitizationTool name"};
};

#endif  // RPC_DIGITIZATION_RPC_DIGITIZER_H
