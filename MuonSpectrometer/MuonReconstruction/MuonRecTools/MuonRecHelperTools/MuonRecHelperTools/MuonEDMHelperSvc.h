/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MuonEDMHelperSvc_H
#define MUON_MuonEDMHelperSvc_H

#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthService.h"

#include "TrkParameters/TrackParameters.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include <set>
#include <string>

namespace Trk {
  class MeasurementBase;
  class Track;
  class IExtrapolator;
}

static const InterfaceID IID_MuonEDMHelperSvc("Muon::MuonEDMHelperSvc",1,0);

namespace Muon {
 
  class MuonSegment;

  /** 
      Helper tool containing functionality needed by multiple tools. 
  */
  class MuonEDMHelperSvc : public extends<AthService, IMuonEDMHelperSvc> {
  public:
    /** @brief default AlgService constructor */
    MuonEDMHelperSvc(const std::string& name, ISvcLocator* svc);

    /** @brief destructor */
    virtual ~MuonEDMHelperSvc() override {};
    
    /** @brief initialize method, method taken from bass-class AlgTool */
    virtual StatusCode initialize() override;

    /** @brief tries to get Identifier for measurement, if not possible it will return Identifier() */
    virtual Identifier getIdentifier( const Trk::MeasurementBase& meas ) const override;

    /** @brief returns the first MDT or CSC identifier of the segment */
    virtual Identifier chamberId( const MuonSegment& seg ) const override;

    /** @brief returns a set containing the chamber Ids of all MDT or CSC chambers on the segment */
    virtual std::set<Identifier> chamberIds( const MuonSegment& seg ) const override;

    /** @brief returns whether the segment is in the endcap */
    virtual bool isEndcap( const MuonSegment& seg ) const override;

    /** @brief returns whether the track is in the endcap */
    virtual bool isEndcap( const Trk::Track& seg ) const override;

    /** brief create a AtaPlane track parameter for segment */
    virtual const Trk::AtaPlane* createTrackParameters( const MuonSegment& seg, double momentum = 1., double charge = 0. ) const override;

    /** check whether track satisfies selection chi2 cut */
    virtual bool goodTrack( const Trk::Track& track, double chi2Cut ) const override;
    
    /** determines whether a track is a straight line track (no momentum fitted). */
    virtual bool isSLTrack( const Trk::Track& track ) const override;

  private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  };
  
}

#endif
