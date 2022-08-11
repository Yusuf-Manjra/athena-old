/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  CaloCompactCellTool_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2007
 * @brief Component test for CaloCompactCellTool.
 */

#undef NDEBUG

#include "../src/CaloCompactCellTool.h"
#include "../src/CaloCellPacker_400_500.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloCell.h"
#include "TileEvent/TileCell.h"
#include "LArRecEvent/LArCell.h"
#include "CaloDetDescr/CaloDetectorElements.h"
#include "CaloDetDescr/CaloDetDescriptor.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArMiniFCAL_ID.h"
#include "CaloIdentifier/TileID.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthAllocators/Arena.h"
#include "IdDictParser/IdDictParser.h"
#include "GaudiKernel/Bootstrap.h"
#include "TestTools/initGaudi.h"
#include "TestTools/random.h"
#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/ThinningDecisionBase.h"
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cmath>
#include <sys/resource.h>
#include <sys/time.h>

#include "StoreGate/WriteCondHandleKey.h"
#include "StoreGate/WriteCondHandle.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "AthenaKernel/ExtendedEventContext.h"

using CLHEP::MeV;
using CLHEP::GeV;
using CLHEP::TeV;


typedef long double efloat_t;


#undef CHECK
#define CHECK(s) do { if (!(s).isSuccess()) {printf("error: %s\n", #s);std::abort();}} while(0)


// Dufus-quality RNG, using LCG.  Constants from numerical recipies.
// I don't particularly care about RNG quality here, just about
// getting something that's reproducible.

#include <stdint.h>

using Athena_test::URNG;
using Athena_test::randi_seed;
using Athena_test::randf_seed;




CaloCell_ID* make_helper ()
{
  TileID*     tile_id = new TileID;
  LArEM_ID*   em_id   = new LArEM_ID;
  LArHEC_ID*  hec_id  = new LArHEC_ID;
  LArFCAL_ID* fcal_id = new LArFCAL_ID;
  LArMiniFCAL_ID* minifcal_id = new LArMiniFCAL_ID;

  IdDictParser* parser = new IdDictParser;
  parser->register_external_entity ("LArCalorimeter",
                                    "IdDictLArCalorimeter.xml");
  IdDictMgr& idd = parser->parse ("IdDictParser/ATLAS_IDS.xml");
  em_id->set_do_neighbours (false);
  assert (em_id->initialize_from_dictionary (idd) == 0);
  assert (hec_id->initialize_from_dictionary (idd) == 0);
  fcal_id->set_do_neighbours (false);
  assert (fcal_id->initialize_from_dictionary (idd) == 0);
  minifcal_id->set_do_neighbours (false);
  assert (minifcal_id->initialize_from_dictionary (idd) == 0);
  tile_id->set_do_neighbours (false);
  assert (tile_id->initialize_from_dictionary (idd) == 0);

  CaloCell_ID* calo_helper = new CaloCell_ID (em_id,
                                              hec_id,
                                              fcal_id,
                                              minifcal_id,
                                              tile_id);
  assert (calo_helper->initialize_from_dictionary (idd) == 0);
  return calo_helper;
}


CaloCell_SuperCell_ID* make_sc_helper (IdDictParser* parser)
{
  Tile_SuperCell_ID*    tile_id = new Tile_SuperCell_ID;
  LArEM_SuperCell_ID*   em_id   = new LArEM_SuperCell_ID;
  LArHEC_SuperCell_ID*  hec_id  = new LArHEC_SuperCell_ID;
  LArFCAL_SuperCell_ID* fcal_id = new LArFCAL_SuperCell_ID;
  LArMiniFCAL_ID*       minifcal_id = new LArMiniFCAL_ID;

  parser->register_external_entity ("LArCalorimeter",
                                    "IdDictLArCalorimeter.xml");
  IdDictMgr& idd = parser->parse ("IdDictParser/ATLAS_IDS.xml");
  em_id->set_do_neighbours (false);
  assert (em_id->initialize_from_dictionary (idd) == 0);
  assert (hec_id->initialize_from_dictionary (idd) == 0);
  fcal_id->set_do_neighbours (false);
  assert (fcal_id->initialize_from_dictionary (idd) == 0);
  minifcal_id->set_do_neighbours (false);
  assert (minifcal_id->initialize_from_dictionary (idd) == 0);
  tile_id->set_do_neighbours (false);
  assert (tile_id->initialize_from_dictionary (idd) == 0);

  CaloCell_SuperCell_ID* calo_helper =
    new CaloCell_SuperCell_ID (em_id,
                               hec_id,
                               fcal_id,
                               minifcal_id,
                               tile_id);
  assert (calo_helper->initialize_from_dictionary (idd) == 0);
  return calo_helper;
}


CaloIdManager* make_idmgr (const CaloCell_ID* cellhelper,
                           const CaloCell_SuperCell_ID* schelper)
{
  CaloIdManager* idmgr = new CaloIdManager;
  idmgr->set_helper (cellhelper);
  idmgr->set_helper (cellhelper->em_idHelper());
  idmgr->set_helper (cellhelper->hec_idHelper());
  idmgr->set_helper (cellhelper->fcal_idHelper());
  idmgr->set_helper (cellhelper->tile_idHelper());

  idmgr->set_helper (schelper);
  idmgr->set_helper (schelper->em_idHelper());
  idmgr->set_helper (schelper->hec_idHelper());
  idmgr->set_helper (schelper->fcal_idHelper());
  idmgr->set_helper (schelper->tile_idHelper());

  return idmgr;
}


class DDLookup
{
public:
  DDLookup (const CaloDetDescrManager_Base* mgr);
  CaloDetDescriptor* find (int hashid);

private:
  typedef std::map<Identifier, CaloDetDescriptor*> ddmap_t;
  ddmap_t m_ddmap;
  const CaloCell_Base_ID* m_helper;
};


DDLookup::DDLookup (const CaloDetDescrManager_Base* mgr)
  : m_helper (mgr->getCaloCell_ID())
{
}


