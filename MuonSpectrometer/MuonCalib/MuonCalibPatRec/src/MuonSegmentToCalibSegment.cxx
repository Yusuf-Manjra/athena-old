/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibPatRec/MuonSegmentToCalibSegment.h"

#include <iostream>

#include "MdtCalibSvc/MdtCalibrationSvcInput.h"
#include "MdtCalibSvc/MdtCalibrationSvcSettings.h"
#include "MuonCalibEvent/MdtCalibHit.h"
#include "MuonCalibEventBase/CscCalibHitBase.h"
#include "MuonCalibEventBase/MdtCalibHitBase.h"
#include "MuonCalibEventBase/MuonCalibSegment.h"
#include "MuonPattern/MuonPatternChamberIntersect.h"
#include "MuonPattern/MuonPatternCombination.h"
#include "MuonRIO_OnTrack/CscClusterOnTrack.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/RpcClusterOnTrack.h"
#include "MuonRIO_OnTrack/TgcClusterOnTrack.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonSegment/MuonSegmentQuality.h"
#include "StoreGate/WriteHandle.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkParameters/TrackParameters.h"
namespace MuonCalib {

    MuonSegmentToCalibSegment::MuonSegmentToCalibSegment(const std::string& name, ISvcLocator* pSvcLocator) :
        AthAlgorithm(name, pSvcLocator) {}

