/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCalibStandAloneBase/T0Refinement.h"

#include <fstream>
#include <iostream>

#include "MdtCalibInterfaces/IMdtSegmentFitter.h"
#include "MuonCalibMath/BaseFunctionFitter.h"
#include "MuonCalibMath/SimplePolynomial.h"
#include "MuonCalibStandAloneBase/NtupleStationId.h"

using namespace MuonCalib;

T0Refinement::T0Refinement() {
    m_time_out = 2.0;
    m_qfitter = std::make_unique<StraightPatRec>();
    m_qfitter->setRoadWidth(1.0);  // 1.0 mm road width
    m_qfitter->switchOnRefit();
    m_qfitter->setTimeOut(m_time_out);
    m_cfitter = std::make_unique<CurvedPatRec>();
    m_cfitter->setRoadWidth(1.0);  // 1.0 mm road width
    m_cfitter->setTimeOut(m_time_out);
    m_delta_t0 = 30.0;
}

double T0Refinement::getDeltaT0(MuonCalibSegment* segment, const IRtRelation* rt, bool overwrite, double& error, bool& failed,
                                bool curved) {
    ///////////////////////////////////////////////////
    // CHECK IF THERE ARE ENOUGH HITS ON THE SEGMENT //
    ///////////////////////////////////////////////////

    if (segment->mdtHitsOnTrack() < 3) { return 0.0; }

    ///////////////
    // VARIABLES //
    ///////////////

    double delta_t0_opt;                                // the best t0 correction
    MuonCalibSegment seg(*segment);                     // segment used internally
    NtupleStationId id((seg.mdtHOT())[0]->identify());  // station identifier
    double time;                                        // time and radius of a hit
    double sigma;                                       // sigma(r)
    BaseFunctionFitter fitter(3);
    SimplePolynomial pol;  // polynomial base functions x^k
    std::vector<SamplePoint> my_points(3);
    IMdtPatRecFitter* segment_fitter = nullptr;
    if (curved) {
        segment_fitter = m_cfitter.get();
    } else {
        segment_fitter = m_qfitter.get();
    }
    segment_fitter->SetRefineSegmentFlag(false);
    IMdtSegmentFitter::HitSelection r_selection(seg.mdtHitsOnTrack(), 0);

    ////////////////////////////////////////////////////////
    // DETERMINE THE CHI^2 VALUES FOR THREE DIFFERENT t0s //
    ////////////////////////////////////////////////////////

    // original drift times //
    m_qfitter->setFixSelection(false);
    unsigned int k = 0;
    for (const MuonCalibSegment::MdtHitPtr& mdt_hit : seg.mdtHOT()) {
        time = mdt_hit->driftTime();
        sigma = mdt_hit->sigmaDriftRadius();
        if (sigma > 10.0 && std::abs(mdt_hit->driftRadius()) > 14.0) r_selection[k] = 1;
        if (sigma > 10.0 && mdt_hit->driftTime() < 50.0) sigma = 0.3;
        mdt_hit->setDriftRadius(rt->radius(time), sigma);
        ++k;
    }
    MTStraightLine straight_track;
    CurvedLine curved_track;
    if (curved) {
        if (!m_cfitter->fit(seg, r_selection, curved_track)) {
            failed = true;
            return 0.0;
        }
    } else {
        if (!m_qfitter->fitCallByReference(seg, r_selection, straight_track)) {
            failed = true;
            return 0.0;
        }
    }

    my_points[0].set_x1(0.0);
    if (curved) {
        my_points[0].set_x2(curved_track.chi2PerDegreesOfFreedom());
    } else {
        my_points[0].set_x2(straight_track.chi2PerDegreesOfFreedom());
    }
    my_points[0].set_error(1.0);

    m_qfitter->setFixSelection(true);
    // shift drift times by +m_delta_t0 and refit //
    for (const MuonCalibSegment::MdtHitPtr& mdt_hit : seg.mdtHOT()) {
        time = mdt_hit->driftTime() + m_delta_t0;
        sigma = mdt_hit->sigmaDriftRadius();
        mdt_hit->setDriftRadius(rt->radius(time), sigma);
    }
    if (!segment_fitter->fit(seg, r_selection)) {
        failed = true;
        return 0.0;
    }

    my_points[1].set_x1(m_delta_t0);
    if (curved) {
        my_points[1].set_x2(curved_track.chi2PerDegreesOfFreedom());
    } else {
        my_points[1].set_x2(straight_track.chi2PerDegreesOfFreedom());
    }
    my_points[1].set_error(1.0);

    // shift drift times by -m_delta_t0 and refit //
    for (const MuonCalibSegment::MdtHitPtr& mdt_hit : seg.mdtHOT()) {
        if (my_points[1].x2() > my_points[0].x2()) {
            time = mdt_hit->driftTime() - m_delta_t0;
            my_points[2].set_x1(-m_delta_t0);
        } else {
            time = mdt_hit->driftTime() + 2.0 * m_delta_t0;
            my_points[2].set_x1(2.0 * m_delta_t0);
        }
        sigma = mdt_hit->sigmaDriftRadius();
        mdt_hit->setDriftRadius(rt->radius(time), sigma);
    }
    if (!segment_fitter->fit(seg, r_selection)) {
        failed = true;
        return 0.0;
    }
    if (curved) {
        my_points[2].set_x2(curved_track.chi2PerDegreesOfFreedom());
    } else {
        my_points[2].set_x2(straight_track.chi2PerDegreesOfFreedom());
    }
    my_points[2].set_error(1.0);

    // negative branch of T0 shifts
    if (my_points[1].x2() > my_points[0].x2() && my_points[2].x2() < my_points[0].x2()) {
        my_points[1].set_x1(my_points[2].x1());
        my_points[1].set_x2(my_points[2].x2());

        my_points[2].set_x1(-2.0 * m_delta_t0);

        for (const MuonCalibSegment::MdtHitPtr& mdt_hit : seg.mdtHOT()) {
            time = mdt_hit->driftTime() - 2.0 * m_delta_t0;
            sigma = mdt_hit->sigmaDriftRadius();
            mdt_hit->setDriftRadius(rt->radius(time), sigma);
        }
        if (!segment_fitter->fit(seg, r_selection)) {
            failed = true;
            return 0.0;
        }
        if (curved) {
            my_points[2].set_x2(curved_track.chi2PerDegreesOfFreedom());
        } else {
            my_points[2].set_x2(straight_track.chi2PerDegreesOfFreedom());
        }
        my_points[2].set_error(1.0);
    }

    ///////////////////////// further steps (negative branch):
    if (my_points[1].x1() < 0.0) {
        for (unsigned int l = 3; my_points[l - 1].x2() < my_points[l - 2].x2() && std::abs(my_points[l - 1].x1()) < 200.0; l++) {
            SamplePoint new_point;
            new_point.set_x1(my_points[l - 1].x1() - m_delta_t0);

            for (const MuonCalibSegment::MdtHitPtr& mdt_hit : seg.mdtHOT()) {
                time = mdt_hit->driftTime() + new_point.x1();
                sigma = mdt_hit->sigmaDriftRadius();
                mdt_hit->setDriftRadius(rt->radius(time), sigma);
            }

            if (!segment_fitter->fit(seg, r_selection)) {
                failed = true;
                return 0.0;
            }
            if (curved) {
                new_point.set_x2(curved_track.chi2PerDegreesOfFreedom());
            } else {
                new_point.set_x2(straight_track.chi2PerDegreesOfFreedom());
            }
            new_point.set_error(1.0);
            my_points.push_back(new_point);
        }
    }

    ///////////////////////// further steps (positive branch):
    if (my_points[2].x1() > 0.0) {
        for (unsigned int l = 3; my_points[l - 1].x2() <= my_points[l - 2].x2() && std::abs(my_points[l - 1].x1()) < 200.0; l++) {
            SamplePoint new_point;
            new_point.set_x1(my_points[l - 1].x1() + m_delta_t0);
            for (const MuonCalibSegment::MdtHitPtr& mdt_hit : seg.mdtHOT()) {
                time = mdt_hit->driftTime() + new_point.x1();
                sigma = mdt_hit->sigmaDriftRadius();
                mdt_hit->setDriftRadius(rt->radius(time), sigma);
            }
            if (!segment_fitter->fit(seg, r_selection)) {
                failed = true;
                return 0.0;
            }
            if (curved) {
                new_point.set_x2(curved_track.chi2PerDegreesOfFreedom());
            } else {
                new_point.set_x2(straight_track.chi2PerDegreesOfFreedom());
            }
            new_point.set_error(1.0);
            my_points.push_back(new_point);
        }
    }
    //////////////////////////////////////
    // CALCULATE THE BEST t0 CORRECTION //
    //////////////////////////////////////

    fitter.fit_parameters(my_points, my_points.size() - 2, my_points.size(), &pol);
    double nom(fitter.coefficients()[1]);
    double denom(fitter.coefficients()[2]);
    delta_t0_opt = -0.5 * nom / denom;
    error = std::sqrt(1.0 / denom);
    if (std::isnan(error)) {
        failed = true;
        return 0.0;
    }

    double direction{-0.5}, min_chi2{DBL_MAX};
    double best_t0 = delta_t0_opt;
    double current_t0 = delta_t0_opt;
    std::vector<double> t0s, chi2, mchi2;
    while (1) {
        for (const MuonCalibSegment::MdtHitPtr& mdt_hit : seg.mdtHOT()) {
            time = mdt_hit->driftTime() + current_t0;
            sigma = mdt_hit->sigmaDriftRadius();
            mdt_hit->setDriftRadius(rt->radius(time), sigma);
        }
        if (!segment_fitter->fit(seg, r_selection)) {
            failed = true;
            return 0.0;
        }
        double chisq(0.0);
        if (curved) {
            chisq = curved_track.chi2PerDegreesOfFreedom();
        } else {
            chisq = straight_track.chi2PerDegreesOfFreedom();
        }

        if (chisq < min_chi2) {
            min_chi2 = chisq;
            best_t0 = current_t0;
        } else {
            if (direction > 0) break;
            direction = 0.5;
            current_t0 = delta_t0_opt;
        }
        current_t0 += direction;
        t0s.push_back(current_t0);
        chi2.push_back(chisq);
        mchi2.push_back(min_chi2);
        if (t0s.size() > 100) {
            failed = true;
            return 0.0;
        }
    }

    /////////////////////////////////////////
    // OVERWRITE THE SEGMENT, IF REQUESTED //
    /////////////////////////////////////////
    delta_t0_opt = best_t0;
    failed = false;
    if (!overwrite) { return delta_t0_opt; }
    bool segment_t0_was_applied{false};
    for (const MuonCalibSegment::MdtHitPtr& mdt_hit : seg.mdtHOT()) {
        segment_t0_was_applied |= mdt_hit->segmentT0Applied();
        time = mdt_hit->driftTime() + delta_t0_opt;
        sigma = mdt_hit->sigmaDriftRadius();
        if (sigma > 10.0 && segment->mdtHOT()[k]->driftTime() < 50.0) sigma = 0.3;
        mdt_hit->setDriftTime(time);
        mdt_hit->setDriftRadius(rt->radius(time), sigma);
        mdt_hit->setSegmentT0Applied(true);
    }
    if (segment_t0_was_applied) {
        segment->setFittedT0(segment->fittedT0() - delta_t0_opt);
    } else {
        segment->setFittedT0(-delta_t0_opt);
    }

    segment_fitter->SetRefineSegmentFlag(true);
    if (segment_fitter->fit(*segment, r_selection) == 0.0) {
        failed = true;
        return 0.0;
    }

    if (segment->chi2() > 100.0) {
        failed = true;
        return 0.0;
    }

    return delta_t0_opt;
}
void T0Refinement::setTimeOut(const double& time_out) {
    m_time_out = time_out;
    return;
}
void T0Refinement::setRoadWidth(const double& road_width) {
    m_qfitter->setRoadWidth(road_width);
    m_cfitter->setRoadWidth(road_width);

    return;
}