CaloDetDescriptor* DDLookup::find (int hashid)
{
  Identifier id = m_helper->cell_id (hashid);
  Identifier reg_id;
  int subcalo = m_helper->sub_calo (id);
  if (subcalo == CaloCell_ID::TILE) {
    int section = m_helper->section (id);
    int side = m_helper->side (id);
    reg_id = m_helper->region_id (subcalo, section, side, 0);
  }
  else {
    int sampling = m_helper->sampling (id);
    int posneg = m_helper->pos_neg (id);
    int region = m_helper->region (id);
    reg_id = m_helper->region_id (subcalo, posneg, sampling, region);
  }
  CaloDetDescriptor* dd = m_ddmap[reg_id];
  if (!dd) {
    dd = new CaloDetDescriptor (reg_id, m_helper->tile_idHelper(), m_helper);
    m_ddmap[reg_id] = dd;
  }
  return dd;
}


CaloCell* make_cell (int hashid,
                     CaloDetDescrManager* mgr,
                     DDLookup& ddlookup,
                     URNG& rng)
{
  CaloDetDescriptor* descr = ddlookup.find (hashid);
  CaloDetDescrElement* dde = new DummyDetDescrElement (hashid -
                                                        descr->caloCellMin(),
                                                       0,
                                                       0,
                                                       descr);
  mgr->add (dde);

  if (descr->is_tile()) {
    int gain1;
    efloat_t ene1;
    switch (randi_seed(rng.seed, 2)) {
    case 0:
      gain1 = TileID::LOWGAIN;
      ene1 = randf_seed(rng.seed, 3200*GeV)-20*GeV;
      break;
    case 1:
    default:
      gain1 = TileID::HIGHGAIN;
      ene1 = randf_seed(rng.seed, 50*GeV)-20*GeV;
      break;
    }
    int qbit1 = randi_seed(rng.seed, 32);
    int qual1 = ((qbit1 & TileCell::MASK_BADCH) != 0) ? 255 : randi_seed(rng.seed, 256);
    int time1 = 0;
    if (randi_seed(rng.seed, 2)>0) {
      qbit1 |= TileCell::KEEP_TIME;
      time1 = (randi_seed(rng.seed, 2000)-1000);
    }  

    int gain2;
    efloat_t ene2;
    switch (randi_seed(rng.seed, 3)) {
    case 0:
      gain2 = TileID::LOWGAIN;
      ene2 = randf_seed(rng.seed, 3200*GeV)-20*GeV;
      break;
    case 1:
      gain2 = TileID::HIGHGAIN;
      ene2 = randf_seed(rng.seed, 50*GeV)-20*GeV;
      break;
    default:
      gain2 = CaloGain::INVALIDGAIN;
      ene2 = 0;
      break;
    }
    int qbit2 = randi_seed(rng.seed, 32);
    int qual2 = ((qbit2 & TileCell::MASK_BADCH) != 0) ? 255 : randi_seed(rng.seed, 256);
    int time2 = 0;
    if (randi_seed(rng.seed, 2)>0) {
      qbit2 |= TileCell::KEEP_TIME;
      time2 = (randi_seed(rng.seed, 2000)-1000);
    }
    
    if (((gain1 == TileID::LOWGAIN && gain2 == TileID::HIGHGAIN) ||
         (gain1 == TileID::HIGHGAIN && gain2 == TileID::LOWGAIN)) &&
        ene1 + ene2 > 400*GeV)
    {
      float scale = 399*GeV / (ene1 + ene2);
      ene1 *= scale;
      ene2 *= scale;
    }

    if (gain1 == TileID::HIGHGAIN && gain2 == TileID::HIGHGAIN &&
        ene1 + ene2 > 50*GeV)
    {
      efloat_t scale = 49*GeV / (ene1 + ene2);
      ene1 *= scale;
      ene2 *= scale;
    }

    if ( ((qbit1 & qbit2) & TileCell::MASK_BADCH) != 0 ) { // both channels masked
      time1 = time2 = 0;
      ene1  = ene2  = 0.512932 * MeV;
      gain1 = gain2 = TileID::LOWGAIN;
    }
    
    return new TileCell (dde,
                         ene1, ene2,
                         time1, time2,
                         qual1, qual2,
                         qbit1, qbit2,
                         gain1, gain2);
  }
  else {
    CaloGain::CaloGain gain = CaloGain::INVALIDGAIN;
    efloat_t energy = 0;
    int quality = randi_seed(rng.seed, 65536);
    int provenance = randi_seed(rng.seed, 0x2000);
    float time = 0;
    // Good quality?  Take 10% to be bad.
    if (randi_seed(rng.seed, 10) != 0) {
      provenance |= 0x2000;
      switch (randi_seed(rng.seed, 3)) {
      case 0:
        gain = CaloGain::LARHIGHGAIN;
        energy = randf_seed(rng.seed, 60*GeV) - 16*GeV;
        break;
      case 1:
        gain = CaloGain::LARMEDIUMGAIN;
        energy = randf_seed(rng.seed, 360*GeV) + 40*GeV;
        break;
      case 2:
      default:
        gain = CaloGain::LARLOWGAIN;
        energy = randf_seed(rng.seed, 3000*GeV) + 40*GeV;
        break;
      }
      time = randf_seed(rng.seed, 2000) - 1000;
    }
    return new CaloCell (dde, energy, time, quality, provenance, gain);
  }
}