    // Initialize
    StatusCode MuonSegmentToCalibSegment::initialize() {
        if (m_TrkSegKey.size() < m_segment_authors.size()) { m_segment_authors.value().resize(m_TrkSegKey.size(), -1); }

        std::string managerName = "Muon";
        ATH_CHECK(m_DetectorManagerKey.initialize());
        ATH_CHECK(m_idToFixedIdTool.retrieve());
        ATH_CHECK(m_assocTool.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        /// Read handle keys
        ATH_CHECK(m_patternKey.initialize());

        ATH_CHECK(m_CombSegKey.initialize());
        ATH_CHECK(m_CscSegKey.initialize(m_useCscSegments));
        ATH_CHECK(m_TrkSegKey.initialize());
        ATH_CHECK(m_patternKey.initialize());

        // Get the maximum number of segments each algorithm can
        // store in the ntuple
        /// Keep this commented cocde for now to remind ourselves that we need to set this property externally
        /// via JO
        ///  MuonSegmentNtupleBranch segBranch;
        /// m_maxStoredSegs = segBranch.MuonSegmentNtupleBranch::blockSize();

        if (m_TrkSegKey.size()) { m_maxStoredSegs.value() /= m_TrkSegKey.size(); }
        ATH_MSG_INFO("Maximum number of stored segments for each algorithm = " << m_maxStoredSegs);

        ATH_MSG_INFO("Initialisation ended     ");
        return StatusCode::SUCCESS;
    }

    StatusCode MuonSegmentToCalibSegment::execute() {
        ATH_MSG_DEBUG(" execute()     ");
        ATH_CHECK(convertPatterns(Gaudi::Hive::currentContext()));
        return StatusCode::SUCCESS;
    }

    StatusCode MuonSegmentToCalibSegment::savePatterns(const EventContext& ctx,
                                                       std::unique_ptr<MuonCalibPatternCollection> newPatterns) const {
        SG::WriteHandle writeHandle{m_patternKey, ctx};

        ATH_CHECK(writeHandle.record(std::move(newPatterns)));
        return StatusCode::SUCCESS;
    }

    StatusCode MuonSegmentToCalibSegment::convertPatterns(const EventContext& ctx) {
        /**
            Routine to extract mdt calibration segments from Mboy mdt segments
        */

        std::unique_ptr<MuonCalibPatternCollection> patterns = std::make_unique<MuonCalibPatternCollection>();

        ATH_MSG_DEBUG(" convertPatterns() ");

        SG::ReadCondHandle<MuonGM::MuonDetectorManager> DetectorManagerHandle{m_DetectorManagerKey, ctx};
        const MuonGM::MuonDetectorManager* MuonDetMgr = DetectorManagerHandle.cptr();
        if (!MuonDetMgr) {
            ATH_MSG_ERROR("Null pointer to the read MuonDetectorManager conditions object");
            return StatusCode::FAILURE;
        }

        if (!m_readSegments) {
            const MuonSegmentCombinationCollection* segCombis = nullptr;
            ATH_CHECK(retrieveContainer(ctx, m_CombSegKey, segCombis));

            MuonSegmentCombinationCollection::const_iterator sit = segCombis->begin();
            MuonSegmentCombinationCollection::const_iterator sit_end = segCombis->end();

            ATH_MSG_DEBUG(" Looping over segment combination " << segCombis->size());

            for (; sit != sit_end; ++sit) {
                // get association to pattern
                const Muon::MuonPatternCombination* pat = nullptr;
                Muon::IMuonPatternSegmentAssociationTool::AssociationMapRange range = m_assocTool->find(*sit);
                if ((range.first) != (range.second)) {
                    if (m_assocTool->count(*sit) != 1) {
                        ATH_MSG_INFO(" This MuonSegPatAssMap for MDTs should only have one entry!! ");
                    }  // retrieve association map:
                    pat = (range.first)->second;
                } else {
                    ATH_MSG_WARNING("MDT Combination missing from the map - something is wrong! " << *sit);
                }

                MuonCalibPattern* calibpat = createMuonCalibPattern(pat);

                ATH_MSG_DEBUG("New segment combination covering " << (*sit)->numberOfStations() << " station ");

                // loop over segments in combination
                unsigned int nstations = (*sit)->numberOfStations();
                for (unsigned int i = 0; i != nstations; ++i) {
                    Muon::MuonSegmentCombination::SegmentVec* stationSegs = (*sit)->stationSegments(i);

                    ATH_MSG_VERBOSE("New station with " << stationSegs->size() << " segments ");
                    for (const std::unique_ptr<Muon::MuonSegment>& seg : *stationSegs) {
                        if (!seg) {
                            ATH_MSG_WARNING(" go NULL pointer for MuonSegment ");
                            continue;
                        }

                        std::shared_ptr<MuonCalibSegment> mdtSeg{createMuonCalibSegment(*seg, MuonDetMgr)};
                        mdtSeg->setAuthor(seg->author());
                        calibpat->addMuonSegment(mdtSeg);
                    }
                }
                // add pattern to segment
                patterns->push_back(calibpat);
            }

        } else {
            // loop over MDT segment collections

            std::vector<int>::const_iterator autIt = m_segment_authors.begin();

            for (const SG::ReadHandleKey<Trk::SegmentCollection>& key : m_TrkSegKey) {
                int nStoredSegments = 0;

                const Trk::SegmentCollection* segCol = nullptr;
                if (!retrieveContainer(ctx, key, segCol).isSuccess()) { ++autIt; }
                ATH_MSG_DEBUG(" Looping over segments " << segCol->size());
                for (const Trk::Segment* seg_it : *segCol) {
                    if (nStoredSegments >= m_maxStoredSegs) {
                        ATH_MSG_INFO("For " << key.fullKey() << ", hit max number of segments = " << nStoredSegments);
                        break;
                    }

                    if (!seg_it) {
                        ATH_MSG_WARNING(" go NULL pointer for Segment ");
                        continue;
                    }

                    const Muon::MuonSegment* seg = dynamic_cast<const Muon::MuonSegment*>(seg_it);
                    if (!seg) {
                        ATH_MSG_WARNING(" dynamic_cast to MuonSegment failed ");
                        continue;
                    }

                    // one pattern per segment
                    ATH_MSG_DEBUG("WARNING, empty muoncalibpattern created");
                    std::unique_ptr<MuonCalibPattern> pat = std::make_unique<MuonCalibPattern>();

                    std::shared_ptr<MuonCalibSegment> mdtSeg{createMuonCalibSegment(*seg, MuonDetMgr)};
                    if (*autIt < 0)
                        mdtSeg->setAuthor(seg->author());
                    else
                        mdtSeg->setAuthor(*autIt);

                    pat->addMuonSegment(mdtSeg);

                    // add pattern to segment
                    patterns->push_back(std::move(pat));
                    ++nStoredSegments;
                }
                ++autIt;
            }
        }

        // retrieve CscSegmentCombis
        if (m_useCscSegments) {
            const MuonSegmentCombinationCollection* segCombis = nullptr;
            ATH_CHECK(retrieveContainer(ctx, m_CscSegKey, segCombis));
            MuonSegmentCombinationCollection::const_iterator sit = segCombis->begin();
            MuonSegmentCombinationCollection::const_iterator sit_end = segCombis->end();

            ATH_MSG_DEBUG(" Looping over Csc segment combination " << segCombis->size());

            for (; sit != sit_end; ++sit) {
                // don't create pattern when csc segment combination is empty (bug in csc segmentmaker?, 3-12-7, JS)
                if ((*sit)->numberOfStations() == 1) {
                    if ((*sit)->stationSegments(0)->size() == 0) continue;
                }

                // get association to pattern
                const Muon::MuonPatternCombination* pat = nullptr;
                Muon::IMuonPatternSegmentAssociationTool::AssociationMapRange range = m_assocTool->find(*sit);
                if ((range.first) != (range.second)) {
                    pat = (range.first)->second;
                } else {
                    ATH_MSG_DEBUG("CSC Combination missing from the map - No combined pattern found for this CSC Segment Combination! "
                                  << *sit);
                }

                MuonCalibPattern* calibpat = createMuonCalibPattern(pat);

                ATH_MSG_DEBUG("New Csc segment combination covering " << (*sit)->numberOfStations() << " station ");

                // loop over segments in combination
                unsigned int nstations = (*sit)->numberOfStations();
                for (unsigned int i = 0; i != nstations; ++i) {
                    Muon::MuonSegmentCombination::SegmentVec* stationSegs = (*sit)->stationSegments(i);

                    ATH_MSG_VERBOSE("New Csc station with " << stationSegs->size() << " segments ");

                    Muon::MuonSegmentCombination::SegmentVec::iterator segit = stationSegs->begin();
                    Muon::MuonSegmentCombination::SegmentVec::iterator segit_end = stationSegs->end();

                    for (; segit != segit_end; ++segit) {
                        Muon::MuonSegment* seg = (*segit).get();

                        if (!seg) {
                            ATH_MSG_WARNING(" go NULL pointer for MuonSegment ");
                            continue;
                        }

                        std::shared_ptr<MuonCalibSegment> CscSeg{createMuonCalibSegment(*seg, MuonDetMgr)};
                        CscSeg->setAuthor(seg->author());
                        calibpat->addMuonSegment(CscSeg);
                    }
                }

                // add pattern to segment
                patterns->push_back(calibpat);
            }
        }
        // store patterns in storegate
        ATH_CHECK(savePatterns(ctx, std::move(patterns)));
        return StatusCode::SUCCESS;
    }

    Identifier MuonSegmentToCalibSegment::getChId(const Muon::MuonSegment& seg) const {
        if (seg.numberOfContainedROTs() == 0) {
            ATH_MSG_DEBUG(" Oops, segment without hits!!! ");
            return Identifier();
        }

        for (unsigned int irot = 0; irot < seg.numberOfContainedROTs(); irot++) {
            // use pointer to rot
            const Trk::RIO_OnTrack* rot = seg.rioOnTrack(irot);

            if (m_idHelperSvc->isMdt(rot->identify())) {
                return rot->identify();
            } else if (m_idHelperSvc->isCsc(rot->identify())) {
                return rot->identify();
            }
        }

        // if we get here the segment did not contain any csc or mdt hits, in which case we return the identifier of the first rot
        return seg.rioOnTrack(0)->identify();
    }

    Amg::Transform3D MuonSegmentToCalibSegment::getGlobalToStation(const Identifier& id,
                                                                   const MuonGM::MuonDetectorManager* MuonDetMgr) const {
        if (m_idHelperSvc->isMdt(id)) {
            const MuonGM::MdtReadoutElement* detEl = MuonDetMgr->getMdtReadoutElement(id);
            if (!detEl) {
                ATH_MSG_WARNING("getGlobalToStation failed to retrieve detEL byebye");
            } else {
                return detEl->GlobalToAmdbLRSTransform();
            }
        } else if (m_idHelperSvc->isCsc(id)) {
            const MuonGM::CscReadoutElement* detEl = MuonDetMgr->getCscReadoutElement(id);
            if (!detEl) {
                ATH_MSG_WARNING("getGlobalToStation failed to retrieve detEL byebye");
            } else {
                return detEl->transform().inverse();
            }
        } else if (m_idHelperSvc->isTgc(id)) {
            const MuonGM::TgcReadoutElement* detEl = MuonDetMgr->getTgcReadoutElement(id);
            if (!detEl) {
                ATH_MSG_WARNING("getGlobalToStation failed to retrieve detEL byebye");
            } else {
                return detEl->transform().inverse();
            }
        } else if (m_idHelperSvc->isRpc(id)) {
            const MuonGM::RpcReadoutElement* detEl = MuonDetMgr->getRpcReadoutElement(id);
            if (!detEl) {
                ATH_MSG_WARNING("getGlobalToStation failed to retrieve detEL byebye");
            } else {
                return detEl->transform().inverse();
            }
        }
        ATH_MSG_WARNING(" Oops, should not be here, returning default transform ");
        return Amg::Transform3D();
    }
    MuonCalibSegment* MuonSegmentToCalibSegment::createMuonCalibSegment(const Muon::MuonSegment& seg,
                                                                        const MuonGM::MuonDetectorManager* MuonDetMgr) const {
        // convert MuonSegment to MuonCalibSegment

        Identifier chid = getChId(seg);

        // global to station transformation for this chamber
        Amg::Transform3D gToStationCheck = seg.associatedSurface().transform().inverse();
        Amg::Transform3D gToStation = getGlobalToStation(chid, MuonDetMgr);
        // create the local position and direction vector
        const Amg::Vector3D& segPosG(seg.globalPosition());
        const Amg::Vector3D& segDirG(seg.globalDirection());

        // calculate local position and direction of segment
        Amg::Vector3D segPosL = gToStation * segPosG;
        Amg::Vector3D segDirL = gToStation.linear() * segDirG;
        Amg::Vector3D segDirLCheck = gToStationCheck.linear() * segDirG;

        double qualityFactor(1e9);
        if (seg.fitQuality()) { qualityFactor = seg.fitQuality()->chiSquared(); }
        // get segment quality
        unsigned int segQuality = getQuality(seg);

        // pointer to new MuonSegment
        MuonCalibSegment* mdtSeg =
            new MuonCalibSegment(qualityFactor, segPosL, Amg::Vector3D(segDirL.unit()), gToStation.inverse(), segQuality);
        double t0Shift = 0.;
        if (seg.hasFittedT0()) { mdtSeg->setFittedT0(seg.time()); }

        Identifier cachedId;  // Cached Identifier of previous hit (invalid for now)

        double chi2check = 0;
        double thetap = std::atan2(mdtSeg->direction().y(), mdtSeg->direction().z());
        double cosin = std::cos(thetap);
        double sinus = std::sin(thetap);
        double thetan = std::atan2(mdtSeg->direction().z(), mdtSeg->direction().y());
        double thetaCheck = std::atan2(segDirLCheck[2], segDirLCheck[1]);
        ATH_MSG_DEBUG(" MuonSegment TO CalibSegment segment found ");
        if (msgLvl(MSG::DEBUG)) {
            if (std::abs(thetaCheck - thetan) > 0.0001) ATH_MSG_DEBUG(" ALARM angle difference " << thetaCheck - thetan);
            ATH_MSG_DEBUG(" segPosL " << segPosL << " segPosG " << segPosG << " local angle " << thetan << " thetaCheck " << thetaCheck);
            ATH_MSG_DEBUG(" segDirL " << segDirL << " segDirG " << segDirG << " phi " << segDirG.phi() << " segDirLCheck " << segDirLCheck);
        }
        bool segment_with_multiple_t0s(false);
        // for debug purposes count number of mdt,csc,tgc and rpc segments
        int nm(0), nr(0), nt(0), nc(0);

        // loop over hits
        const std::vector<const Trk::MeasurementBase*>& rots = seg.containedMeasurements();
        std::vector<const Trk::MeasurementBase*>::const_iterator rit = rots.begin();
        std::vector<const Trk::MeasurementBase*>::const_iterator rit_end = rots.end();
        for (; rit != rit_end; ++rit) {
            Identifier id;
            const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(*rit);
            const Trk::CompetingRIOsOnTrack* rotc = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(*rit);
            bool competingRio = false;
            if (rot) {
                // loop over Rios
                const Trk::PrepRawData* prd = rot->prepRawData();
                id = prd->identify();
            } else {
                // loop over Competing Rios
                if (rotc) {
                    id = rotc->rioOnTrack(0).identify();
                    competingRio = true;
                } else {
                    continue;
                }
            }

            if (m_idHelperSvc->isMdt(id)) {
                if (competingRio) {
                    ATH_MSG_WARNING("  MDT hit is competing Rio !!! ");
                    continue;
                }
                // Mdt digit
                ++nm;

                const Muon::MdtDriftCircleOnTrack* mrot = dynamic_cast<const Muon::MdtDriftCircleOnTrack*>(rot);
                if (!mrot) {
                    ATH_MSG_WARNING("This is not a  MdtDriftCircleOnTrack!!! ");
                    continue;
                }
                //      mdtSegment = true;

                // get digit from segment
                const Muon::MdtPrepData* prd = mrot->prepRawData();

                // digit identifier
                id = prd->identify();

                // get MdtDetectorElement for current digit
                const MuonGM::MdtReadoutElement* detEl = prd->detectorElement();

                // get tube geometry
                const Trk::StraightLineSurface* pStraightLineSurface =
                    dynamic_cast<const Trk::StraightLineSurface*>(&(detEl->surface(prd->identify())));
                if (!pStraightLineSurface) {
                    ATH_MSG_WARNING("This has no StraightLineSurface  !!! ");
                    continue;
                }

                // Prd has no second coordinate
                Amg::Vector3D tubePosLoc = gToStation * prd->globalPosition();

                // Get local tube direction, orient tube direction along the x local axis direction and get all DCA stuff in local
                // coordinates
                Amg::Vector3D tubeDirGlo = (pStraightLineSurface->transform()).rotation().col(2);
                Amg::Vector3D tubeDirLoc = gToStation.linear() * tubeDirGlo;
                ATH_MSG_DEBUG(" tubeDirLoc " << tubeDirLoc);
                tubeDirLoc = tubeDirLoc.unit();
                if (tubeDirLoc.x() < 0.) tubeDirLoc = -tubeDirLoc;

                Amg::Vector3D segPosLoc(mdtSeg->position());
                Amg::Vector3D segDirLoc(mdtSeg->direction());
                segDirLoc = segDirLoc.unit();

                Amg::Vector3D TubSegLoc(segPosLoc - tubePosLoc);

                Amg::Vector3D segDirLocprojected = segDirLoc - (tubeDirLoc.dot(segDirLoc)) * tubeDirLoc;
                segDirLocprojected = segDirLocprojected.unit();

                double ImpactParameter = tubeDirLoc.dot(TubSegLoc.cross(segDirLocprojected));

                double ScaleDenomi = 1. - std::pow(tubeDirLoc.dot(segDirLoc), 2);
                double ScaleOnTube = (tubeDirLoc.dot(TubSegLoc) - (segDirLoc.dot(TubSegLoc)) * (tubeDirLoc.dot(segDirLoc))) / ScaleDenomi;
                double ScaleOnSeg = (-segDirLoc.dot(TubSegLoc) + (tubeDirLoc.dot(TubSegLoc)) * (tubeDirLoc.dot(segDirLoc))) / ScaleDenomi;

                Amg::Vector3D tubePosLocAtDCA = tubePosLoc + ScaleOnTube * tubeDirLoc;
                Amg::Vector3D segPosLocAtDCA = segPosLoc + ScaleOnSeg * segDirLoc;

                Amg::Vector3D segPosAtDCA = gToStation.inverse() * segPosLocAtDCA;

                // global and local position of rot
                Amg::Vector3D trk_pos_rot(mrot->globalPosition());
                Amg::Vector3D trk_pos_loc_rot = gToStation * trk_pos_rot;

                double rtrk = cosin * (mdtSeg->position().y() - tubePosLoc.y()) - sinus * (mdtSeg->position().z() - tubePosLoc.z());

                // Recalculate Point of closest approach (taking local x from second measurments)
                double locx = trk_pos_loc_rot.x();
                double locy = tubePosLoc.y() + cosin * rtrk;
                double locz = tubePosLoc.z() - sinus * rtrk;
                Amg::Vector3D trk_pos_loc(locx, locy, locz);
                Amg::Vector3D trk_pos = gToStation.inverse() * trk_pos_loc;
                ATH_MSG_DEBUG("  trk_pos_loc_rot " << trk_pos_loc_rot << " tubePosLoc " << tubePosLoc << " trk_pos_loc " << trk_pos_loc
                                                   << "  trk_pos_rot " << trk_pos_rot);

                ATH_MSG_DEBUG(" standard rtrk " << rtrk << " ImpactParameter " << ImpactParameter << " diff rtrk " << rtrk - ImpactParameter
                                                << " trk_pos " << trk_pos << " OR segPosAtDCA " << segPosAtDCA);

                if (std::abs(rtrk - ImpactParameter) > 0.001)
                    ATH_MSG_DEBUG(" ALARM Impact parameter difference " << rtrk - ImpactParameter);

                // Alternative
                if (seg.author() == 3 || m_newImpactParameter) {
                    rtrk = ImpactParameter;
                    trk_pos = segPosAtDCA;
                }

                ATH_MSG_DEBUG("MDT RIO tdc " << prd->tdc() << " adc " << prd->adc() << " r_time "
                                             << rot->localParameters()[Trk::driftRadius] << " r_track " << rtrk);

                tubePosLoc[Trk::locX] = trk_pos_loc.x();

                // Alternative
                if (seg.author() == 3) { tubePosLoc = tubePosLocAtDCA; }

                double xLocTwin(-99999999.);

                // Store local twin tube coordinate
                if (prd->localPosition()[Trk::locY]) {
                    Identifier test_prd_Id = prd->detectorElement()->identify();
                    ATH_MSG_DEBUG(" Twin Position :  prd->localPosition()[Trk::locY] = "
                                  << prd->localPosition()[Trk::locY] << " in station "
                                  << m_idHelperSvc->mdtIdHelper().stationNameString(m_idHelperSvc->mdtIdHelper().stationName(test_prd_Id))
                                  << "  multilayer = " << m_idHelperSvc->mdtIdHelper().multilayer(test_prd_Id)
                                  << "  layer = " << m_idHelperSvc->mdtIdHelper().tubeLayer(test_prd_Id)
                                  << " tube = " << m_idHelperSvc->mdtIdHelper().tube(test_prd_Id)
                                  << "  modulo4 = " << (m_idHelperSvc->mdtIdHelper().tube(test_prd_Id) % 4));

                    Amg::Vector3D lposTrking(0., 0., prd->localPosition()[Trk::locY]);
                    Amg::Vector3D gposAMDB = detEl->surface(id).transform() * lposTrking;
                    Amg::Vector3D lposAMDB = detEl->GlobalToAmdbLRSTransform() * gposAMDB;
                    ATH_MSG_DEBUG(" CHECK lposTrking = " << lposTrking.z() << " lposAMDB " << lposAMDB.x());

                    xLocTwin = lposAMDB.x();
                }

                Amg::Vector3D tubePos = gToStation.inverse() * tubePosLoc;
                //      If the wire is rotated wrt segmentsurface we need this transform
                // get distance to readoud from detector manager
                double distRo_det = detEl->distanceFromRO(mrot->globalPosition(), id);

                // create new MdtCalibHit
                MdtCalibHit calibHit(id, prd->tdc(), prd->adc(), tubePos, tubePosLoc, detEl);
                calibHit.setGlobalPointOfClosestApproach(trk_pos);

                calibHit.setLocalPos(tubePosLoc);
                calibHit.setLocXtwin(xLocTwin);

                MdtCalibrationSvcSettings settings;
                // Copy settings from ROT
                // Window lower & upper bounds not set yet
                const Muon::MuonDriftCircleErrorStrategy& rotErrorStrategy = mrot->errorStrategy();
                settings.doTof = rotErrorStrategy.creationParameter(Muon::MuonDriftCircleErrorStrategy::TofCorrection);
                settings.doProp = rotErrorStrategy.creationParameter(Muon::MuonDriftCircleErrorStrategy::PropCorrection);
                settings.doTemp = rotErrorStrategy.creationParameter(Muon::MuonDriftCircleErrorStrategy::TempCorrection);
                settings.doField = rotErrorStrategy.creationParameter(Muon::MuonDriftCircleErrorStrategy::MagFieldCorrection);
                settings.doWireSag = rotErrorStrategy.creationParameter(Muon::MuonDriftCircleErrorStrategy::WireSagTimeCorrection);
                settings.doSlew = rotErrorStrategy.creationParameter(Muon::MuonDriftCircleErrorStrategy::SlewCorrection);
                settings.doBkg = rotErrorStrategy.creationParameter(Muon::MuonDriftCircleErrorStrategy::BackgroundCorrection);
                settings.windowSetting = rotErrorStrategy.calibWindow();
                settings.initialize();
                bool apply_t0 = ((m_updateForT0Shift < 0) ? rotErrorStrategy.creationParameter(Muon::MuonDriftCircleErrorStrategy::T0Refit)
                                                          : static_cast<bool>(m_updateForT0Shift)) &&
                                seg.hasFittedT0();
                if (apply_t0 && (t0Shift == 0.0)) { t0Shift = seg.time(); }
                MdtCalibrationSvcInput input;
                if (m_doTof) input.tof = calibHit.globalPointOfClosestApproach().mag() * (1. / 299.792458);
                input.trackDirection = &seg.globalDirection();

                input.pointOfClosestApproach = &calibHit.globalPointOfClosestApproach();
                bool sameChamber = false;
                if (cachedId.is_valid()) {
                    sameChamber = (m_idHelperSvc->mdtIdHelper().stationName(id) == m_idHelperSvc->mdtIdHelper().stationName(cachedId)) &&
                                  (m_idHelperSvc->mdtIdHelper().stationEta(id) == m_idHelperSvc->mdtIdHelper().stationEta(cachedId)) &&
                                  (m_idHelperSvc->mdtIdHelper().stationPhi(id) == m_idHelperSvc->mdtIdHelper().stationPhi(cachedId));
                }
                if (!sameChamber) ATH_MSG_DEBUG("Moving to a new chamber! " << cachedId << " to " << id);
                // We're done with the cached Id for now, so immediately reassign it
                cachedId = id;

                if (t0Shift == 0 || seg.author() != 3 || sameChamber) {
                    // There is one t0 shift for the whole segment - only one calibration is needed
                    input.tof += t0Shift;
                    ATH_MSG_DEBUG("Author " << seg.author() << " added single t0 shift of " << t0Shift);
                } else {
                    segment_with_multiple_t0s = true;
                    // We may be in a new chamber, with a different fitted t0
                    m_calibrationTool->driftRadiusFromTime(calibHit, input, settings);

                    // Reset the value of the t0 shift
                    t0Shift = calibHit.driftTime() - mrot->driftTime();
                    input.tof += t0Shift;
                    ATH_MSG_DEBUG("t0 shift updated to " << t0Shift);

                    if (std::abs(seg.time() - t0Shift) > 0.01 && std::abs(t0Shift) > 0.01) {
                        ATH_MSG_INFO(" Inconsistent fitted t0 found: from ROT " << t0Shift << " from segment " << seg.time());
                    }
                }

                // Calculate drift radius from drift time using MdtCalibrationSvc
                double oldDriftTime = calibHit.driftTime();  // 0 unless we shift t0
                m_calibrationTool->driftRadiusFromTime(calibHit, input, settings);

                double timeDif = calibHit.driftTime() - mrot->driftTime();

                // Store Sign of DriftRadius from Tracking (ROT) convention
                float driftR = calibHit.driftRadius();
                float sigmaDriftR = calibHit.sigmaDriftRadius();
                if (rot->localParameters()[Trk::driftRadius] < 0) {
                    driftR = -driftR;
                    calibHit.setDriftRadius(driftR, sigmaDriftR);
                }

                if (std::abs(timeDif) >= 0.1 && !segment_with_multiple_t0s) {
                    ATH_MSG_WARNING(" Bad T0Shift " << t0Shift << "  cor " << timeDif << " ROT " << mrot->driftRadius() << " t  "
                                                    << mrot->driftTime() << " calib " << calibHit.driftRadius() << " t "
                                                    << calibHit.driftTime() << " old " << oldDriftTime << " author " << seg.author());
                }
                if (std::abs(mrot->driftRadius() - calibHit.driftRadius()) > 0.01 && !segment_with_multiple_t0s) {
                    ATH_MSG_WARNING("Detected radius difference> 10 mu. MROT r= " << mrot->driftRadius()
                                                                                  << " calib r=" << calibHit.driftRadius());
                }
                ATH_MSG_DEBUG("B-field correction: " << calibHit.lorentzTime());

                // fill distance to track
                calibHit.setDistanceToTrack(rtrk, 0.);

                // set distance to readout
                calibHit.setDistanceToReadout(distRo_det);

                // convert MdtCalibHit to MdtCalibHitBase and then delete it
                // add hit to MuonSegment

                double resi = std::abs(driftR) - std::abs(rtrk);
                if (rtrk < 0) resi = -resi;
                double error2 = rot->localCovariance()(0, 0);
                double chi2c = (resi * resi) / error2;
                chi2check += chi2c;
                if (msgLvl(MSG::DEBUG) && seg.author() == 4) {
                    // Craig Blocker his checks
                    MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(id);
                    std::string st = fixid.stationNumberToFixedStationString(fixid.stationName());
                    int ml = fixid.mdtMultilayer();
                    int la = fixid.mdtTubeLayer();
                    ATH_MSG_DEBUG(" station " << st << " eta " << fixid.eta() << " phi " << fixid.phi() << " ML " << ml << " Layer " << la
                                              << " drift R " << driftR << " MROT drift R " << mrot->driftRadius() << " drift Time "
                                              << mrot->driftTime() << " ROT error " << std::sqrt(error2) << " residual " << resi
                                              << " tubePosLoc " << tubePosLoc << " t0 shift " << t0Shift << " chi2c " << chi2c);
                    if (std::sqrt(error2) < 1.999) ATH_MSG_DEBUG(" ALARM TOO SMALL drift error ");
                    if (chi2c > qualityFactor) ATH_MSG_DEBUG(" ALARM TOO LARGE chi2 single hit ");
                }
                MdtCalibHitBase* basehit = calibHit.hitBase(*m_idToFixedIdTool);
                basehit->setSegmentT0Applied(apply_t0);
                mdtSeg->addHitOnTrack(basehit);

            } else if (m_idHelperSvc->isRpc(id)) {
                // rpc ROT
                ++nr;

                int nRios = 1;

                if (competingRio) nRios = rotc->numberOfContainedROTs();
                for (int irio = 0; irio < nRios; ++irio) {
                    // Loop over competing Rios or Rios
                    if (competingRio) rot = &rotc->rioOnTrack(irio);

                    if (msgLvl(MSG::DEBUG)) {
                        if (!competingRio) ATH_MSG_DEBUG("Found RPC Rio !");
                        if (competingRio) ATH_MSG_DEBUG("Found RPC Competing Rio !");
                    }

                    const Muon::RpcClusterOnTrack* rrot = dynamic_cast<const Muon::RpcClusterOnTrack*>(rot);
                    if (!rrot) {
                        ATH_MSG_WARNING("This is not a  RpcClusterOnTrack!!! ");
                        continue;
                    }

                    const Muon::RpcPrepData* rprd = rrot->prepRawData();
                    id = rprd->identify();
                    int nStrips = rprd->rdoList().size();
                    // get detector element
                    const MuonGM::RpcReadoutElement* detEl = rprd->detectorElement();

                    double stripWidth = detEl->StripWidth(m_idHelperSvc->rpcIdHelper().measuresPhi(id));
                    double time = rprd->time();
                    double error = std::sqrt(rrot->localCovariance()(0, 0));
                    Amg::Vector3D rgp = rrot->globalPosition();

                    Amg::Vector3D rlp = gToStation * rgp;

                    // get strip lengths
                    double stripLen = detEl->StripLength(m_idHelperSvc->rpcIdHelper().measuresPhi(id));

                    double distRO;
                    if (m_idHelperSvc->rpcIdHelper().measuresPhi(id)) {
                        distRO = detEl->distanceToPhiReadout(rgp, id);
                    } else {
                        distRO = detEl->distanceToEtaReadout(rgp, id);
                    }

                    RpcCalibHitBase* rpcCH = new RpcCalibHitBase(nStrips, stripWidth, time, error, rgp, rlp);

                    MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(id);
                    rpcCH->setIdentifier(fixid);
                    rpcCH->setStripLength(stripLen);
                    rpcCH->setDistanceToRO(distRO);

                    mdtSeg->addHitOnTrack(rpcCH);
                }
            } else if (m_idHelperSvc->isTgc(id)) {
                ++nt;

                int nRios = 1;

                if (competingRio) nRios = rotc->numberOfContainedROTs();
                for (int irio = 0; irio < nRios; ++irio) {
                    // Loop over competing Rios or Rios
                    if (competingRio) rot = &rotc->rioOnTrack(irio);

                    if (msgLvl(MSG::DEBUG)) {
                        if (!competingRio) ATH_MSG_DEBUG("Found TGC Rio !");
                        if (competingRio) ATH_MSG_DEBUG("Found TGC Competing Rio !");
                    }

                    const Muon::TgcClusterOnTrack* trot = dynamic_cast<const Muon::TgcClusterOnTrack*>(rot);
                    if (!trot) {
                        ATH_MSG_WARNING("This is not a  TgcClusterOnTrack!!! ");
                        continue;
                    }

                    const Muon::TgcPrepData* tprd = trot->prepRawData();
                    id = tprd->identify();
                    ATH_MSG_DEBUG("TGC RIO ");

                    int nStrips = tprd->rdoList().size();

                    double stripWidth;
                    bool measuresPhi = (bool)m_idHelperSvc->tgcIdHelper().isStrip(tprd->identify());
                    int gasGap = m_idHelperSvc->tgcIdHelper().gasGap(tprd->identify());
                    int channel = m_idHelperSvc->tgcIdHelper().channel(tprd->identify());
                    const MuonGM::TgcReadoutElement* detEl = tprd->detectorElement();
                    if (!measuresPhi) {
                        stripWidth = detEl->gangMaxZ(gasGap, channel) - detEl->gangMinZ(gasGap, channel);
                    } else {
                        Amg::Vector3D localPos = detEl->transform(tprd->identify()).inverse() * detEl->channelPos(tprd->identify());
                        stripWidth = detEl->stripMaxX(gasGap, channel, localPos.y()) - detEl->stripMinX(gasGap, channel, localPos.y());
                    }

                    double error = std::sqrt(trot->localCovariance()(0, 0));
                    Amg::Vector3D tgp = trot->globalPosition();

                    Amg::Vector3D tlp = gToStation * tgp;
                    TgcCalibHitBase* tgcCH = new TgcCalibHitBase(nStrips, stripWidth, error, tgp, tlp);

                    MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(id);

                    tgcCH->setIdentifier(fixid);

                    mdtSeg->addHitOnTrack(tgcCH);
                }
            } else if (m_idHelperSvc->isCsc(id)) {
                ++nc;

                int nRios = 1;

                if (competingRio) nRios = rotc->numberOfContainedROTs();
                for (int irio = 0; irio < nRios; ++irio) {
                    // Loop over competing Rios or Rios
                    if (competingRio) rot = &rotc->rioOnTrack(irio);
                    ATH_MSG_DEBUG((competingRio ? "Found CSC Competing Rio !" : "Found CSC Rio !"));
                    const Muon::CscClusterOnTrack* crot = dynamic_cast<const Muon::CscClusterOnTrack*>(rot);
                    if (!crot) {
                        ATH_MSG_WARNING("This is not a  CscClusterOnTrack!!!");
                        continue;
                    }

                    const Muon::CscPrepData* cprd = crot->prepRawData();
                    Identifier id = cprd->identify();

                    int nStrips = cprd->rdoList().size();

                    int measuresPhi = m_idHelperSvc->cscIdHelper().measuresPhi(id);
                    int chamberLayer = m_idHelperSvc->cscIdHelper().chamberLayer(id);
                    double stripWidth = cprd->detectorElement()->cathodeReadoutPitch(chamberLayer, measuresPhi);
                    int charge = cprd->charge();

                    double error = std::sqrt(crot->localCovariance()(0, 0));
                    Amg::Vector3D cgp = crot->globalPosition();

                    Amg::Vector3D clp = gToStation * cgp;
                    CscCalibHitBase* cscCH = new CscCalibHitBase(nStrips, stripWidth, charge, error, cgp, clp);

                    MuonFixedId fixid = m_idToFixedIdTool->idToFixedId(id);

                    cscCH->setIdentifier(fixid);

                    mdtSeg->addHitOnTrack(cscCH);
                    ATH_MSG_DEBUG("mdtSeg->cscHitsOnTrack()=" << mdtSeg->cscHitsOnTrack());
                    ATH_MSG_DEBUG("mdtSeg->hitsOnTrack()=" << mdtSeg->hitsOnTrack());
                    // set the global to amdb transform in case of first hit
                }

            } else {
                ATH_MSG_WARNING("ERROR unknown RIO type ");
            }
        }
        ATH_MSG_VERBOSE("Number of *&* mdt " << nm << " rpc " << nr << " tgc " << nt << " csc " << nc);

        // add magnetic-field entries for MDT hits //

        if (msgLvl(MSG::DEBUG) && seg.author() == 4) {
            if (qualityFactor > 0.0001) {
                if (chi2check / qualityFactor > 1.01 || chi2check / qualityFactor < 0.99) {
                    ATH_MSG_DEBUG(" ALARM wrong chi2 "
                                  << "Mdt segment chi2 " << qualityFactor << " mdt hits " << nm << " chi2check " << chi2check << " t0Shift "
                                  << t0Shift);
                } else {
                    ATH_MSG_DEBUG(" good chi2 "
                                  << "Mdt segment chi2 " << qualityFactor << " mdt hits " << nm << " chi2check " << chi2check << " t0Shift "
                                  << t0Shift);
                }
            } else {
                ATH_MSG_DEBUG(" good chi2 "
                              << "Mdt segment chi2 " << qualityFactor << " mdt hits " << nm << " chi2check " << chi2check << " t0Shift "
                              << t0Shift);
            }
        }

        return mdtSeg;
    }  // createMuonCalibSegment

    MuonCalibPattern* MuonSegmentToCalibSegment::createMuonCalibPattern(const Muon::MuonPatternCombination* pat) const {
        ATH_MSG_VERBOSE("createMuonCalibPattern");
        MuonCalibPattern* calibpat = nullptr;
        MuonCalibPattern::defineParams pars{};
        if (pat) {
            const Trk::TrackParameters* trkparameters = pat->trackParameter();
            const Trk::Perigee* perigee = dynamic_cast<const Trk::Perigee*>(trkparameters);
            if (perigee) {
                const AmgVector(5) parameters = perigee->parameters();
                pars.phi = parameters[Trk::phi];
                pars.theta = parameters[Trk::theta];
                pars.dist0 = -perigee->position().y() * std::cos(pars.phi) + perigee->position().x() * std::sin(pars.phi);
                double charge = pars.dist0 > 0 ? 1. : -1;

                // Approximate conversion of radius of curvature to Pinv in MeV-1
                pars.invP = (10. * charge) / (perigee->momentum().mag());
                pars.z0 = perigee->position().z();
                ATH_MSG_DEBUG(" r0,z0 " << pars.dist0 << " " << pars.z0 << " phi,theta " << pars.phi << " " << pars.theta);
                ATH_MSG_DEBUG(" pat " << perigee->position() << " " << perigee->momentum());
            } else {
                ATH_MSG_WARNING("Trackparameters are not set or is not a Perigee!! Pattern gets empty parameters");
            }

            // Here I have to add the nmdt, nrpc, ntgc and ncsc...
            const std::vector<Muon::MuonPatternChamberIntersect>& mpcivec = pat->chamberData();
            std::vector<Muon::MuonPatternChamberIntersect>::const_iterator pat_it = mpcivec.begin();
            for (; pat_it != mpcivec.end(); ++pat_it) {
                const std::vector<const Trk::PrepRawData*> prdvec = (*pat_it).prepRawDataVec();
                std::vector<const Trk::PrepRawData*>::const_iterator prd_it = prdvec.begin();
                for (; prd_it != prdvec.end(); ++prd_it) {
                    Identifier id = (*prd_it)->identify();
                    if (m_idHelperSvc->isMdt(id)) {
                        pars.nmdt += 1000;  // a mdt is always an eta-hit.
                    } else if (m_idHelperSvc->isRpc(id)) {
                        if (m_idHelperSvc->rpcIdHelper().measuresPhi(id))
                            pars.nrpc += 1;
                        else
                            pars.nrpc += 1000;
                    } else if (m_idHelperSvc->isTgc(id)) {
                        if (m_idHelperSvc->tgcIdHelper().isStrip(id))
                            pars.ntgc += 1;
                        else
                            pars.ntgc += 1000;
                    } else if (m_idHelperSvc->isCsc(id)) {
                        if (m_idHelperSvc->cscIdHelper().measuresPhi(id))
                            pars.ncsc += 1;
                        else
                            pars.ncsc += 1000;
                    } else
                        ATH_MSG_INFO("PrepRawData on pat is not a muon-technom_logy");
                }
            }
            calibpat = new MuonCalibPattern(pars);
        } else {
            ATH_MSG_DEBUG("WARNING, empty muoncalibpattern created");
            calibpat = new MuonCalibPattern();
        }
        return calibpat;
    }

    unsigned int MuonSegmentToCalibSegment::getQuality(const Muon::MuonSegment& seg) const {
        ATH_MSG_DEBUG(" plotting quality ");
        // try to dynamic_cast to MdtSegmentQuality in order to obtain quality

        const Muon::MuonSegmentQuality* q = dynamic_cast<const Muon::MuonSegmentQuality*>(seg.fitQuality());

        if (!q) {
            //    NO quality available for CSC
            return 0;
        }
        ATH_MSG_DEBUG("Got MuonSegmentQuality "
                      << " hots " << q->numberDoF() + 2 << " number of holes " << q->channelsWithoutHit().size());

        unsigned int packedInfo = 0;

        packedInfo += 100 * (q->channelsWithoutHit().size() < 9 ? q->channelsWithoutHit().size() : 9);

        ATH_MSG_DEBUG(" packedInfo " << packedInfo);
        return packedInfo;
    }

    template <class container_type>
    StatusCode MuonSegmentToCalibSegment::retrieveContainer(const EventContext& ctx, const SG::ReadHandleKey<container_type>& key,
                                                            const container_type*& container_ptr) const {
        SG::ReadHandle<container_type> readHandle{key, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_WARNING("Failed to retrieve " << key.fullKey());
            return StatusCode::FAILURE;
        }
        container_ptr = readHandle.cptr();

        const container_type* container{readHandle.cptr()};
        if (!container) {
            ATH_MSG_WARNING("Failed to retrieve " << key.fullKey());
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }

}  // namespace MuonCalib
