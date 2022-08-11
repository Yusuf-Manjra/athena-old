/*
 *   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1CALO_VERSIONS_GBLOCK_V1_H
#define XAODTRIGL1CALO_VERSIONS_GBLOCK_V1_H

#include "xAODBase/IParticle.h"
#include "AthLinks/ElementLink.h"
#include "AthContainers/AuxElement.h"
#include "xAODTrigL1Calo/JGTower.h"
#include "xAODTrigL1Calo/JGTowerContainer.h"

namespace xAOD
{
  class GBlock_v1 : public IParticle
  {
  public:
    /// Default constructor
    GBlock_v1() = default;
    /// Copy constructor
    GBlock_v1(const GBlock_v1 &other);
    /// Assignment operator
    GBlock_v1 &operator=(const GBlock_v1 &other);
    /// Default destructor
    virtual ~GBlock_v1() override;

    /// The transverse momentum of the particle
    virtual double pt() const final;

    /// @brief The pseudorapidity (\f$\eta\f$) of the particle
    virtual double eta() const final;
    void setEta(double);

    /// @brief The azimuthal angle (\f$\phi\f$) of the particle
    virtual double phi() const final;
    void setPhi(double);

    /// @brief The pseudorapidity (\f$\eta\f$) of the particle
    float deta() const;
    void setdEta(float);

    /// @brief The azimuthal angle (\f$\phi\f$) of the particle
    float dphi() const;
    void setdPhi(float);

    float et() const;
    void setEt(float);

    /// @brief The invariant mass of the particle
    virtual double m() const final;

    /// The total energy of the particle
    virtual double e() const final;

    /// @brief The true rapidity (y) of the particle
    virtual double rapidity() const final;

    /// @brief Definition of the 4-momentum type
    typedef IParticle::FourMom_t FourMom_t;

    /// @brief The full 4-momentum of the particle as a TLoretzVector
    virtual FourMom_t p4() const final;

    /// @brief The type of the object as a simple enumeration, remains pure virtual in e/gamma.
    virtual Type::ObjectType type() const final;

    /// The element link to the seed tower of this gBlock
    const ElementLink<JGTowerContainer> &seedTowerLink() const;
    /// The seed tower for this gBlock
    const JGTower *seedTower() const;

    /// The number of towers that make up this gBlock (including the seed)
    std::size_t nTowers() const;

    /// Element links to the towers making up this gBlock (including the seed)
    const std::vector<ElementLink<JGTowerContainer>> &towerLinks() const;

    /// Get a link by number
    const JGTower *getTower(std::size_t idx) const;

    /// The area of the gBlock
    float area() const;

    /// Set the area
    void setArea(float);

    /// Set the seed link
    void setSeedTowerLink(const ElementLink<JGTowerContainer> &link);

    /// set the tower links
    void setTowerLinks(const std::vector<ElementLink<JGTowerContainer>> &links);
  }; //> end class GBlock_v1
} // namespace xAOD

#endif //> !XAODTRIGL1CALO_VERSIONS_GBLOCK_V1_H