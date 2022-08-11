/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file SCT_CalibBsErrorTool.cxx
 * Implementation file for the SCT_CalibBsErrorTool class
 * @author Shaun Roe
 **/

#include "SCT_CalibBsErrorTool.h"
#include "SCT_CalibUtilities.h"
#include "SCT_CalibNumbers.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"

// RAW data access
#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "InDetRawData/SCT3_RawData.h"

#include "GaudiKernel/ITHistSvc.h"

#include "TFile.h"
#include "TH1I.h"

#include <set>

using namespace SCT_CalibAlgs;

static const std::string pathRoot{"/BSErrors/"};
static const std::string detectorPaths[] {"SCTEC/", "SCTB/","SCTEA/"};
static const std::string detectorNames[] {"negativeEndcap", "barrel", "positiveEndcap"};
static const int n_BSErrorType{15};
static const int firstBSErrorType{0};
static const int lastBSErrorType{14};

SCT_CalibBsErrorTool::SCT_CalibBsErrorTool(const std::string& type, const std::string& name, const IInterface* parent) :
   base_class(type, name, parent)
{
}

StatusCode
SCT_CalibBsErrorTool::initialize() {
   ATH_CHECK(service("THistSvc", m_thistSvc));
   ATH_CHECK(detStore()->retrieve(m_pSCTHelper, "SCT_ID"));
   ATH_CHECK(m_bytestreamErrorsTool.retrieve());

   m_maxHash = m_pSCTHelper->wafer_hash_max();
   m_waferItrBegin = m_pSCTHelper->wafer_begin();
   m_waferItrEnd = m_pSCTHelper->wafer_end();

   return StatusCode::SUCCESS;
}

StatusCode
SCT_CalibBsErrorTool::finalize() {
   ATH_MSG_INFO("Finalize of SCT_CalibBsErrorTool");

   return StatusCode::SUCCESS;
}

bool
SCT_CalibBsErrorTool::book() {
   bool result{true};
   m_phistoVector.clear();
   std::string histoName{pathRoot+"GENERAL/"};
   //histogram for numbers of events
   m_numberOfEventsHisto = new TH1I{"events", "Events", 1, 0.5, 1.5};
   if (m_thistSvc->regHist(histoName.c_str(), m_numberOfEventsHisto ).isFailure()) {
      ATH_MSG_ERROR("Error in booking BSErrors histogram");
   }
   //--- BSErrors for each wafer
   SCT_ID::const_id_iterator waferItr{m_waferItrBegin};
   SCT_ID::const_id_iterator waferItrE{m_waferItrEnd};
   for (; waferItr not_eq waferItrE; ++waferItr) {
      Identifier waferId{*waferItr};
      const int bec{m_pSCTHelper->barrel_ec(waferId)};
      const std::string formattedPosition{formatPosition(waferId, m_pSCTHelper)};
      std::string histotitle{std::string{"SCT "} + detectorNames[bec2Index(bec)] + std::string{" BSErrors : plane "} + formattedPosition};
      const std::string name{pathRoot+detectorPaths[bec2Index(m_pSCTHelper->barrel_ec(waferId))] + formattedPosition};
      TH1F* hitmapHisto_tmp{new TH1F{TString{formattedPosition}, TString{histotitle}, n_BSErrorType, firstBSErrorType-0.5, lastBSErrorType+0.5}};
      if (m_thistSvc->regHist(name.c_str(), hitmapHisto_tmp).isFailure()) {
         ATH_MSG_ERROR("Error in booking BSErrors histogram");
      }
      m_phistoVector.push_back(hitmapHisto_tmp);
   }
   return result;
}

