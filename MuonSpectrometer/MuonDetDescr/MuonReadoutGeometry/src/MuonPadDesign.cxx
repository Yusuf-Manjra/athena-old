/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonReadoutGeometry/MuonPadDesign.h"

#include <TString.h>  // for Form

#include <ext/alloc_traits.h>
#include <stdexcept>

using MuonGM::MuonPadDesign;

bool MuonPadDesign::withinSensitiveArea(const Amg::Vector2D& pos) const {
    double top_H1 = maxSensitiveY();
    double bot_H2 = minSensitiveY();
    double max_x = maxAbsSensitiveX(pos.y());
    bool y_in_range = (pos.y() <= top_H1 and pos.y() >= bot_H2);
    bool x_in_range = std::abs(pos.x()) <= max_x + 0.01;
    return y_in_range and x_in_range;
}
//----------------------------------------------------------
double MuonPadDesign::minSensitiveY() const { return yCutout ? -(Size - yCutout) : -0.5 * Size; }
//----------------------------------------------------------
double MuonPadDesign::maxSensitiveY() const { return yCutout ? yCutout : 0.5 * Size; }
//----------------------------------------------------------
double MuonPadDesign::maxAbsSensitiveX(const double& y) const {
    double half_openingAngle = sectorOpeningAngle / 2.0;
    if (isLargeSector && yCutout) {  // if QL3
        if (y > 0)                   // In cutout region
            return 0.5 * lPadWidth;
        else
            return y * tan(M_PI * half_openingAngle / 180.) + 0.5 * lPadWidth;
    } else
        return (y - Size * 0.5) * tan(M_PI * half_openingAngle / 180.) + 0.5 * lPadWidth;

    return -1;
}
//----------------------------------------------------------
std::pair<int, int> MuonPadDesign::channelNumber(const Amg::Vector2D& pos) const {
    /* Changes in this package are due to new geometry implementations
     * Correct active area position and inclusion of proper QL3 shape.
     * coordinates (0,0) now point to the center of the active region, not gas volume
     * for QL3, where ycutout !=0, (0,0) is at start of ycutout */

    // perform check of the sensitive area
    std::pair<int, int> result(-1, -1);

    // padEta
    double y1 = yCutout ? Size - yCutout + pos.y() : 0.5 * Size + pos.y();  // distance from small edge to hit
    double padEtadouble;
    int padEta = 0;
    // padPhi
    // DT-2015-11-29 : currently easier: attribute 'phi pad fuzzy shift' to hit rather than to pad edge
    double locPhi = 180 * std::atan(-1.0 * pos.x() / (radialDistance + pos.y())) / M_PI;
    double maxlocPhi = 180 * std::atan(maxAbsSensitiveX(pos.y()) / (radialDistance + pos.y())) / M_PI;
    // fuzziness for negative z takes negative of PadPhiShift
    double fuzziedX = pos.x() - (-1.0 * PadPhiShift / std::cos(locPhi * M_PI / 180));
    double fuzziedlocPhi = 180 * std::atan(-1.0 * fuzziedX / (radialDistance + pos.y())) / M_PI;

    bool below_half_length = (y1 < 0);
    bool outside_phi_range = (std::abs(locPhi) > maxlocPhi) or (std::abs(fuzziedlocPhi) > maxlocPhi);

    if (withinSensitiveArea(pos) and not below_half_length) {
        if (y1 > firstRowPos) {
            //+1 for firstRow, +1 because a remainder means another row (3.1=4)
            padEtadouble = ((y1 - firstRowPos) / inputRowPitch) + 1 + 1;
            padEta = padEtadouble;
        } else if (y1 > 0) {
            padEta = 1;
        }
        double padPhidouble;
        // These are separated as the hits on the pads closest to the side edges are not fuzzied
        // We must do a correction in order to stay consistent with indexing
        if (outside_phi_range)
            padPhidouble = (locPhi - firstPhiPos) / inputPhiPitch;
        else  // Look for the index of the fuzzied hit
            padPhidouble = (fuzziedlocPhi - firstPhiPos) / inputPhiPitch;
        int padPhi = padPhidouble + 2;  //(+1 because remainder means next column e.g. 1.1=2, +1 so rightmostcolumn=1)

        // adjust indices if necessary
        if (padEta == nPadH + 1) { padEta -= 1; }                 // the top row can be bigger, therefore it is really in the nPadH row.
        if (padPhi == 0) { padPhi = 1; }                          // adjust rightmost
        if (padPhi == nPadColumns + 1) { padPhi = nPadColumns; }  // adjust lefmost

        // final check on range
        bool ieta_out_of_range = (padEta > nPadH + 1);
        bool iphi_out_of_range = (padPhi < 0 || padPhi > nPadColumns + 1);
        bool index_out_of_range = ieta_out_of_range or iphi_out_of_range;
        if (index_out_of_range) {
            if (ieta_out_of_range)
                throw std::runtime_error(
                    Form("File: %s, Line: %d\nMuonPadDesign::channelNumber() - eta out of range (x,y)=(%.2f, %.2f) (ieta, iphi)=(%d, %d)",
                         __FILE__, __LINE__, pos.x(), pos.y(), padEta, padPhi));
            else
                throw std::runtime_error(
                    Form("File: %s, Line: %d\nMuonPadDesign::channelNumber() - phi out of range (x,y)=(%.2f, %.2f) (ieta, iphi)=(%d, %d)",
                         __FILE__, __LINE__, pos.x(), pos.y(), padEta, padPhi));
        } else {
            result = std::make_pair(padEta, padPhi);
        }
    }
    return result;
}

