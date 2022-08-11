/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// MuonGeoModel
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

// MuonCalibTools
#include "MuonCalibTools/IdToFixedIdTool.h"

// MuonCalibIdentifier
#include "MuonCalibIdentifier/MuonFixedId.h"

// MuonCalibStandAloneBase
#include "MuonCalibStandAloneBase/RegionElement.h"
#include "MuonCalibStandAloneBase/RegionLogicalOperation.h"
#include "MuonCalibStandAloneBase/RegionSelectionSvc.h"
#include "MuonCalibStandAloneBase/RegionSelectorBase.h"

// root
#include "TDirectory.h"
#include "TFile.h"
#include "TKey.h"
#include "TTree.h"

using namespace MuonCalib;
RegionSelectionSvc::~RegionSelectionSvc() = default;
RegionSelectionSvc ::RegionSelectionSvc(const std::string &name, ISvcLocator *svc_locator) : AthService(name, svc_locator) {}

StatusCode RegionSelectionSvc::queryInterface(const InterfaceID &riid, void **ppvUnknown) {
    ATH_MSG_VERBOSE("StatusCode RegionSelectionSvc::queryInterface");

    if (interfaceID().versionMatch(riid)) {
        *ppvUnknown = (RegionSelectionSvc *)this;
    } else {
        return AthService::queryInterface(riid, ppvUnknown);
    }

    return StatusCode::SUCCESS;
}

StatusCode RegionSelectionSvc ::initialize() {
    ATH_MSG_INFO("initialize RegionSelectionSvc");

    if (!ProcessString(m_region_string)) return StatusCode::FAILURE;

    // detector stre - id to fixed id ...
    StoreGateSvc *detStore{nullptr};
    ATH_CHECK(serviceLocator()->service("DetectorStore", detStore));
    ATH_MSG_DEBUG("Retrieved DetectorStore");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore->retrieve(m_detMgr));
    ATH_CHECK(m_idToFixedIdTool.retrieve());
    ATH_MSG_INFO("Retrieved " << m_idToFixedIdTool);

    search_chambers_in_region();
    if (m_print_list_of_selected_chambers) print_list_of_selected_chambers();

    return StatusCode::SUCCESS;
}

bool RegionSelectionSvc ::isInRegion(const MuonCalib::MuonFixedId &id) const { return m_master_region->Result(id); }

std::string RegionSelectionSvc ::GetRegionSelection() const { return m_region_string; }
const std::vector<MuonCalib ::NtupleStationId> &RegionSelectionSvc ::GetStationsInRegions() const { return m_stations_in_region; }
int RegionSelectionSvc ::AddRegionNtuples(const std::string &infile, TChain *chain) {
    std::list<std::string> dirnames;
    return AddRegionNtuples(infile, chain, dirnames);
}
int RegionSelectionSvc ::AddRegionNtuples(const std::string &infile, TChain *chain, std::list<std::string> &dirnames) {
    int n_trees(0);
    if (dirnames.empty()) {
        // open file
        std::unique_ptr<TFile> inf{TFile::Open(infile.c_str(), "READ")};
        TIter nextkey(inf->GetListOfKeys());
        TKey *key;
        // take a list here because erase does not invalidate iterators
        while ((key = (TKey *)nextkey())) { dirnames.push_back(key->GetName()); }
    }
    // loop over found ntuples and selected region
    for (std::set<MuonCalib::NtupleStationId>::const_iterator it = m_unique_chambers.begin(); it != m_unique_chambers.end(); it++)
        for (std::list<std::string>::iterator nt_it = dirnames.begin(); nt_it != dirnames.end(); nt_it++) {
            // truncate region name
            std::string regname(it->regionId(), 0, nt_it->size());
            if (regname == (*nt_it)) {
                chain->AddFile(infile.c_str(), TChain::kBigNumber, ((*nt_it) + "/" + "Segments").c_str());
                std::list<std::string>::iterator new_it = nt_it;
                new_it--;
                dirnames.erase(nt_it);
                nt_it = new_it;
                n_trees++;
            }
        }
    return n_trees;
}

void RegionSelectionSvc ::Print(std::ostream &os) const {
    if (!m_master_region) {
        os << "ERROR";
        return;
    }
    m_master_region->Print(os);
}

void RegionSelectionSvc ::search_chambers_in_region() {
    MdtIdHelper::const_id_iterator it = m_idHelperSvc->mdtIdHelper().module_begin();
    MdtIdHelper::const_id_iterator it_end = m_idHelperSvc->mdtIdHelper().module_end();
    for (; it != it_end; ++it) {
        const MuonGM::MdtReadoutElement *detEl = m_detMgr->getMdtReadoutElement(m_idHelperSvc->mdtIdHelper().channelID(*it, 1, 1, 1));
        if (!detEl) continue;
        // get number of mls;
        int n_mls = m_idHelperSvc->mdtIdHelper().numberOfMultilayers(*it);
        // fixed id
        MuonCalib::MuonFixedId fixed_id(m_idToFixedIdTool->idToFixedId(*it));
        std::vector<MuonCalib::NtupleStationId> the_ids;
        for (int i = 1; i <= n_mls; i++) {
            fixed_id.setMdtMultilayer(i);
            if (isInRegion(fixed_id)) {
                NtupleStationId the_id(fixed_id);
                the_ids.push_back(the_id);
            }
        }
        // all multilayers are in the region - inser the whole station
        if (static_cast<int>(the_ids.size()) == n_mls) {
            NtupleStationId full_station(fixed_id);
            full_station.SetMultilayer(0);
            m_stations_in_region.push_back(full_station);
        } else {
            for (std::vector<MuonCalib::NtupleStationId>::iterator it2 = the_ids.begin(); it2 != the_ids.end(); it2++) {
                m_stations_in_region.push_back(*it2);
            }
        }
    }
    // create a set of chambers (not multilayers)
    std::vector<int> tower(3);
    for (std::vector<MuonCalib::NtupleStationId>::iterator it2 = m_stations_in_region.begin(); it2 != m_stations_in_region.end(); it2++) {
        MuonCalib::NtupleStationId station(*it2);
        station.SetMultilayer(0);
        m_unique_chambers.insert(station);
    }
}

bool RegionSelectionSvc ::ProcessString(const std::string &input) {
    m_master_region = RegionSelectorBase::GetRegion(input);
    return (m_master_region != nullptr);
}

void RegionSelectionSvc ::print_list_of_selected_chambers() const {
    ATH_MSG_INFO("Selected regions: " << m_stations_in_region.size());
    msg(MSG::INFO) << "Selected regions: ";
    for (std::vector<MuonCalib::NtupleStationId>::const_iterator it = m_stations_in_region.begin(); it != m_stations_in_region.end();
         it++) {
        msg(MSG::INFO) << " " << it->regionId();
    }
    msg(MSG::INFO) << endmsg;
}
