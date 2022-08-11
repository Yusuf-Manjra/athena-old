/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthBinding.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_EVENT_TRUTHBINDING_H
#define ISF_EVENT_TRUTHBINDING_H

// HepMC
#include "AtlasHepMC/GenParticle.h"

namespace ISF {

  /**
     @class TruthBinding
     @author Andreas.Salzburger -at- cern.ch , Elmar.Ritsch -at- cern.ch
  */

  class TruthBinding {
  public:
    /** No default constructor */
    TruthBinding() = delete;

    /** constructor setting all truth particle pointers to the given particle */
    inline TruthBinding(HepMC::GenParticlePtr allTruthP);
    /** constructor setting all truth particle pointers individually */
    inline TruthBinding(HepMC::GenParticlePtr truthP, HepMC::GenParticlePtr primaryTruthP, HepMC::GenParticlePtr genZeroTruthP);

    /** copy constructors */
    inline TruthBinding(const TruthBinding &rhs);
    inline TruthBinding(TruthBinding&& rhs);

    //** assignment operators */
    inline TruthBinding& operator=(const TruthBinding &rhs);
    inline TruthBinding& operator=(TruthBinding&& rhs);

    /** comparisons */
    inline bool operator==(const TruthBinding& rhs) const;
    inline bool isEqual(const TruthBinding& rhs) const;
    inline bool isIdent(const TruthBinding& rhs) const;

    /** destructor */
    inline ~TruthBinding();

    /** pointer to the particle in the simulation truth */
    inline HepMC::GenParticlePtr getTruthParticle() const;
    inline void                setTruthParticle(HepMC::GenParticlePtr p);

    /** pointer to the primary particle in the simulation truth */
    inline HepMC::GenParticlePtr getPrimaryTruthParticle() const;

    /** pointer to the simulation truth particle before any regeneration happened (eg. brem) */
    inline HepMC::GenParticlePtr getGenerationZeroTruthParticle() const;
    inline void                setGenerationZeroTruthParticle(HepMC::GenParticlePtr p);

  private:
    HepMC::GenParticlePtr   m_truthParticle{};               //!< pointer to particle in MC truth
    HepMC::GenParticlePtr   m_primaryTruthParticle{};        //!< pointer to corresponding primary (generator) particle
    HepMC::GenParticlePtr   m_generationZeroTruthParticle{}; //!< pointer to corresponding truth particle before any regenration
  };

} // end of namespace

/* implementation for inlined methods */
#include <ISF_Event/TruthBinding.icc>

#endif // ISF_EVENT_TRUTHBINDING_H