//----------------------------------------------------------
bool MuonPadDesign::channelPosition(std::pair<int, int> pad, Amg::Vector2D& pos) const {
    std::vector<Amg::Vector2D> corners;
    channelCorners(pad, corners);
    // double yCenter = 0.5*(corners.at(0)[1]+corners.at(3)[1]);
    double yCenter = 0.5 * (0.5 * (corners.at(0)[1] + corners.at(1)[1]) + 0.5 * (corners.at(2)[1] + corners.at(3)[1]));
    double xCenter = 0.5 * (0.5 * (corners.at(0)[0] + corners.at(1)[0]) + 0.5 * (corners.at(2)[0] + corners.at(3)[0]));
    pos[0] = xCenter;
    pos[1] = yCenter;
    return true;
}
//----------------------------------------------------------
bool MuonPadDesign::channelCorners(std::pair<int, int> pad, std::vector<Amg::Vector2D>& corners) const {
    // DG-2015-11-30: todo check whether the offset subtraction is still needed
    int iEta = pad.first;   // -1 + padEtaMin;
    int iPhi = pad.second;  //  -1 + padPhiMin;
    // bool invalid_indices = iEta<1 || iPhi<1; // DG-2015-11-30 do we still need to check this?
    // if(invalid_indices) return false;
    // double yBot = -0.5*Length + firstRowPos + ysFrame + iEta*inputRowPitch;
    // double yTop = -0.5*Length + firstRowPos + ysFrame + (iEta+1)*inputRowPitch;

    ////// ASM-2015-12-07 : New Implementation
    double yBot = 0., yTop = 0.;
    if (iEta == 1) {
        yBot = yCutout ? -(Size - yCutout) : -0.5 * Size;
        yTop = yBot + firstRowPos;
    } else if (iEta > 1) {
        yBot =
            yCutout ? -(Size - yCutout) + firstRowPos + (iEta - 2) * inputRowPitch : -0.5 * Size + firstRowPos + (iEta - 2) * inputRowPitch;
        yTop = yBot + inputRowPitch;
        if (iEta == nPadH) yTop = maxSensitiveY();
    } else {  // Unkwown ieta
        return false;
    }
    ////// ASM-2015-12-07 : New Implementation

    // restrict y to the module sensitive area
    double minY = minSensitiveY();
    double maxY = maxSensitiveY();
    if (yBot < minY) yBot = minY;
    if (yTop > maxY) yTop = maxY;

    // here L/R are defined as if you were looking from the IP to the
    // detector (same a clockwise/counterclockwise phi but shorter)
    double phiRight = firstPhiPos + (iPhi - 2) * inputPhiPitch;
    double phiLeft = firstPhiPos + (iPhi - 1) * inputPhiPitch;
    double xBotRight = -(yBot + radialDistance) * tan(M_PI * phiRight / 180.) + PadPhiShift;
    double xBotLeft = -(yBot + radialDistance) * tan(M_PI * phiLeft / 180.) + PadPhiShift;
    double xTopRight = -(yTop + radialDistance) * tan(M_PI * phiRight / 180.) + PadPhiShift;
    double xTopLeft = -(yTop + radialDistance) * tan(M_PI * phiLeft / 180.) + PadPhiShift;

    // Adjust outer columns
    if (iPhi == 1) {
        double yLength = yCutout ? Size - yCutout : Size;
        xBotRight = 0.5 * (sPadWidth + (lPadWidth - sPadWidth) * (yBot - minY) / yLength);
        xTopRight = 0.5 * (sPadWidth + (lPadWidth - sPadWidth) * (yTop - minY) / yLength);
    }
    if (iPhi == nPadColumns) {
        double yLength = yCutout ? Size - yCutout : Size;
        xBotLeft = -0.5 * (sPadWidth + (lPadWidth - sPadWidth) * (yBot - minY) / yLength);
        xTopLeft = -0.5 * (sPadWidth + (lPadWidth - sPadWidth) * (yTop - minY) / yLength);
    }

    // Adjust for cutout region
    if (yCutout && yTop > 0) {
        float cutoutXpos = 0.5 * lPadWidth;
        if (iPhi == 1) {
            xTopRight = cutoutXpos;
            if (yBot > 0) xBotRight = cutoutXpos;
        } else if (iPhi == nPadColumns) {
            xTopLeft = -1.0 * cutoutXpos;
            if (yBot > 0) xBotLeft = -1.0 * cutoutXpos;
        }
    }

    corners.push_back(Amg::Vector2D(xBotLeft, yBot));
    corners.push_back(Amg::Vector2D(xBotRight, yBot));
    corners.push_back(Amg::Vector2D(xTopLeft, yTop));
    corners.push_back(Amg::Vector2D(xTopRight, yTop));
    return true;
    // return false; // DG-2015-12-01 \todo run validation and determine when this function fails

    // ------------- old implementation ------- [begin]
    // DG-2015-11-30 keep it temporarily, just for reference

    /* DT 2/9/2015
       The test on the indices for pads on subsequent layers is not
       implemented in MuonPadDesign,but in TrigT1NSW (IIRC).
     */
    // int padEta = pad.first-1+padEtaMin;
    // int padPhi = pad.second-1+padPhiMin;
    // if(padEta < padEtaMin || padEta > padEtaMax) return false;
    // if(padPhi < padPhiMin || padPhi > padPhiMax) return false;
    // pos[1] = firstRowPos + (padEta-0.5)*inputRowPitch - radialDistance;
    // double phi = firstPhiPos + padPhi*inputPhiPitch;
    // pos[0] =  tan(phi)*(radialDistance + pos.y());
    // return true;
    // ------------- old implementation ------- [end]
}
//----------------------------------------------------------