std::vector<CaloCell*> make_cells (CaloDetDescrManager* mgr,
                                   DDLookup& ddlookup,
                                   URNG& rng)
{
  size_t hashmax = mgr->getCaloCell_ID()->calo_cell_hash_max(); 
 std::vector<CaloCell*> v;
  v.reserve (hashmax);
  for (size_t i = 0; i < hashmax; i++)
    v.push_back (make_cell (i, mgr, ddlookup, rng));

  // Give a few cells off-scale energies, to test saturation.
  IdentifierHash tilemin, tilemax;
  mgr->getCaloCell_ID()->calo_cell_hash_range (CaloCell_ID::TILE,
                                               tilemin, tilemax);
  assert (tilemax <= v.size());
  assert (tilemin <= tilemax);
  for (size_t i = 0; i < 10; i++) {
    CaloCell* cell = v[randi_seed(rng.seed, tilemin)];
    assert ( ! cell->caloDDE()->is_tile());
    cell->setGain ((i&2) ? CaloGain::LARLOWGAIN : CaloGain::LARHIGHGAIN);
    cell->setEnergy ((((int)i&1)*2-1) * 4000*GeV);
  }
  for (size_t i = 0; i < 8; i++) {
    TileCell* cell =
      dynamic_cast<TileCell*>(v[tilemin + randi_seed(rng.seed, tilemax-tilemin)]);
    assert (cell != 0);
    assert (cell->caloDDE()->is_tile());
    int pmt = (i&2)>>1;
    cell->addEnergy ((((int)i&1)*2-1) * 8000*GeV, pmt,
                     (i&4) ? TileID::LOWGAIN : TileID::HIGHGAIN);
  }
  return v;
}


void fill_cells_rand (int n,
                      const std::vector<CaloCell*>& cells,
                      CaloCellContainer& cont,
                      URNG& rng)
{
  std::vector<CaloCell*> tmp = cells;
  std::shuffle (tmp.begin(), tmp.end(), rng);
  for (int i = 0; i < n; i++)
    cont.push_back (tmp[i]);
}


void fill_cells_clustery (int n,
                          const std::vector<CaloCell*>& cells,
                          CaloCellContainer& cont,
                          URNG& rng)
{
  std::vector<CaloCell*> tmp = cells;
  while (n > 0) {
    int idx = randi_seed (rng.seed, tmp.size());
    int nmax = std::min ((unsigned int)n, (unsigned int)tmp.size() - idx);
    assert (nmax > 0);
    int thisn = randi_seed (rng.seed, nmax) + 1;
    for (int i = idx; i < idx + thisn; i++)
      cont.push_back (tmp[i]);
    tmp.erase (tmp.begin()+idx, tmp.begin()+idx+thisn);
    n -= thisn;
  }
}


CaloCellContainer* fill_cells (int n,
                               const std::vector<CaloCell*>& cells,
                               bool clustery,
                               bool ordered,
                               URNG& rng)
{
  CaloCellContainer* cont = new CaloCellContainer (SG::VIEW_ELEMENTS);
  if (clustery)
    fill_cells_clustery (n, cells, *cont, rng);
  else
    fill_cells_rand (n, cells, *cont, rng);

  if (ordered) {
    cont->order();
    cont->updateCaloIterators();
  }
  return cont;
}


CaloCellContainer* fill_supercells (const std::vector<CaloCell*>& cells,
				    CaloSuperCellDetDescrManager* scmgr,
                                    ISvcLocator* svcloc)
{
  CaloCellContainer* cont = new CaloCellContainer;

  IToolSvc* toolsvc = 0;
  CHECK( svcloc->service ("ToolSvc", toolsvc, true) );

  ICaloSuperCellIDTool* sctool = 0;
  CHECK( toolsvc->retrieveTool ("CaloSuperCellIDTool", sctool) );

  StoreGateSvc* detstore = 0;
  CHECK( svcloc->service ("DetectorStore", detstore) );
  const CaloIdManager* idmgr = 0;
  CHECK( detstore->retrieve (idmgr, "CaloIdManager") );
  const CaloCell_SuperCell_ID* schelper =
    idmgr->getCaloCell_SuperCell_ID();

  DDLookup ddlookup (scmgr);

  std::vector<CaloCell*> scells (schelper->calo_cell_hash_max());
  for (const CaloCell* cell : cells) {
    Identifier scid = sctool->offlineToSuperCellID (cell->ID());
    if (!scid.is_valid()) continue;
    IdentifierHash hash = schelper->calo_cell_hash (scid);
    assert (hash < scells.size());
    if (scells[hash] == 0) {
      const CaloDetDescrElement* dde = scmgr->get_element (hash);
      if (!dde) {
        CaloDetDescriptor* descr = ddlookup.find (hash);
        CaloDetDescrElement* dde_nc =
          new DummyDetDescrElement (hash -
                                      descr->caloCellMin(),
                                    0,
                                    0,
                                    descr);
        scmgr->add (dde_nc);
        dde = dde_nc;
      }
      CaloGain::CaloGain gain = cell->gain();
      switch (gain)
      {
      case CaloGain::TILELOWLOW:
      case CaloGain::TILELOWHIGH:
      case CaloGain::TILEONELOW:
        gain = CaloGain::LARLOWGAIN;
        break;
      case CaloGain::TILEHIGHHIGH:
      case CaloGain::TILEHIGHLOW:
      case CaloGain::TILEONEHIGH:
        gain = CaloGain::LARHIGHGAIN;
        break;
      default:
        break;
      }
      scells[hash] = new CaloCell (dde,
                                   cell->energy(),
                                   cell->provenance() & 0x2000 ? 
                                     cell->time() : 0,
                                   cell->quality(),
                                   cell->provenance() & 0x3fff,
                                   gain);
    }
    else if (scells[hash]->gain() != CaloGain::INVALIDGAIN) {
      scells[hash]->setEnergy (scells[hash]->energy() + cell->energy());
    }
  }

  for (CaloCell* cell : scells) {
    cont->push_back (cell);
  }

  cont->order();
  cont->updateCaloIterators();
  return cont;
}


void compare_float (float x1, float x2,
                    const std::string& what,
                    float thresh=0.01)
{
  float den = std::abs(x1) + std::abs(x2);
  if (den == 0) den = 1;
  if (den < 1e-3) den = 1e-3;
  if (std::abs(x1-x2) / den > thresh) {
    printf ("Float %s miscompare: %f %f\n", what.c_str(), x1, x2);
    std::abort();
  }
}


