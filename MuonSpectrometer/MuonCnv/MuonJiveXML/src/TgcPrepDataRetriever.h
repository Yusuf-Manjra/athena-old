/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_TGCPREPDATARETRIEVER_H
#define JIVEXML_TGCPREPDATARETRIEVER_H

#include "JiveXML/IDataRetriever.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"

#include <string>

namespace JiveXML {
  
  class TgcPrepDataRetriever : virtual public IDataRetriever, public AthAlgTool {
    
  public:
    
    /// Standard Constructor
    TgcPrepDataRetriever(const std::string& type, const std::string& name, const IInterface* parent);

     /// Retrieve all the data
    virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool);

    /// Return the name of the data type
    virtual std::string dataTypeName() const { return "TGC"; };

    ///Default AthAlgTool methods
    StatusCode initialize();

  private:


    ///The storegate key for the TGC collection
    SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_sgKey{this, "StoreGateKey", "TGC_Measurements", "Name of the MMPrepDataContainer"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  };
  
}
#endif
