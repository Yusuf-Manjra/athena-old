/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "xAODBTagging/BTaggingUtilities.h" 

#include <cmath>
#include <cstddef>

namespace str {
  const std::string d0Uncertainty = "d0Uncertainty";
  const std::string z0SinThetaUncertainty = "z0SinThetaUncertainty";
  const std::string trackDisplacement = "trackDisplacement";
  const std::string trackMomentum = "trackMomentum";
}

BTagTrackIpAccessor::BTagTrackIpAccessor(const std::string& prefix):
  m_ip_d0_sigma(prefix + str::d0Uncertainty),
  m_ip_z0_sigma(prefix + str::z0SinThetaUncertainty),
  m_track_displacement(prefix + str::trackDisplacement),
  m_track_momentum(prefix + str::trackMomentum),
  m_ip2d_trackParticleLinks("IP2D_TrackParticleLinks"),
  m_ip3d_trackParticleLinks("IP3D_TrackParticleLinks"),
  m_ip2d_gradeOfTracks("IP2D_gradeOfTracks"),
  m_ip3d_gradeOfTracks("IP3D_gradeOfTracks"),
  m_ip2d_signed_d0("IP2D_signed_d0"),
  m_ip3d_signed_d0("IP3D_signed_d0"),
  m_ip3d_signed_z0("IP3D_signed_z0"),
  m_ip3d_signed_d0_significance("IP3D_signed_d0_significance"),
  m_ip3d_signed_z0_significance("IP3D_signed_z0_significance"),
  m_ip2d_grade("IP2D_grade"),
  m_ip3d_grade("IP3D_grade"),
  m_prefix(prefix)
{
}

namespace {
  Amg::Vector3D get_vector3d(const std::vector<float> &input_vector) {
    if (input_vector.size() != 3) {
      std::string size = std::to_string(input_vector.size());
      throw std::logic_error(
        "Tried to build an Eigen 3-vector from " + size + " elements");
    }
    return Eigen::Vector3f(input_vector.data()).cast<double>();
  }
}

void BTagTrackIpAccessor::augment(const xAOD::TrackParticle &track, const xAOD::Jet &jet) const {
  augment_with_grades(track, jet);
  augment_with_ip(track, jet);
}

BTagSignedIP BTagTrackIpAccessor::getSignedIp(const xAOD::TrackParticle &track, const xAOD::Jet &jet) const {
  const TLorentzVector jet_fourVector = jet.p4();
  const Amg::Vector3D jet_threeVector(jet_fourVector.X(),jet_fourVector.Y(),jet_fourVector.Z());
  const Amg::Vector3D track_displacement = get_vector3d(m_track_displacement(track));
  const Amg::Vector3D track_momentum = get_vector3d(m_track_momentum(track));

  BTagSignedIP ip;
  const double ip_d0 = d0(track);
  ip.ip2d_signed_d0 = std::copysign(ip_d0, std::sin(jet_threeVector.phi() - track_momentum.phi()) * ip_d0);
  const double ip3d_signed_d0 = std::copysign(ip_d0, jet_threeVector.cross(track_momentum).dot(track_momentum.cross(-track_displacement)));
  ip.ip3d_signed_d0 = ip3d_signed_d0;
  ip.ip3d_signed_d0_significance = ip3d_signed_d0 / m_ip_d0_sigma(track);

  const double ip_z0 = z0SinTheta(track);
  const double signed_z0 = std::copysign(ip_z0, (jet_threeVector.eta() - track_momentum.eta()) * ip_z0);
  ip.ip3d_signed_z0 = signed_z0;
  ip.ip3d_signed_z0_significance = signed_z0 / m_ip_z0_sigma(track);
  return ip;
}

double BTagTrackIpAccessor::d0(const xAOD::TrackParticle &track) const {
  const Amg::Vector3D track_displacement = get_vector3d(m_track_displacement(track));
  const Amg::Vector3D track_momentum = get_vector3d(m_track_momentum(track));
  return std::copysign(
    std::hypot(track_displacement[Amg::x], track_displacement[Amg::y]),
    track_momentum.cross(track_displacement)[Amg::z]);
}
double BTagTrackIpAccessor::d0Uncertainty(const xAOD::TrackParticle &track)
  const {
  return m_ip_d0_sigma(track);
}
double BTagTrackIpAccessor::z0SinTheta(const xAOD::TrackParticle &track) const {
  const Amg::Vector3D track_displacement = get_vector3d(m_track_displacement(track));
  const Amg::Vector3D track_momentum = get_vector3d(m_track_momentum(track));
  return track_displacement[Amg::z] * std::sqrt(
    1 - std::pow(track_momentum[Amg::z],2) / track_momentum.squaredNorm());
}
double BTagTrackIpAccessor::z0SinThetaUncertainty(const xAOD::TrackParticle &track) const {
  return m_ip_z0_sigma(track);
}

void BTagTrackIpAccessor::augment_with_ip(const xAOD::TrackParticle &track, const xAOD::Jet &jet) const {
  BTagSignedIP ip = getSignedIp(track, jet);
  m_ip2d_signed_d0(track) = ip.ip2d_signed_d0;
  m_ip3d_signed_d0(track) = ip.ip3d_signed_d0;
  m_ip3d_signed_d0_significance(track) = ip.ip3d_signed_d0_significance;
  m_ip3d_signed_z0(track) = ip.ip3d_signed_z0;
  m_ip3d_signed_z0_significance(track) = ip.ip3d_signed_z0_significance;
}
void BTagTrackIpAccessor::augment_with_grades(const xAOD::TrackParticle &track, const xAOD::Jet &jet) const {
  int ip3d_grade = -1;
  const xAOD::BTagging &btagging = *xAOD::BTaggingUtilities::getBTagging( jet );
  const std::vector<ElementLink<xAOD::TrackParticleContainer> > ip3d_tracks = m_ip3d_trackParticleLinks(btagging);
  for (std::size_t ip3d_track_index = 0; ip3d_track_index < ip3d_tracks.size(); ++ip3d_track_index) {
    if (&track == *(ip3d_tracks.at(ip3d_track_index))) {
      ip3d_grade = m_ip3d_gradeOfTracks(btagging).at(ip3d_track_index);
      break;
    }
  }
  m_ip3d_grade(track) = ip3d_grade;

  int ip2d_grade = -1;
  const std::vector<ElementLink<xAOD::TrackParticleContainer> > ip2d_tracks = m_ip2d_trackParticleLinks(btagging);
  for (std::size_t ip2d_track_index = 0; ip2d_track_index < ip2d_tracks.size(); ++ip2d_track_index) {
    if (&track == *(ip2d_tracks.at(ip2d_track_index))) {
      ip2d_grade = m_ip2d_gradeOfTracks(btagging).at(ip2d_track_index);
      break;
    }
  }
  m_ip2d_grade(track) = ip2d_grade;
}

std::set<std::string> BTagTrackIpAccessor::getTrackIpDataDependencyNames() const
{
  return {
    m_prefix + str::d0Uncertainty,
    m_prefix + str::z0SinThetaUncertainty,
    m_prefix + str::trackDisplacement,
    m_prefix + str::trackMomentum};
}