bool
SCT_CalibBsErrorTool::read(const std::string& fileName) {
   bool result{true};
   //pointers to the histos are deleted by m_thistSvc methods
   m_phistoVector.clear();
   TFile* fileHitmap{TFile::Open(fileName.c_str())};
   ATH_MSG_INFO("opening Hitmap file : " << fileName.c_str());

   if (fileHitmap==nullptr) {
      ATH_MSG_ERROR("can not open Hitmap file : " << fileName.c_str());
      return result;
   }
   //histogram for numbers of events
   m_numberOfEventsHisto = static_cast<TH1I*>(fileHitmap->Get("GENERAL/events"));
   if (m_numberOfEventsHisto==nullptr) {
      ATH_MSG_ERROR("Error in reading EventNumber histogram");
   }
   //histograms for each wafer
   SCT_ID::const_id_iterator waferItr{m_waferItrBegin};
   for (; waferItr not_eq m_waferItrEnd; ++waferItr) {
      const Identifier& waferId{*waferItr};
      const std::string formattedPosition{formatPosition(waferId, m_pSCTHelper)};
      std::string name{detectorPaths[bec2Index(m_pSCTHelper->barrel_ec(waferId))] + formattedPosition};
      TH1F* hitmapHisto_tmp{static_cast<TH1F*>(fileHitmap->Get(name.c_str()))};
      if (hitmapHisto_tmp==nullptr) {
         ATH_MSG_ERROR("Error in reading BSErrors histogram");
      } else {
         m_phistoVector.push_back(hitmapHisto_tmp);
      }
   }
   return result;
}

/*
bool
SCT_CalibBsErrorTool::read(const std::string& fileName) {
   ATH_MSG_ERROR("Reading BsError histograms from " << fileName.c_str() << " is not supported!");
   return false;
}
*/

bool
SCT_CalibBsErrorTool::fill(const bool fromData) {
   if (fromData) {
      return fillFromData();
   }
   bool result{true};
   //--- Number of event
   m_numberOfEventsHisto->Fill( 1 );
   //--- Fill BSErrors
   const int maxEntry{static_cast<int>(m_scterr_type->size())};
   for (int i{0}; i != maxEntry; ++i ) {
      int bec{(*m_scterr_bec)[i]};
      int layer{(*m_scterr_layer)[i]};
      int phi{(*m_scterr_phi)[i]};
      int eta{(*m_scterr_eta)[i]};
      int side{(*m_scterr_side)[i]};
      int type{(*m_scterr_type)[i]};
      Identifier waferId{m_pSCTHelper->wafer_id( bec, layer, phi, eta, side )};
      fillBsErrorsForWafer(waferId, type);
   }
   return result;
}

bool
SCT_CalibBsErrorTool::fillFromData() {
  bool result{true};
  //--- Number of event
  m_numberOfEventsHisto->Fill( 1 );
  //--- Loop over BSErrors
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  for (int type = 0; type < SCT_ByteStreamErrors::NUM_ERROR_TYPES; ++type) {
    const std::set<IdentifierHash> errorSet{m_bytestreamErrorsTool->getErrorSet(type, ctx)};
    for(const auto& idHash : errorSet) {
      Identifier waferId{m_pSCTHelper->wafer_id(idHash)};
      fillBsErrorsForWafer(waferId, type);
    }
  }
  return result;
}

void
SCT_CalibBsErrorTool::fillBsErrorsForWafer(const Identifier& waferId, const int type) {
   int iWaferHash{static_cast<int>(m_pSCTHelper->wafer_hash(waferId))};
   const std::string osWafer{formatPosition(waferId, m_pSCTHelper,".")};
   //--- Protection for wrong waferID
   if ( iWaferHash < 0 || iWaferHash >= m_maxHash ) {
      ATH_MSG_WARNING("WaferHash " << iWaferHash << " is out of range : [ bec.layer.eta.phi.side, BSErrorType ] = [ " << osWafer << ", " << type << " ]");
   } else {
      ATH_MSG_DEBUG("BSError : [ bec.layer.eta.phi.side, Type ] = [ " << osWafer<< ", " << type << " ]");
      m_phistoVector[ iWaferHash ]->Fill( type );
   }
}
