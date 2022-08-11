/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauJetRNNEvaluator.h"
#include "tauRecTools/TauJetRNN.h"
#include "tauRecTools/HelperFunctions.h"

#include "PathResolver/PathResolver.h"

#include <algorithm>


TauJetRNNEvaluator::TauJetRNNEvaluator(const std::string &name): 
  TauRecToolBase(name),
  m_net_0p(nullptr),
  m_net_1p(nullptr),
  m_net_2p(nullptr),
  m_net_3p(nullptr) {
    
  declareProperty("NetworkFile0P", m_weightfile_0p = "");
  declareProperty("NetworkFile1P", m_weightfile_1p = "");
  declareProperty("NetworkFile2P", m_weightfile_2p = "");
  declareProperty("NetworkFile3P", m_weightfile_3p = "");
  declareProperty("OutputVarname", m_output_varname = "RNNJetScore");
  declareProperty("MaxTracks", m_max_tracks = 10);
  declareProperty("MaxClusters", m_max_clusters = 6);
  declareProperty("MaxClusterDR", m_max_cluster_dr = 1.0f);
  declareProperty("VertexCorrection", m_doVertexCorrection = true);
  declareProperty("TrackClassification", m_doTrackClassification = true);

  // Naming conventions for the network weight files:
  declareProperty("InputLayerScalar", m_input_layer_scalar = "scalar");
  declareProperty("InputLayerTracks", m_input_layer_tracks = "tracks");
  declareProperty("InputLayerClusters", m_input_layer_clusters = "clusters");
  declareProperty("OutputLayer", m_output_layer = "rnnid_output");
  declareProperty("OutputNode", m_output_node = "sig_prob");
  }

TauJetRNNEvaluator::~TauJetRNNEvaluator() {}

