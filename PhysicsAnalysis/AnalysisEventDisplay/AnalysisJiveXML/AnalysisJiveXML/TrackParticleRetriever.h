/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_TRACKPARTICLERETRIEVER_H
#define JIVEXML_TRACKPARTICLERETRIEVER_H

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

//Forward declarations

namespace JiveXML{

  /**
   * @class TrackParticleRetriever
   * @brief Retrieves all @c TrackParticle (AOD) Collection objects
   *  This is a chopped version of the full (RDO/ESD) Trk::Track
   *  retriever TrigJiveXML/TrackRetriever
   *
   *  - @b Properties
   *    - <em> PriorityTrackCollection</em><tt> = 'TrackParticleCandidate' </tt>:  @copydoc m_PriorityTrackCollection
   *    - <em> OtherTrackCollections</em><tt> = [ ] </tt>: @copydoc m_OtherTrackCollections
   *    - <em> DoWriteHLT</em><tt> = False </tt>: @copydoc m_doWriteHLT
   *
   *  - @b Retrieved @b Data
   *    - @e id : internal track ID counter
   *    - @e chi2 : @f$\chi^2@f$ of track fit
   *    - @e numDoF : number of degrees of freedom for the fit
   *    - @e trackAuthor : track author as defined in TrkTrack/TrackInfo.h
   */

  class TrackParticleRetriever : virtual public IDataRetriever,
                                 public AthAlgTool {
   
    public:

      /// Standard Constructor
      TrackParticleRetriever(const std::string& type,const std::string& name,const IInterface* parent);
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return m_typeName; };

    private:
      ///The data type that is generated by this retriever
      const std::string m_typeName;

      //@name Property members
      //@{
      /// First track collections to retrieve, shown as default in Atlantis
      std::string m_PriorityTrackCollection;
      /// Track collections to retrieve in the sequence they are given, all if empty
      std::vector<std::string> m_OtherTrackCollections;
      /// Wether to write HLTAutoKey objects
      bool m_doWriteHLT;
      //@}
  };

}
#endif
