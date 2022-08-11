/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "LArClusterRec/LArClusterCollisionTimeAlg.h"

#include <algorithm>

//__________________________________________________________________________
StatusCode LArClusterCollisionTimeAlg::initialize()
  {
    ATH_MSG_DEBUG ("LArClusterCollisionTimeAlg initialize()");
    ATH_CHECK( m_clusterContainerName.initialize() );
    ATH_CHECK( m_outputName.initialize() );
    return StatusCode::SUCCESS; 

  }

//__________________________________________________________________________
StatusCode LArClusterCollisionTimeAlg::finalize()
  {
   
    ATH_MSG_INFO( m_nCollEvt << "/" << m_nEvt << " Events found to be collision events with at least one cluster in each EM endcap."  );
    ATH_MSG_DEBUG ("LArClusterCollisionTimeAlg finalize()");
    return StatusCode::SUCCESS; 
  }
  
LArClusterCollisionTimeAlg::perSide_t LArClusterCollisionTimeAlg::analyseClustersPerSide(std::vector<const xAOD::CaloCluster*>& clusters) const {

  perSide_t result;

  //Sort clusters by Energy
  std::sort(clusters.begin(),clusters.end(),[](const xAOD::CaloCluster* o1, const xAOD::CaloCluster* o2) {return o1->e() > o2->e();}); 

  result.nClusters=std::min(m_maxClusters.value(),clusters.size());
  
  if (result.nClusters>0) {
    for (size_t i=0;i<result.nClusters;++i) {
      const xAOD::CaloCluster* clu=clusters[i];
      result.time+=clu->time();
      result.energy+=clu->e();
    }

    result.time/=(float)(result.nClusters);
    result.energy/=(float)(result.nClusters);
  }

  return result;
}
  
//__________________________________________________________________________
StatusCode LArClusterCollisionTimeAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG ("LArClusterCollisionTimeAlg execute()");

  m_nEvt++;
  
  // Get the cluster container
  SG::ReadHandle<xAOD::CaloClusterContainer> cluster_container (m_clusterContainerName,ctx);
  if( !cluster_container.isValid()) { // record empty object
    ATH_MSG_WARNING ("Could not get ClusterContainer with key " << m_clusterContainerName);
     // Construct the output object
     SG::WriteHandle<LArCollisionTime> larTime (m_outputName,ctx);
     ATH_CHECK( larTime.record (std::make_unique<LArCollisionTime>()) );
     return StatusCode::SUCCESS;
  }

  std::vector<const xAOD::CaloCluster*> clustersEMECA,clustersEMECC;

  const unsigned barrelPattern=CaloSampling::barrelPattern();
  
  for(const xAOD::CaloCluster* pCluster :  *cluster_container) {
    if ((pCluster->samplingPattern() & barrelPattern) == 0 ) { //Take only clusters that have only endcap samplings, ignore barrel
      if (pCluster->eta()>0.)
	clustersEMECA.push_back(pCluster);
      else
	clustersEMECC.push_back(pCluster);
    }
  } //End loop over clusters

  const perSide_t sideA=analyseClustersPerSide(clustersEMECA);
  const perSide_t sideC=analyseClustersPerSide(clustersEMECC);

  auto tmplarTime = std::make_unique<LArCollisionTime>(sideA.nClusters,sideC.nClusters,sideA.energy,sideC.energy,sideA.time,sideC.time);
  // Construct the output object
  SG::WriteHandle<LArCollisionTime> larTime (m_outputName,ctx);
  if (! larTime.put (std::move (tmplarTime))  )  {
     ATH_MSG_WARNING( "Could not record the LArCollisionTime object with key "<<m_outputName );
  }

  ATH_MSG_DEBUG("Number/Time/Energy, Side A: " << sideA.nClusters << "/" << sideA.time << "/" << sideA.energy);
  ATH_MSG_DEBUG("Number/Time/Energy, Side C: " << sideC.nClusters << "/" << sideC.time << "/" << sideC.energy);
  
  if ( sideA.nClusters>0 && sideC.nClusters>0 && std::fabs(sideA.time-sideC.time)<m_timeCut) {
    setFilterPassed(true,ctx);
    m_nCollEvt++;
  }
  else {
    setFilterPassed(false,ctx);
  }

  
  return StatusCode::SUCCESS;
}
