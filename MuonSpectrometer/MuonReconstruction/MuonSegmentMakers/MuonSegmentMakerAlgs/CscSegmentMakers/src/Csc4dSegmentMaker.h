/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Csc4dSegmentMaker_H
#define Csc4dSegmentMaker_H

// Algorithm to find CSC 4D segments from clusters.
//
// Segmetns are witten as MuonSegment objects to a
// MuonSegmentCombinationCollection. There is a separate combination for
// each chamber (with segments) and separate r and phi collections
// for each combination.

#include <atomic>

#include "AthenaBaseComps/AthAlgTool.h"
#include "CscSegmentMakers/ICscSegmentFinder.h"  //including MuonSegmentCombination(Collection).h CscPrepDataContainer.h"
#include "GaudiKernel/ToolHandle.h"
#include "Identifier/Identifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

class ICscSegmentUtilTool;

namespace Track {
    class TrackRaod;
    class RIO_OnTrack;
}  // namespace Track

namespace Muon {
    class MdtDriftCircleOnTrack;
    class MuonClusterOnTrack;
    class MuonEDMPrinterTool;
    class MuonSegment;
}  // namespace Muon

class Csc4dSegmentMaker : virtual public ICscSegmentFinder, public AthAlgTool {
public:  // methods
    // Constructor.
    Csc4dSegmentMaker(const std::string&, const std::string&, const IInterface*);

    // Destructor.
    virtual ~Csc4dSegmentMaker();

    // Initialization.
    StatusCode initialize() override;

    std::unique_ptr<MuonSegmentCombinationCollection> find(const MuonSegmentCombinationCollection&, const EventContext& ctx) const override;
    std::unique_ptr<MuonSegmentCombinationCollection> find(const std::vector<const Muon::CscPrepDataCollection*>& pcols,
                                                           const EventContext& ctx) const override;  // not used here

private:  // data
    // Properties.
    // Number of events dumped.
    Gaudi::Property<double> m_max_chisquare{this, "max_chisquare", 25.};
    Gaudi::Property<double> m_max_slope_r{this, "max_slope_r", 0.2};
    Gaudi::Property<double> m_max_slope_phi{this, "max_slope_phi", 0.2};
    Gaudi::Property<double> m_max_seg_per_chamber{this, "max_seg_per_chamber", 50};

    ToolHandle<ICscSegmentUtilTool> m_segmentTool{
        this,
        "segmentTool",
        "CscSegmentUtilTool/CscSegmentUtilTool",
    };
    ToolHandle<Muon::MuonEDMPrinterTool> m_printer{
        this,
        "Printer",
        "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool",
    };
};

#endif
