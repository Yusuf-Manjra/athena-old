/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_XAODMuonROIRETRIEVER_H
#define JIVEXML_XAODMuonROIRETRIEVER_H

#include <string>

#include "JiveXML/IDataRetriever.h"

#include "AthenaBaseComps/AthAlgTool.h"

namespace JiveXML{

  /**
   * @class MuonROIRetriever
   * @brief Retrieves ROI objects, default mode uses HLT/Offline object
   *        LVL1_ROI::emtaus_type
   *
   *  - @b Properties
   *    - <em>readCPM</em>: 'false' by default. If 'true' reads low-level ROI data
   *                        from L1Calo hardware (for ACR when reading SFI)
   *
   *  - @b Retrieved @b Data
   *    - <em>eta,phi</em>: coordinates of ROI
   *    - <em>energy</em>: energy in ROI
   *    - <em>roiWord, thrPattern</em>: trigger bitpatterns, undecoded
   *    .
   */
 
  class xAODMuonROIRetriever : virtual public IDataRetriever, public AthAlgTool {

  public:

    /// Standard Constructor
    xAODMuonROIRetriever(const std::string& type, const std::string& name, const IInterface* parent);

    /// Retrieve all the data
    virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool);

    /// Return the name of the data type
    virtual std::string dataTypeName() const { return m_typeName; };

  private:

   ///The data type that is generated by this retriever
    const std::string m_typeName;

    std::string m_sgKey;
  };

}
#endif
