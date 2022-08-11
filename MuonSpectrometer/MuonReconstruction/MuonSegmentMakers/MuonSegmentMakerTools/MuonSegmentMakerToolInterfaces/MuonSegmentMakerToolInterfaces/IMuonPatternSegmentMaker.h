/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSEGMENTMAKETOOLINTERFACES_MUON_IMUONPATTERNSEGMENTMAKER_H
#define MUONSEGMENTMAKETOOLINTERFACES_MUON_IMUONPATTERNSEGMENTMAKER_H

#include <vector>

#include "GaudiKernel/IAlgTool.h"
#include "MuonEDM_AssociationObjects/MuonSegmentCombPatternCombAssociationMap.h"
#include "MuonPattern/MuonPatternCombinationCollection.h"
#include "MuonPrepRawData/RpcPrepDataCollection.h"
#include "MuonPrepRawData/TgcPrepDataCollection.h"
#include "MuonSegment/MuonSegmentCombinationCollection.h"
#include "TrkSegment/SegmentCollection.h"

namespace Trk {
    class Track;
}

namespace Muon {

    /** @brief The IMuonSegmentMaker is a pure virtual interface for tools to find tracks starting from MuonSegmentCombinations  */
    class IMuonPatternSegmentMaker : virtual public IAlgTool {
    public:
        /** access to tool interface */
        static const InterfaceID& interfaceID() {
            static const InterfaceID IID_IMuonPatternSegmentMaker("Muon::IMuonPatternSegmentMaker", 1, 0);
            return IID_IMuonPatternSegmentMaker;
        }

        /** @brief find tracks starting from a MuonSegmentCombination
            @param combi a reference to a MuonSegmentCombination
            @return a pointer to a vector of tracks, the ownership of the tracks is passed to the client calling the tool.
        */
        virtual void find(const MuonPatternCombination& pattern, const std::vector<const RpcPrepDataCollection*>& rpcCols,
                          const std::vector<const TgcPrepDataCollection*>& tgcCols, Trk::SegmentCollection* segColl) const = 0;

        /** @brief find tracks starting from a MuonSegmentCombinationCollection
            @param combiCol a reference to a MuonSegmentCombinationCollection
            @return a pointer to a vector of tracks, the ownership of the tracks is passed to the client calling the tool.
        */
        virtual std::unique_ptr<MuonSegmentCombinationCollection> find(const MuonPatternCombinationCollection* patterns,
                                                                       MuonSegmentCombPatternCombAssociationMap* segPattMap,
                                                                       const std::vector<const RpcPrepDataCollection*>& rpcCols,
                                                                       const std::vector<const TgcPrepDataCollection*>& tgcCols) const = 0;

        virtual ~IMuonPatternSegmentMaker() = default;
    };

}  // namespace Muon

#endif
