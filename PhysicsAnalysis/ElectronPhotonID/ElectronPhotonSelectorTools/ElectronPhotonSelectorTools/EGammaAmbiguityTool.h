/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-
#ifndef __EGammaAmbiguityTool__
#define __EGammaAmbiguityTool__

/**
   @class EGammaAmbiguityTool
   @brief Electron / photon ambiguity resolution. Dual-use version
   @authors Bruno Lenzi, Anastopoulos Christos, Ludovica Apperio Bella
   @date   May 2015
*/

// Atlas includes
#include "AsgTools/AsgTool.h"
#include "EgammaAnalysisInterfaces/IEGammaAmbiguityTool.h"
#include "xAODEgamma/EgammaContainerFwd.h"

class EGammaAmbiguityTool : public asg::AsgTool,
			    virtual public IEGammaAmbiguityTool
{
  ASG_TOOL_CLASS2(EGammaAmbiguityTool, IEGammaAmbiguityTool, asg::IAsgTool)

public: 
  /** Standard constructor */
  EGammaAmbiguityTool(const std::string& myname);


  /** Standard destructor */
  virtual ~EGammaAmbiguityTool() = default;


public:
  /** Gaudi Service Interface method implementations */
  virtual StatusCode initialize() override final;

   /** Return value: AuthorElectron, AuthorPhoton, AuthorAmbiguous, AuthorUnknown */

  virtual unsigned int ambiguityResolve(
    const xAOD::CaloCluster* cluster,
    const xAOD::Vertex* vx,
    const xAOD::TrackParticle* tp,
    xAOD::AmbiguityTool::AmbiguityType& type) const override final;

  /** Return value: AuthorElectron, AuthorPhoton, AuthorAmbiguous, AuthorUnknown
      Needed because of existing client usage (i.e Trigger). Implementation
     calls method above
  */
  virtual unsigned int ambiguityResolve(
    const xAOD::CaloCluster* cluster,
    const xAOD::Vertex* vx,
    const xAOD::TrackParticle* tp) const override final;

  /** Access the ambiguity resolution of central electrons and photons and
   * return AuthorElectron, AuthorPhoton, AuthorAmbiguous, AuthorUnknown
   * or the author of the object if no overlapping object is found **/
  virtual unsigned int ambiguityResolve(const xAOD::Egamma& egamma) const override final;

  /** Accept or reject egamma object based on ambiguity resolution 
    * (e.g. if object is a photon and ambiguity return value is electron -> reject) 
    **/
  virtual bool accept( const xAOD::Egamma& egamma) const override final;
  
  /** Return true if track has innermost pixel hit 
   * or next-to-innermost in case innermost is not expected
   * or at least m_MinNoPixHits pixel hits in case next-to-innermost is not expected
   **/
  bool hasInnermostPixelHit(const xAOD::TrackParticle& tp) const;
  
  /** Return the number of tracks with "innermost pixel hits" (see above) 
   * in the given vertex **/
  size_t nTrkWithInnermostPixelHits(const xAOD::Vertex& vx) const;
  
  /** Return true if the given TrackParticle is part of the vertex **/
  bool isInVertex(const xAOD::TrackParticle&, const xAOD::Vertex&) const;
  
  /** Return true if the vertex passes the requirement on Rconv - RfirstHit **/
  bool passDeltaR_innermost(const xAOD::Vertex& vx) const;
  
private:

  /** @brief Minimum number of silicon hits to be an electron and not a photon */
  int m_MinNoSiHits;
  /** @brief Minimum number of pixel hits to be an electron and not a photon */
  int m_MinNoPixHits;
  /** @brief Maximum EoverP, more that this is ambiguous */
  float  m_maxEoverPCut;
  /** @brief Minimum Pt, less that this is ambiguous */
  float m_minPtCut;
  /** @brief Maximum value for Rconv - RfirstHit for Si+Si conversions where both tracks have innermost hits **/
  float m_maxDeltaR_innermost;
  /** @brief no vertex && no innermost hit -> amb. Use true in reconstruction to have more ambiguous  **/
  bool m_noVertexNoInnermostAsAmb;
  /** @When used as a selector reject/accpet ambiguous cases */
  bool m_acceptAmbiguous;  
}; // End: class definition

#endif