void compare_lar_energy (int hash, float e1, float e2, float prec)
{
  std::ostringstream ss;
  ss << "energy " << hash;
  if (e1 > 3.2*TeV)
    compare_float (3.2*TeV, e2, ss.str(), prec);
  else if (e1 < -3.2*TeV)
    compare_float (-3.2*TeV, e2, ss.str(), prec);
  else
    compare_float (e1, e2, ss.str(), prec);
}


float tile_ele (const TileCell* c, int pmt)
{
  if (pmt)
    return c->ene2();
  return c->ene1();
}


int tile_gain (const TileCell* c, int pmt)
{
  if (pmt)
    return c->gain2();
  return c->gain1();
}


void compare_tile_energy (const TileCell* c1, const TileCell* c2,
                          int pmt,
                          const char* what)
{
  float max = 3.2*TeV;
  if (tile_gain (c2, pmt) == TileID::HIGHGAIN)
    max = 50*GeV;

  float prec = 0.01;
  if (fabs (c2->ene1() - c2->ene2()) > 30*GeV) prec = 0.02;

  float e1 = tile_ele (c1, pmt);
  float e2 = tile_ele (c2, pmt);

  if (e1 > max)
    compare_float (max, e2, what, prec);
  else if (e1 < -max)
    compare_float (-max, e2, what, prec);
  else
    compare_float (e1, e2, what, prec);
}


void compare_lar (const CaloCell* cell1, const CaloCell* cell2,
                  int version)
{
  const CaloDetDescrElement* dde = cell1->caloDDE();
  
  float prec = 0.01;
  if (fabs(cell1->energy()) < 0.1)
    prec = 0.1;
  else if (fabs(cell1->energy()) < 10) {
    if (dde->getSubCalo() == CaloCell_ID::LARHEC)
      prec = 0.02;
    else
      prec = 0.015;
  }

  if (dde->getSubCalo() == CaloCell_ID::TILE)
    prec = 0.03;  // supercell

  compare_lar_energy (dde->calo_hash(),
                      cell1->energy(), cell2->energy(), prec);

  std::ostringstream twhat;
  twhat << "time " << dde->calo_hash();
  compare_float (cell1->time(), cell2->time(), twhat.str());
  assert (cell1->gain() == cell2->gain());

  // Test quality flag.
  assert ((cell1->provenance()&0x2000) == (cell2->provenance()&0x2000));

  if (version <= CaloCompactCellTool::VERSION_400) {
    // Quality not saved.
    assert (cell2->quality() == 0);
  }
  else {
    assert (cell1->provenance() == cell2->provenance());

    // Quality saved if good flag is set.
    if ((cell1->provenance() & 0x2000) != 0) {
      assert (cell1->quality() == cell2->quality());
    }
    else {
      assert (cell2->quality() == 0);
    }
  }
}


void compare_tile (const TileCell* cell1, const TileCell* cell2,
                   int version)
{
  assert (cell1->gain1() == cell2->gain1());
  assert (cell1->gain2() == cell2->gain2());
  compare_tile_energy (cell1, cell2, 0, "ene1");
  compare_float (cell1->time1(), cell2->time1(), "time1", 0.1);

  assert ((cell1->qbit1() >= TileCell::KEEP_TIME) == 
          (cell2->qbit1() >= TileCell::KEEP_TIME));

  if (version < CaloCompactCellTool::VERSION_502 ) {
    assert ((cell2->qbit1() & 0x1F) == 0); // Provenance not saved.
  }
  else {
    assert ((cell1->qbit1() & 0x1F) == (cell2->qbit1() & 0x1F));
  }

  if (version < CaloCompactCellTool::VERSION_502 ) {
    assert (cell2->qual1() == 0); // Quality not saved.
  } else {
      if (cell1->qbit1() < TileCell::KEEP_TIME && 
          (cell1->qbit2() < TileCell::KEEP_TIME || cell1->gain2() == CaloGain::INVALIDGAIN)) {
      if ((cell1->qbit1() & TileCell::MASK_BADCH) != 0)
        assert (cell2->qual1() == 255); // Quality is bad.
      else 
        assert (cell2->qual1() == 0); // Quality is good.
    } else {
      assert (cell1->qual1() == cell2->qual1());
    }
  }

  if (cell1->gain2() != CaloGain::INVALIDGAIN) {
    compare_tile_energy (cell1, cell2, 1, "ene2");
    compare_float (cell1->time2(), cell2->time2(), "time2", 0.1);
    assert ((cell1->qbit2() >= TileCell::KEEP_TIME) == 
            (cell2->qbit2() >= TileCell::KEEP_TIME));

    if (version < CaloCompactCellTool::VERSION_502 ) {
      assert ((cell2->qbit2() & 0x1F) == 0); // Provenance not saved.
    }
    else {
      assert ((cell1->qbit2() & 0x1F) == (cell2->qbit2() & 0x1F));
    }

    if (version < CaloCompactCellTool::VERSION_502 ) {
      assert (cell2->qual2() == 0); // Quality not saved.
    } else {
      if (cell1->qbit1() < TileCell::KEEP_TIME && cell1->qbit2() < TileCell::KEEP_TIME) {
        if ((cell1->qbit2() & TileCell::MASK_BADCH) != 0)
          assert (cell2->qual2() == 255); // Quality is bad.
        else 
          assert (cell2->qual2() == 0); // Quality is good.
      } else {
        assert (cell1->qual2() == cell2->qual2());
      }
    }
  }
}


void dump_packed (const CaloCompactCellContainer& packed)
{
  const std::vector<CaloCompactCellContainer::value_type>& data =
    packed.getData();

  int ccount = 0;
  for (size_t i = 0; i < data.size(); i++) {
    if (ccount == 0)
      printf ("\n%08x ", (unsigned int)i*4);
    printf (" %08x", data[i]);
    ++ccount;
    if (ccount == 4) ccount = 0;
  }
  printf ("\n");
}


void dump_e (double e)
{
  if (e > 1000*GeV)
    e = round (e * 0.05) * 20;
  else
    e = round (e * 0.10) * 10;
  printf ("%8.0f ", e);
}


