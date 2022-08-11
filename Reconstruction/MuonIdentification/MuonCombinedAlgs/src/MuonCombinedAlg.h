/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDALGS_MUONCOMBINEDALG_H
#define MUONCOMBINEDALGS_MUONCOMBINEDALG_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCombinedEvent/InDetCandidate.h"
#include "MuonCombinedEvent/InDetCandidateCollection.h"
#include "MuonCombinedEvent/InDetCandidateToTagMap.h"
#include "MuonCombinedEvent/MuonCandidate.h"
#include "MuonCombinedEvent/MuonCandidateCollection.h"
#include "MuonCombinedToolInterfaces/IMuonCombinedTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkTrack/TrackCollection.h"

class MuonCombinedAlg : public AthReentrantAlgorithm {
public:
    MuonCombinedAlg(const std::string& name, ISvcLocator* pSvcLocator);

    ~MuonCombinedAlg() = default;

    StatusCode execute(const EventContext& ctx) const override;
    StatusCode initialize() override;

private:
    ToolHandle<MuonCombined::IMuonCombinedTool> m_muonCombinedTool{this, "MuonCombinedTool",
                                                                   "MuonCombined::MuonCombinedTool/MuonCombinedTool", "muon combined tool"};
    SG::ReadHandleKey<InDetCandidateCollection> m_indetCandidateCollectionName{this, "InDetCandidateLocation", "InDetCandidates",
                                                                               "name of ID candidate collection"};
    SG::ReadHandleKey<MuonCandidateCollection> m_muonCandidateCollectionName{this, "MuonCandidateLocation", "MuonCandidates",
                                                                             "name of muon candidate collection"};
    SG::WriteHandleKeyArray<MuonCombined::InDetCandidateToTagMap> m_combTagMaps{
        this, "CombinedTagMaps", {"muidcoTagMap", "stacoTagMap"}, "combined muon tag maps"};
    SG::WriteHandleKey<TrackCollection> m_muidCombinedTracks{this, "MuidCombinedTracksLocation", "MuidCombinedTracks",
                                                             "Muidco combined Tracks"};
    SG::WriteHandleKey<TrackCollection> m_muidMETracks{this, "MuidMETracksLocation", "MuidMETracks", "Muidco ME Tracks"};
};

#endif
