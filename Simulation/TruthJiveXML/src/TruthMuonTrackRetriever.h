/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_TRUTHMUONTRACKRETRIEVER_H
#define JIVEXML_TRUTHMUONTRACKRETRIEVER_H

#include <string>

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

namespace HepPDT{
  class ParticleDataTable;
}


namespace JiveXML{

  /**
   * @class TruthMuonTrackRetriever
   * @brief Retrieves the muons information from the @c TrackRecordCollection
   *
   *  - @b Properties
   *    - <em>TrackRecCollNames</em><tt> = ["MuonEntryLayer","MuonEntryRecord"]</tt> : @copydoc TrackRecCollNames
   * 
   *  - @b Retrieved @b Data
   *    - <em>code</em> : the PDG ID of the particle
   *    - <em>id</em> : the particle barcode
   *    - <em>pt</em> : transverse momentum
   *    - <em>eta, phi</em> : @f$\eta@f$ and @f$\phi@f$ of the momentum vector
   *    - <em>rhoVertex,phiVertex,zVertex</em> : position of the production vertex in @f$\rho@f$, @f$\phi@f$ and @f$z@f$
   *    .
   *
   */

  class TruthMuonTrackRetriever : public extends<AthAlgTool, IDataRetriever> {

  public:
  
    /// Standard Constructor
    TruthMuonTrackRetriever(const std::string& type ,const std::string& name,const IInterface* parent);

    /// Retrieve all the data
    virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool);

    /// Return the name of the data type
    virtual std::string dataTypeName() const { return m_typeName; };

    ///Default AthAlgTool methods
    StatusCode initialize();

  private:
    ///The data type that is generated by this retriever
    const std::string m_typeName;

    /// A list of StoreGate names to probe in this order for the muon record collecton.
    std::vector<std::string> m_TrackRecCollNames;

  };

}
#endif