void dump_cells (const CaloCellContainer& cont)
{
  for (size_t i = 0; i < cont.size(); i++) {
    const CaloCell* cell = cont[i];
    printf ("%8u ", (unsigned int)cell->caloDDE()->calo_hash());
    if (const TileCell* tcell = dynamic_cast<const TileCell*> (cell)) {
      dump_e (tcell->ene1());
      dump_e (tcell->ene2());
      printf ("%8.2f %8.2f %d %d 0x%02x 0x%02x %d %d\n",
              tcell->time1(), tcell->time2(),
              tcell->qual1(), tcell->qual2(),
              tcell->qbit1(), tcell->qbit2(),
              (int)tcell->gain1(), (int)tcell->gain2());
    }
    else {
      printf ("%8.1f %8.2f %d %d %d\n",
              cell->energy(), cell->time(),
              cell->quality(), cell->provenance(),
              (int)cell->gain());
    }
  }
}


void compare_containers (const CaloCellContainer* cont,
                         const CaloCellContainer* cont2,
                         bool ordered,
                         int version)
{
  assert (cont->size() == cont2->size());

  CaloCell_ID::SUBCALO subcalos[4] = {CaloCell_ID::LAREM,
                                      CaloCell_ID::LARHEC,
                                      CaloCell_ID::LARFCAL,
                                      CaloCell_ID::TILE};
  for (int i = 0; i < 4; i++) {
    assert (cont->hasCalo(subcalos[i]) ==
            cont2->hasCalo(subcalos[i]));
    if (ordered) {
      assert (cont->indexFirstCellCalo(subcalos[i]) ==
              cont2->indexFirstCellCalo(subcalos[i]));
      assert (cont->indexLastCellCalo(subcalos[i]) ==
              cont2->indexLastCellCalo(subcalos[i]));
      assert (cont->nCellsCalo(subcalos[i]) ==
              cont2->nCellsCalo(subcalos[i]));
    }
  }

  for (size_t i = 0; i < cont->size(); i++) {
    const CaloCell* cell1 = (*cont)[i];
    const CaloCell* cell2 = (*cont2)[i];
    assert (cell1->caloDDE() == cell2->caloDDE());
    if (dynamic_cast<const LArCell*> (cell2) != 0) {
      assert (dynamic_cast<const TileCell*> (cell1) == 0);
      compare_lar (cell1, cell2, version);
    }
    else if (const TileCell* tcell2 =
             dynamic_cast<const TileCell*> (cell2)) {
      const TileCell* tcell1 = dynamic_cast<const TileCell*> (cell1);
      assert (tcell1 != 0);
      compare_tile (tcell1, tcell2, version);
    }
    else
      std::abort();
  }
}


void compare_thinned_containers (const CaloCellContainer* cont,
                                 const CaloCellContainer* cont2,
                                 const SG::ThinningDecisionBase* dec,
                                 int version)
{
  size_t i2 = 0;
  for (size_t i1 = 0; i1 < cont->size(); ++i1) {
    if (dec->thinned (i1)) continue;
    assert (i2 < cont2->size());
    const CaloCell* cell1 = (*cont)[i1];
    const CaloCell* cell2 = (*cont2)[i2];
    ++i2;

    assert (cell1->caloDDE() == cell2->caloDDE());
    if (dynamic_cast<const LArCell*> (cell2) != 0) {
      assert (dynamic_cast<const TileCell*> (cell1) == 0);
      compare_lar (cell1, cell2, version);
    }
    else if (const TileCell* tcell2 =
             dynamic_cast<const TileCell*> (cell2)) {
      const TileCell* tcell1 = dynamic_cast<const TileCell*> (cell1);
      assert (tcell1 != 0);
      compare_tile (tcell1, tcell2, version);
    }
    else
      std::abort();
  }

  assert (i2 == cont2->size());
}


void test_one (int n,
               int version,
               const std::vector<CaloCell*>& cells,
               CaloCompactCellTool& tool,
               URNG& rng,
               SG::Arena& arena,
               bool clustery = false,
               bool dump = false,
               bool ordered = true,
               const SG::ThinningDecisionBase* dec = nullptr)
{
  printf ("*** test %d %d %d %d\n", version, n, clustery, ordered);
  CaloCellContainer* cont = fill_cells (n, cells, clustery, ordered, rng);

  if (dump)
    dump_cells (*cont);

  CaloCompactCellContainer ccc;
  assert (tool.getPersistent (*cont, &ccc, dec, version));

  if (dump)
    dump_packed (ccc);

  CaloCellContainer* cont2 = new CaloCellContainer (SG::VIEW_ELEMENTS);
  SG::Arena::Push push (arena);
  assert (tool.getTransient (ccc, cont2));

  if (dump)
    dump_cells (*cont2);

  if (dec)
    compare_thinned_containers (cont, cont2, dec, version);
  else
    compare_containers (cont, cont2, ordered, version);

  delete cont;
  delete cont2;

  arena.reset();
}


void test_supercells (int version,
		      CaloCellContainer* cont,
                      CaloCompactCellTool& tool,
                      SG::Arena& arena)
{
  printf ("*** test SC %d\n", version);
  //dump_cells (*cont);

  CaloCompactCellContainer ccc;
  assert (tool.getPersistent (*cont, &ccc, nullptr, version));

  CaloCellContainer* cont2 = new CaloCellContainer (SG::VIEW_ELEMENTS);
  SG::Arena::Push push (arena);
  assert (tool.getTransient (ccc, cont2));
  //dump_cells (*cont2);

  compare_containers (cont, cont2, true, version);

  delete cont;
  delete cont2;
  arena.reset();
}


//============================================================================


class CaloCellPacker_400_500_test
{
public:
  typedef CaloCellPacker_400_500::header500 header_t;
  typedef CaloCompactCellContainer::value_type value_type;
  typedef std::vector<value_type> vec_t;

  static
  void test_fin (const CaloCompactCellContainer& ccc,
                 CaloCompactCellTool& tool,
                 SG::Arena& arena);

