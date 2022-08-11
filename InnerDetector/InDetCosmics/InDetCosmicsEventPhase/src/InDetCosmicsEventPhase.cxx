/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "InDetCosmicsEventPhase/InDetCosmicsEventPhase.h"

#include "TrkTrack/Track.h"
#include "TrkSegment/SegmentCollection.h"

#include "TrkTrack/TrackCollection.h"

#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"
#include "GaudiKernel/ListItem.h"

#include "TRT_ConditionsServices/ITRT_CalDbTool.h"
#include "TRT_ConditionsData/RtRelation.h"
#include "TRT_ConditionsData/BasicRtRelation.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "InDetIdentifier/TRT_ID.h"



/////////////////////////////////////////////////////////////////////////////

namespace InDet
{
  InDetCosmicsEventPhase::InDetCosmicsEventPhase(const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator),
    m_caldbtool("TRT_CalDbTool", this),
    m_eventPhaseTool() {
    declareProperty("TRTCalDbTool", m_caldbtool);
    declareProperty("TrackSummaryTool", m_trackSumTool);
    declareProperty("EventPhaseTool", m_eventPhaseTool);
  }

  StatusCode InDetCosmicsEventPhase::initialize() {
    ATH_MSG_INFO("initialize()");

    StatusCode sc = m_caldbtool.retrieve();
    if (sc.isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve TRT Calibration DB Service!");
      return sc;
    }

    ATH_CHECK(m_eventPhaseTool.retrieve());
    m_trackSumTool.disable(); // never used?

    ATH_CHECK(m_readKey_tracks.initialize());
    ATH_CHECK(m_writeKey_TRTPhase.initialize());

    return StatusCode::SUCCESS;
  }


  StatusCode InDetCosmicsEventPhase::execute() {

    m_phase = 0;

    const Trk::Track* selected = nullptr;
    int maxTRT = -1;

// Sasa Nov 30

    const TRT_ID* TRTHelper = nullptr;
    StatusCode sc = detStore()->retrieve(TRTHelper, "TRT_ID");
    if (sc.isFailure()) {
      msg(MSG::ERROR) << "Unable to retrieve TRT ID Helper." << endmsg;
      return sc;
    }

    std::vector<double> EPforTracksWithBarrelHits;

    for (SG::ReadHandleKey<TrackCollection>& k : m_readKey_tracks) {
      SG::ReadHandle<TrackCollection> MyTrackCollection(k);
      if (MyTrackCollection.isValid()) {
        const TrackCollection* trtbarreltracks = MyTrackCollection.cptr();
        TrackCollection::const_iterator trackIt = trtbarreltracks->begin();
        TrackCollection::const_iterator trackEnd = trtbarreltracks->end();
        for (; trackIt != trackEnd; ++trackIt) {
          int nTRTbarrelHits = 0;
          Trk::Track const* track = *trackIt;
          for (Trk::TrackStateOnSurface const* state : *track->trackStateOnSurfaces()) {
            Trk::MeasurementBase const* mesb = state->measurementOnTrack();
            if (!mesb || !state->type(Trk::TrackStateOnSurface::Measurement)) continue;
            InDet::TRT_DriftCircleOnTrack const* trtcirc = dynamic_cast<InDet::TRT_DriftCircleOnTrack const*>(mesb);
            if (!trtcirc) continue;
            Identifier const& id = trtcirc->identify();
            if (abs(TRTHelper->barrel_ec(id)) != 1) continue;
            nTRTbarrelHits++;
          }

          if (nTRTbarrelHits >= 10) { // require at least 10 barrel TRT hits
            double tmpEP = m_eventPhaseTool->findPhase(track);
            if (tmpEP != 0.) EPforTracksWithBarrelHits.push_back(tmpEP);
          }

          if (nTRTbarrelHits > maxTRT) {
            maxTRT = nTRTbarrelHits;
            selected = *trackIt; // track with max N of barrel hits
          }
        }
      }
    }

    m_phase = 0.;

    if (0 && selected) { // used to be default in cosmic to use only one track
      ATH_MSG_DEBUG("Using track with " << maxTRT << " TRT Hits");
      m_phase = m_eventPhaseTool->findPhase(selected);
    }

// Sasa Dec 2009: use average instead of result for one track
    if (EPforTracksWithBarrelHits.size()) {
      for (unsigned int i = 0; i < EPforTracksWithBarrelHits.size(); i++) m_phase += EPforTracksWithBarrelHits[i];
      m_phase /= (double) (EPforTracksWithBarrelHits.size());
    }

    ATH_MSG_DEBUG("Event Phase - " << m_phase);

    sc = storePhase();

    ATH_MSG_DEBUG("exit InDetCosmicsEventPhase");
    sc = StatusCode::SUCCESS;
    return sc;
  }

  StatusCode InDetCosmicsEventPhase::finalize() {
    ATH_MSG_INFO("finalize()");
    return StatusCode::SUCCESS;
  }

  StatusCode InDetCosmicsEventPhase::storePhase() {
    ATH_MSG_DEBUG("Recording phase...  " << m_phase);
    SG::WriteHandle<ComTime> writeTRTPhase(m_writeKey_TRTPhase);
    writeTRTPhase = std::make_unique<ComTime>(m_phase, m_phase);
    ATH_CHECK(writeTRTPhase.isValid());
    return StatusCode::SUCCESS;
  }
}
