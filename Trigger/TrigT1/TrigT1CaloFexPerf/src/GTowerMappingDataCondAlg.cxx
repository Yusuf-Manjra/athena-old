#include "GTowerMappingDataCondAlg.h"
#include "TMath.h"
#include "TVector2.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "GTowerHelpers.h"

namespace LVL1
{
  GTowerMappingDataCondAlg::GTowerMappingDataCondAlg(const std::string &name, ISvcLocator *pSvcLocator)
      : JGTowerMappingDataCondAlgBase(name, pSvcLocator)
  {
  }

  GTowerMappingDataCondAlg::~GTowerMappingDataCondAlg() {}

  StatusCode GTowerMappingDataCondAlg::initialize()
  {
    ATH_CHECK(JGTowerMappingDataCondAlgBase::initialize());
    ATH_CHECK(detStore()->retrieve(m_gtowerID));
    return StatusCode::SUCCESS;
  }

  StatusCode GTowerMappingDataCondAlg::buildForwardMapping(
      JGTowerMappingData &data,
      const CaloSuperCellDetDescrManager *scDetMgr) const
  {
    // Define GTower geometry
    std::array<float, 5> etaBounds{3.1, 3.5, 4.0, 4.45, 4.9};
    constexpr std::size_t nEtaTowers = etaBounds.size() - 1;
    constexpr std::size_t nPhiTowers = 16;
    float dPhi = 2 * TMath::Pi() / nPhiTowers;
    for (std::size_t iEta = 0; iEta < etaBounds.size() - 1; ++iEta)
    {
      float eta = (etaBounds[iEta] + etaBounds[iEta + 1]) / 2;
      float dEta = etaBounds[iEta + 1] - etaBounds[iEta];

      for (int sign : {1, -1})
      {
        for (std::size_t iPhi = 0; iPhi < nPhiTowers; ++iPhi)
        {
          std::size_t index = data.size();
          float phi = dPhi * (iPhi + 0.5);
          if (phi > TMath::Pi())
            phi -= 2 * TMath::Pi();
          JGTowerHelper helper(eta * sign, dEta, phi, dPhi);
          helper.SetSampling(2);
          for (std::size_t scIdx = 0; scIdx < m_scid->calo_cell_hash_max(); ++scIdx)
          {
            Identifier scid = m_scid->cell_id(scIdx);
            if (m_scid->is_tile(scid) && m_scid->sampling(scid) != 2)
              continue;
            const CaloDetDescrElement *dde = scDetMgr->get_element(scid);
            if (!dde)
            {
              ATH_MSG_ERROR("Failed to load CaloDetDescrElement");
              return StatusCode::FAILURE;
            }
            if (std::abs(dde->eta_raw()) < 3.2)
              continue;
            if (helper.inBox(dde->eta_raw(), dde->phi_raw()) && sign * m_scid->pos_neg(scid) > 0)
              helper.SetSCIndices(scIdx);
          }
          data.push_back(std::move(helper));
          if (iEta > 0)
          {
            if (sign > 0)
              helper.setPreviousEtaIndex(index - 2 * nPhiTowers);
            else
              helper.setNextEtaIndex(index - 2 * nPhiTowers);
          }
          if (iEta < nEtaTowers - 1)
          {
            if (sign > 0)
              helper.setNextEtaIndex(index - 2 * nPhiTowers);
            else
              helper.setPreviousEtaIndex(index - 2 * nPhiTowers);
          }
          helper.setPreviousPhiIndex(iPhi == 0 ? index + nPhiTowers - 1 : index - 1);
          helper.setNextPhiIndex(iPhi == nPhiTowers - 1 ? index - (nPhiTowers - 1) : index + 1);
        }
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode GTowerMappingDataCondAlg::loadTowerAreas(JGTowerMappingData &data) const
  {
    for (JGTowerHelper &helper : data)
      helper.setArea(gFEX::towerArea(helper.Eta()));
    return StatusCode::SUCCESS;
  }
} // namespace LVL1