  static
  void test_err1 (const CaloCompactCellContainer& ccc,
                  CaloCompactCellTool& tool,
                  SG::Arena& arena);
  static
  void test_err2 (const CaloCompactCellContainer& ccc,
                  CaloCompactCellTool& tool,
                  SG::Arena& arena);

  static
  void test_err3 (const CaloCompactCellContainer& ccc,
                  CaloCompactCellTool& tool,
                  SG::Arena& arena);

  static
  void test_err4 (const CaloCompactCellContainer& ccc,
                  CaloCompactCellTool& tool,
                  SG::Arena& arena);

  static
  void test_err5 (const CaloCompactCellContainer& ccc,
                  CaloCompactCellTool& tool,
                  SG::Arena& arena);

  static
  void test_err6 (const CaloCompactCellContainer& ccc,
                  CaloCompactCellTool& tool,
                  SG::Arena& arena);

  static
  void test_err7 (const CaloCompactCellContainer& ccc,
                  CaloCompactCellTool& tool,
                  SG::Arena& arena);

  static
  void test_err8 (const CaloCompactCellContainer& ccc,
                  CaloCompactCellTool& tool,
                  SG::Arena& arena);

  static
  void test_err9 (const CaloCompactCellContainer& ccc,
                  CaloCompactCellTool& tool,
                  SG::Arena& arena);

  static
  void test_err10 (const CaloCompactCellContainer& ccc,
                   CaloCompactCellTool& tool,
                   SG::Arena& arena,
                   CaloDetDescrManager* mgr);

  static
  void test_err11 (const CaloCompactCellContainer& ccc,
                   CaloCompactCellTool& tool,
                   SG::Arena& arena);
};


void
CaloCellPacker_400_500_test::test_fin (const CaloCompactCellContainer&
                                       ccc2,
                                       CaloCompactCellTool& tool,
                                       SG::Arena& arena)
{
  CaloCellContainer cont2 (SG::VIEW_ELEMENTS);
  SG::Arena::Push push (arena);
  assert (tool.getTransient (ccc2, &cont2));
}


void
CaloCellPacker_400_500_test::test_err1 (const CaloCompactCellContainer&
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err1\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  header_t& header = *reinterpret_cast<header_t*> (&*vec.begin());
  header.m_ncells_larhec = 1000000;
  test_fin (ccc2, tool, arena);
}


void
CaloCellPacker_400_500_test::test_err2 (const CaloCompactCellContainer& 
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err2\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  header_t& header = *reinterpret_cast<header_t*> (&*vec.begin());
  header.m_seq_larem = 1000000;
  test_fin (ccc2, tool, arena);
}


void
CaloCellPacker_400_500_test::test_err3 (const CaloCompactCellContainer& 
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err3\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  header_t& header = *reinterpret_cast<header_t*> (&*vec.begin());
  header.m_seq_larem = 10000;
  test_fin (ccc2, tool, arena);
}


void
CaloCellPacker_400_500_test::test_err4 (const CaloCompactCellContainer&
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err4\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  vec.resize (vec.size() + 100);
  test_fin (ccc2, tool, arena);
}


void
CaloCellPacker_400_500_test::test_err5 (const CaloCompactCellContainer& 
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err5\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  vec.resize (vec.size() - 100);
  test_fin (ccc2, tool, arena);
}


void
CaloCellPacker_400_500_test::test_err6 (const CaloCompactCellContainer& 
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err6\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  vec.resize (vec.size() - 100);
  header_t& header = *reinterpret_cast<header_t*> (&*vec.begin());
  value_type val = vec[header.m_length];
  vec[header.m_length] = (10000<<2) | (val & 0xffff0003);
  test_fin (ccc2, tool, arena);
}


void
CaloCellPacker_400_500_test::test_err7 (const CaloCompactCellContainer& 
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err7\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  vec.resize (vec.size() - 100);
  header_t& header = *reinterpret_cast<header_t*> (&*vec.begin());
  value_type val = vec[header.m_length];
  int newhash = 200000;
  vec[header.m_length] = (newhash>>16) | ((newhash&0xffff)<<16) | (val&0xfffc);
  test_fin (ccc2, tool, arena);
}


void
CaloCellPacker_400_500_test::test_err8 (const CaloCompactCellContainer& 
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err8\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  header_t* header = reinterpret_cast<header_t*> (&*vec.begin());
  vec.insert (vec.begin() + header->m_length, 10, 0);
  header = reinterpret_cast<header_t*> (&*vec.begin());
  header->m_length += 10;
  test_fin (ccc2, tool, arena);
}


// Deletes the last word of the header.
void
CaloCellPacker_400_500_test::test_err9 (const CaloCompactCellContainer& 
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err9\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  header_t* header = reinterpret_cast<header_t*> (&*vec.begin());
  vec.erase (vec.begin() + header->m_length-1);
  header = reinterpret_cast<header_t*> (&*vec.begin());
  --header->m_length;
  test_fin (ccc2, tool, arena);
}