StatusCode TauJetRNNEvaluator::initialize() {
  ATH_MSG_INFO("Initializing TauJetRNNEvaluator");
  
  std::string weightfile_0p("");
  std::string weightfile_1p("");
  std::string weightfile_2p("");
  std::string weightfile_3p("");

  // Use PathResolver to search for the weight files
  if (!m_weightfile_0p.empty()) {
    weightfile_0p = find_file(m_weightfile_0p);
    if (weightfile_0p.empty()) {
      ATH_MSG_ERROR("Could not find network weights: " << m_weightfile_0p);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO("Using network config [0-prong]: " << weightfile_0p);
    }
  }

  if (!m_weightfile_1p.empty()) {
    weightfile_1p = find_file(m_weightfile_1p);
    if (weightfile_1p.empty()) {
      ATH_MSG_ERROR("Could not find network weights: " << m_weightfile_1p);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO("Using network config [1-prong]: " << weightfile_1p);
    }
  }

  if (!m_weightfile_2p.empty()) {
    weightfile_2p = find_file(m_weightfile_2p);
    if (weightfile_2p.empty()) {
      ATH_MSG_ERROR("Could not find network weights: " << m_weightfile_2p);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO("Using network config [2-prong]: " << weightfile_2p);
    }
  }

  if (!m_weightfile_3p.empty()) {
    weightfile_3p = find_file(m_weightfile_3p);
    if (weightfile_3p.empty()) {
      ATH_MSG_ERROR("Could not find network weights: " << m_weightfile_3p);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO("Using network config [3-prong]: " << weightfile_3p);
    }
  }

  // Set the layer and node names in the weight file
  TauJetRNN::Config config;
  config.input_layer_scalar = m_input_layer_scalar;
  config.input_layer_tracks = m_input_layer_tracks;
  config.input_layer_clusters = m_input_layer_clusters;
  config.output_layer = m_output_layer;
  config.output_node = m_output_node;

  // Load the weights and create the network
  // 0p is for trigger only
  if (!weightfile_0p.empty()) {
    m_net_0p = std::make_unique<TauJetRNN>(weightfile_0p, config);
    if (!m_net_0p) {
      ATH_MSG_ERROR("No network configured for 0-prong taus.");
      return StatusCode::FAILURE;
    }
  }

  m_net_1p = std::make_unique<TauJetRNN>(weightfile_1p, config);
  if (!m_net_1p) {
    ATH_MSG_ERROR("No network configured for 1-prong taus.");
    return StatusCode::FAILURE;
  }

  // 2p is optional
  if (!weightfile_2p.empty()) {
    m_net_2p = std::make_unique<TauJetRNN>(weightfile_2p, config);
    if (!m_net_2p) {
      ATH_MSG_ERROR("No network configured for 2-prong taus.");
      return StatusCode::FAILURE;
    }
  }

  m_net_3p = std::make_unique<TauJetRNN>(weightfile_3p, config);      
  if (!m_net_3p) {
    ATH_MSG_ERROR("No network configured for 3-prong taus.");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode TauJetRNNEvaluator::execute(xAOD::TauJet &tau) const {
  // Output variable accessor
  const SG::AuxElement::Accessor<float> output(m_output_varname);

  // Set default score and overwrite later
  output(tau) = -1111.0f;

  const auto nTracksCharged = tau.nTracksCharged();

  // Get input objects
  std::vector<const xAOD::TauTrack *> tracks;
  ATH_CHECK(get_tracks(tau, tracks));
  std::vector<xAOD::CaloVertexedTopoCluster> clusters;
  ATH_CHECK(get_clusters(tau, clusters));

  // Evaluate networks
  if (nTracksCharged==0 && m_net_0p) {
    output(tau) = m_net_0p->compute(tau, tracks, clusters);
  }
  else if (nTracksCharged == 1) {
    output(tau) = m_net_1p->compute(tau, tracks, clusters);
  }
  else if (nTracksCharged == 2) {
    if(m_net_2p) {
      output(tau) = m_net_2p->compute(tau, tracks, clusters);
    } else {
      output(tau) = m_net_3p->compute(tau, tracks, clusters);
    }
  }
  else if (nTracksCharged > 2) {
    output(tau) = m_net_3p->compute(tau, tracks, clusters);
  }

  return StatusCode::SUCCESS;
}

const TauJetRNN* TauJetRNNEvaluator::get_rnn_0p() const {
  return m_net_0p.get();
}

const TauJetRNN* TauJetRNNEvaluator::get_rnn_1p() const {
  return m_net_1p.get();
}

const TauJetRNN* TauJetRNNEvaluator::get_rnn_2p() const {
  return m_net_2p.get();
}

const TauJetRNN* TauJetRNNEvaluator::get_rnn_3p() const {
  return m_net_3p.get();
}

StatusCode TauJetRNNEvaluator::get_tracks(const xAOD::TauJet &tau, std::vector<const xAOD::TauTrack *> &out) const {
  std::vector<const xAOD::TauTrack*> tracks = tau.allTracks();

  // Skip unclassified tracks:
  // - the track is a LRT and classifyLRT = false
  // - the track is not among the MaxNtracks highest-pt tracks in the track classifier
  // - track classification is not run (trigger)
  if(m_doTrackClassification) {
    std::vector<const xAOD::TauTrack*>::iterator it = tracks.begin();
    while(it != tracks.end()) {
      if((*it)->flag(xAOD::TauJetParameters::unclassified)) {
	it = tracks.erase(it);
      }
      else {
	++it;
      }
    }
  }

  // Sort by descending pt
  auto cmp_pt = [](const xAOD::TauTrack *lhs, const xAOD::TauTrack *rhs) {
    return lhs->pt() > rhs->pt();
  };
  std::sort(tracks.begin(), tracks.end(), cmp_pt);

  // Truncate tracks
  if (tracks.size() > m_max_tracks) {
    tracks.resize(m_max_tracks);
  }
  out = std::move(tracks);

  return StatusCode::SUCCESS;
}

StatusCode TauJetRNNEvaluator::get_clusters(const xAOD::TauJet &tau, std::vector<xAOD::CaloVertexedTopoCluster> &clusters) const {

  TLorentzVector tauAxis = tauRecTools::getTauAxis(tau, m_doVertexCorrection);

  std::vector<xAOD::CaloVertexedTopoCluster> vertexedClusterList = tau.vertexedClusters();
  for (const xAOD::CaloVertexedTopoCluster& vertexedCluster : vertexedClusterList) {
    TLorentzVector clusterP4 = vertexedCluster.p4();
    if (clusterP4.DeltaR(tauAxis) > m_max_cluster_dr) continue;
      
    clusters.push_back(vertexedCluster);
  }

  // Sort by descending et
  auto et_cmp = [](const xAOD::CaloVertexedTopoCluster& lhs,
		   const xAOD::CaloVertexedTopoCluster& rhs) {
    return lhs.p4().Et() > rhs.p4().Et();
  };
  std::sort(clusters.begin(), clusters.end(), et_cmp);

  // Truncate clusters
  if (clusters.size() > m_max_clusters) {
    clusters.resize(m_max_clusters, clusters[0]);
  }

  return StatusCode::SUCCESS;
}
