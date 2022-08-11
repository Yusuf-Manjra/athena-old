/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTRDOTOMDTPREPDATA_H
#define MDTRDOTOMDTPREPDATA_H 

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "IRegionSelector/IRegSelTool.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"

class MdtIdHelper;
class MdtDigit;
class MDTcablingSvc;

#include <string>

////////////////////////////////////////////////////////////////////////////////////////
/// Author: Davide Costanzo
/// BNL, April 04, 2005
///
/// algorithm to decode RDO into MdtPrepData
/// get the RDO container from Storegate
/// loop over the RDO
/// Decode RDO into PrepRawData
/// loop over the PrepRawData and build the PrepRawData container
/// store the PrepRawData container in StoreGate
////////////////////////////////////////////////////////////////////////////////////////

class MdtRdoToMdtPrepData : public AthAlgorithm {

 public:

  MdtRdoToMdtPrepData(const std::string& name, ISvcLocator* pSvcLocator);
    
  StatusCode initialize();
  StatusCode execute();

 private:

    // EJWM - where is this implemented? Removing to avoid missing symbol
    //void printRpcPrepRawData(); //!< Prints information about the resultant PRDs.

    ToolHandle<Muon::IMuonRdoToPrepDataTool> m_tool{this,"DecodingTool","Muon::MdtRdoToPrepDataTool/MdtPrepDataProviderTool","mdt rdo to prep data conversion tool"};
    ToolHandle<IRegSelTool> m_regsel_mdt{this,"RegSel_MDT","RegSelTool/RegSelTool_MDT"};

    bool                                    m_print_inputRdo; //!<< If true, will dump information about the input RDOs.
    bool                                    m_print_prepData; //!<< If true, will dump information about the resulting PRDs.
    bool m_seededDecoding;
    SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey;
    SG::WriteHandleKey<Muon::MdtPrepDataContainer> m_mdtCollection;

};

#endif /// MDTRDOTOMDTPREPDATA_H