void
CaloCellPacker_400_500_test::test_err10(const CaloCompactCellContainer&
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena,
                                        CaloDetDescrManager* mgr)
{
  printf (" --- err10\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  header_t& header = *reinterpret_cast<header_t*> (&*vec.begin());
  value_type val = vec[header.m_length];
  int nseq = (val & 0xfffc) >> 2;
  int hash = (val & 3) << 16 | ((val & 0xffff0000)>>16);
  int target = hash + std::min (10, nseq);

  CaloDetDescrElement* dde = mgr->release_element (target);
  test_fin (ccc2, tool, arena);
  mgr->add (dde);
}


void
CaloCellPacker_400_500_test::test_err11(const CaloCompactCellContainer&
                                        ccc,
                                        CaloCompactCellTool& tool,
                                        SG::Arena& arena)
{
  printf (" --- err11\n");
  CaloCompactCellContainer ccc2 = ccc;
  vec_t& vec = ccc2.getData();
  header_t& header = *reinterpret_cast<header_t*> (&*vec.begin());
  header.m_lengthProvenance = 9999999;
  test_fin (ccc2, tool, arena);
}


// Test handling of corrupt data.
void test_errs (const std::vector<CaloCell*>& cells,
                CaloCompactCellTool& tool,
                URNG& rng,
                SG::Arena& arena,
                CaloDetDescrManager* mgr)
{
  printf ("*** test_errs\n");
  CaloCellContainer* cont = fill_cells (10000, cells, true, true, rng);
  CaloCompactCellContainer ccc;
  assert (tool.getPersistent (*cont, &ccc, nullptr, CaloCompactCellTool::VERSION_501));

  typedef CaloCellPacker_400_500_test T;

  T::test_err1  (ccc, tool, arena);
  T::test_err2  (ccc, tool, arena);
  T::test_err3  (ccc, tool, arena);
  T::test_err4  (ccc, tool, arena);
  T::test_err5  (ccc, tool, arena);
  T::test_err6  (ccc, tool, arena);
  T::test_err7  (ccc, tool, arena);
  T::test_err8  (ccc, tool, arena);
  T::test_err9  (ccc, tool, arena);
  T::test_err10 (ccc, tool, arena, mgr);
  T::test_err11 (ccc, tool, arena);

  // Unordered
  printf (" --- err unordered\n");
  rng.seed = 101;
  CaloCellContainer* cont2 = fill_cells (10000, cells, true, false, rng);
  CaloCompactCellContainer ccc2;
  assert (tool.getPersistent (*cont2, &ccc2, nullptr, CaloCompactCellTool::VERSION_500));
  T::test_fin (ccc2, tool, arena);

  delete cont;
}


//============================================================================


std::vector<CaloCell*> init (IdDictParser* parser,
                             URNG& rng,
                             ISvcLocator*& svcloc,
                             CaloDetDescrManager*& mgr,
			     CaloSuperCellDetDescrManager*& scmgr,
			     CaloCellContainer*& cont)
{
  if (!Athena_test::initGaudi("CaloCompactCellTool_test.txt", svcloc)) {
    std::cerr << "This test can not be run" << std::endl;
    std::abort();
  }  

  CaloCell_ID* helper = make_helper ();
  CaloCell_SuperCell_ID* schelper = make_sc_helper (parser);
  CaloIdManager* idmgr = make_idmgr (helper, schelper);
  mgr = new CaloDetDescrManager;
  mgr->set_helper (helper);
  mgr->initialize();

  scmgr = new CaloSuperCellDetDescrManager;
  scmgr->set_helper (schelper);
  scmgr->initialize();

  StoreGateSvc* detstore = nullptr;
  CHECK( svcloc->service ("DetectorStore", detstore) );
  StoreGateSvc* condstore = nullptr;
  CHECK( svcloc->service ("StoreGateSvc/ConditionStore", condstore) );

  CHECK( detstore->record (idmgr, "CaloIdManager") );
  CHECK( detstore->record (helper, "CaloCell_ID") );
  CHECK( detstore->record (schelper, "CaloCell_SuperCell_ID") );

  DDLookup ddlookup (mgr);
  std::vector<CaloCell*> cells = make_cells (mgr, ddlookup, rng);

  cont = fill_supercells(cells,scmgr,svcloc);

  SG::WriteCondHandleKey<CaloDetDescrManager> caloMgrKey{"CaloDetDescrManager"};
  CHECK( caloMgrKey.initialize() );
  SG::WriteCondHandle<CaloDetDescrManager> caloMgrHandle{caloMgrKey};
  caloMgrHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));
  CHECK( caloMgrHandle.record(mgr) );

  SG::WriteCondHandleKey<CaloSuperCellDetDescrManager> caloSCMgrKey{"CaloSuperCellDetDescrManager"};
  CHECK( caloSCMgrKey.initialize() );
  SG::WriteCondHandle<CaloSuperCellDetDescrManager> caloSCMgrHandle{caloSCMgrKey};
  caloSCMgrHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));
  CHECK( caloSCMgrHandle.record(scmgr) );


  EventIDBase now(0, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, 0, 1);
  EventContext ctx(1);
  ctx.setEventID( now );
  ctx.setExtension( Atlas::ExtendedEventContext(condstore) );
  Gaudi::Hive::setCurrentContext(ctx);

  return cells;
}


void runtests (IdDictParser* parser)
{
  Athena_test::URNG rng;
  CaloCompactCellTool tool;
  ISvcLocator* svcloc = nullptr;
  CaloDetDescrManager* mgr = nullptr;
  CaloSuperCellDetDescrManager* scmgr = nullptr;
  CaloCellContainer* cont = nullptr;
  std::vector<CaloCell*> cells = init (parser, rng, svcloc, mgr, scmgr, cont);
  auto arena = std::make_unique<SG::Arena> ("arena");

  rng.seed = 10;
  test_one (400, CaloCompactCellTool::VERSION_400, cells, tool, rng, *arena, false, true);
  test_one (400, CaloCompactCellTool::VERSION_400, cells, tool, rng, *arena, true, true);
  test_one (10000, CaloCompactCellTool::VERSION_400, cells, tool, rng, *arena);
  test_one (10000, CaloCompactCellTool::VERSION_400, cells, tool, rng, *arena, true);

  test_one (100000, CaloCompactCellTool::VERSION_400, cells, tool, rng, *arena);
  test_one (100000, CaloCompactCellTool::VERSION_400, cells, tool, rng, *arena, true);

  test_one (cells.size(), CaloCompactCellTool::VERSION_400, cells, tool, rng, *arena);

  rng.seed = 20;
  test_one (400, CaloCompactCellTool::VERSION_500, cells, tool, rng, *arena, false, true);
  test_one (400, CaloCompactCellTool::VERSION_500, cells, tool, rng, *arena, true, true);

  test_one (10000, CaloCompactCellTool::VERSION_500, cells, tool, rng, *arena);
  test_one (10000, CaloCompactCellTool::VERSION_500, cells, tool, rng, *arena, true);

  test_one (100000, CaloCompactCellTool::VERSION_500, cells, tool, rng, *arena);
  test_one (100000, CaloCompactCellTool::VERSION_500, cells, tool, rng, *arena, true);

  test_one (cells.size(), CaloCompactCellTool::VERSION_500, cells, tool, rng, *arena);

  rng.seed = 40;
  test_one (400, CaloCompactCellTool::VERSION_501, cells, tool, rng, *arena, false);
  test_one (400, CaloCompactCellTool::VERSION_501, cells, tool, rng, *arena,  true);

  test_one (10000, CaloCompactCellTool::VERSION_501, cells, tool, rng, *arena);
  test_one (10000, CaloCompactCellTool::VERSION_501, cells, tool, rng, *arena, true);

  test_one (100000, CaloCompactCellTool::VERSION_501, cells, tool, rng, *arena);
  test_one (100000, CaloCompactCellTool::VERSION_501, cells, tool, rng, *arena, true);

  test_one (cells.size(), CaloCompactCellTool::VERSION_501, cells, tool, rng, *arena);

  // Unordered
  test_one (400, CaloCompactCellTool::VERSION_501, cells, tool, rng, *arena, false,
            true, false);

  rng.seed = 50;
  test_one (400, CaloCompactCellTool::VERSION_502, cells, tool, rng, *arena, false);
  test_one (400, CaloCompactCellTool::VERSION_502, cells, tool, rng, *arena,  true);

  test_one (10000, CaloCompactCellTool::VERSION_502, cells, tool, rng, *arena);
  test_one (10000, CaloCompactCellTool::VERSION_502, cells, tool, rng, *arena, true);

  test_one (100000, CaloCompactCellTool::VERSION_502, cells, tool, rng, *arena);
  test_one (100000, CaloCompactCellTool::VERSION_502, cells, tool, rng, *arena, true);

  test_one (cells.size(), CaloCompactCellTool::VERSION_502, cells, tool, rng, *arena);

  rng.seed = 60;
  test_one (400, CaloCompactCellTool::VERSION_503, cells, tool, rng, *arena, false);
  test_one (400, CaloCompactCellTool::VERSION_503, cells, tool, rng, *arena,  true);

  test_one (10000, CaloCompactCellTool::VERSION_503, cells, tool, rng, *arena);
  test_one (10000, CaloCompactCellTool::VERSION_503, cells, tool, rng, *arena, true);

  test_one (100000, CaloCompactCellTool::VERSION_503, cells, tool, rng, *arena);
  test_one (100000, CaloCompactCellTool::VERSION_503, cells, tool, rng, *arena, true);

  test_one (cells.size(), CaloCompactCellTool::VERSION_503, cells, tool, rng, *arena);

  rng.seed = 70;
  test_one (400, CaloCompactCellTool::VERSION_504, cells, tool, rng, *arena, false);
  test_one (400, CaloCompactCellTool::VERSION_504, cells, tool, rng, *arena,  true);

  test_one (10000, CaloCompactCellTool::VERSION_504, cells, tool, rng, *arena);
  test_one (10000, CaloCompactCellTool::VERSION_504, cells, tool, rng, *arena, true);

  test_one (100000, CaloCompactCellTool::VERSION_504, cells, tool, rng, *arena);
  test_one (100000, CaloCompactCellTool::VERSION_504, cells, tool, rng, *arena, true);

  test_one (cells.size(), CaloCompactCellTool::VERSION_504, cells, tool, rng, *arena);

  rng.seed = 80;
  test_supercells (CaloCompactCellTool::VERSION_504, cont, tool, *arena);

  rng.seed = 30;
  test_errs (cells, tool, rng, *arena, mgr);

  // Test thinning.
  rng.seed = 100;
  SG::ThinningDecisionBase dec (cells.size());
  dec.keepAll();
  for (size_t i = 0; i < cells.size(); i++) {
    if ((i%3) == 0) dec.thin (i);
  }
  test_one (cells.size(), CaloCompactCellTool::VERSION_501, cells, tool, rng,
            *arena, false, false, true, &dec);
}


float tv_diff (const timeval& tv1, const timeval& tv2)
{
  return tv2.tv_sec - tv1.tv_sec + (tv2.tv_usec - tv1.tv_usec) * 1e-6;
}


void timetests (IdDictParser* parser, int nrep)
{
  Athena_test::URNG rng;
  CaloCompactCellTool tool;
  ISvcLocator* svcloc = nullptr;
  CaloDetDescrManager* mgr = nullptr;
  CaloSuperCellDetDescrManager* scmgr = nullptr;
  CaloCellContainer* contsc = nullptr;
  std::vector<CaloCell*> cells = init (parser, rng, svcloc, mgr, scmgr, contsc);
  CaloCellContainer* cont = fill_cells (10000, cells, true, true, rng);

  rusage ru0, ru1, ru2, ru3;

  CaloCompactCellContainer ccc;
  getrusage (RUSAGE_SELF, &ru0);
  for (int i=0; i < nrep; i++)
    assert (tool.getPersistent (*cont, &ccc, nullptr));
  getrusage (RUSAGE_SELF, &ru1);

  auto arena = std::make_unique<SG::Arena> ("arena");
  SG::Arena::Push push (*arena);
  CaloCellContainer* cont2 = new CaloCellContainer (SG::VIEW_ELEMENTS);
  getrusage (RUSAGE_SELF, &ru2);
  for (int i = 0; i < nrep; i++) {
    assert (tool.getTransient (ccc, cont2));
    cont2->clear();
    arena->reset();
  }
  getrusage (RUSAGE_SELF, &ru3);

  printf ("pack: %f  ", tv_diff (ru0.ru_utime, ru1.ru_utime));
  printf ("unpack: %f\n", tv_diff (ru2.ru_utime, ru3.ru_utime));
}


int main (int argc, char** argv)
{
  errorcheck::ReportMessage::hideErrorLocus (true);
  auto parser = std::make_unique<IdDictParser>();

  if (argc >= 3 && strcmp (argv[1], "-t") == 0) {
    timetests (parser.get(), atoi (argv[2]));
    return 0;
  }

  runtests (parser.get());

  return 0;
}
