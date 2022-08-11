/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jeremy Dalseno                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtParticle.hh"

#include <fstream>

namespace Belle2 {
//! Register Decay model EvtB0toKsKK
  class EvtB0toKsKK : public  EvtDecayAmp {

  public:

    /* Constructor. */
    //EvtB0toKsKK() {}
    EvtB0toKsKK() : m_alpha_kpkmnr(0.0), m_alpha_kskpnr(0.0), m_alpha_kskmnr(0.0) {};

    /* Destructor. */
    virtual ~EvtB0toKsKK();

    std::string getName();  /**< Get function Name  */

    EvtDecayBase* clone();  /**< Clone the decay of B0toKsKK */

    void init();  /**< Initialize standard stream objects  */

    void initProbMax();  /**< Initialize standard stream objects for probability function  */

    void decay(EvtParticle* p); /**< Member of particle in EvtGen */

    EvtVector4R umu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                    const EvtVector4R& p4c);  /**< Function 4Vector umu */
    EvtVector4R Smu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                    const EvtVector4R& p4c);  /**< Function 4Vector Smu */
    EvtVector4R Lmu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                    const EvtVector4R& p4c);  /**< Function 4Vector Lmu */
    EvtTensor4C gmunu_tilde(const EvtVector4R& p4a,
                            const EvtVector4R& p4b,
                            const EvtVector4R& p4c); /**< Function Tensor gmunu  */
    EvtTensor4C Tmunu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                      const EvtVector4R& p4c);     /**< Function Tensor Tmunu  */
    EvtTensor4C Multiply(const EvtTensor4C& t1,
                         const EvtTensor4C& t2);   /**< Function Tensor Multiply  */
    EvtTensor4C RaiseIndices(const EvtTensor4C& t);  /**< Function RaiseIndices   */
    void RaiseIndex(EvtVector4R& vector);            /**< Member function RaiseIndices*/
    EvtTensor4C Mmunu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                      const EvtVector4R& p4c);       /**< Function Tensor Mmunu */
    double BWBF(const double& q, const unsigned int& L); /**< Meson radius  */
    double BWBF(const double& q, const double& q0,
                const unsigned int& L);              /**< Meson radius  */
    EvtComplex BreitWigner(const double& m, const double& m0,
                           const double& Gamma0,
                           const double& q, const double& q0,
                           const unsigned int& L);   /**< BreitWigner Shape */
    EvtVector4R Boost(const EvtVector4R& p4,
                      const EvtVector4R& boost);     /**< Parameter for boost frame  */
    double p(const double& mab, const double& M, const double& mc);  /**< Constants p  */
    double q(const double& mab, const double& ma, const double& mb); /**< Constants q */
    EvtComplex Flatte_k(const double& s, const double& m_h); /**< Constant Flatte_k */
    EvtComplex Flatte(const double& m, const double& m0);    /**< Constant Flatte */

    EvtComplex A_f0ks(const EvtVector4R& p4ks,
                      const EvtVector4R& p4kp, const EvtVector4R& p4km); /**< A_f0ks is amplitude of f0 */
    EvtComplex A_phiks(const EvtVector4R& p4ks,
                       const EvtVector4R& p4kp, const EvtVector4R& p4km); /**< A_phiks is amplitude of phi*/
    EvtComplex A_fxks(const EvtVector4R& p4ks,
                      const EvtVector4R& p4kp, const EvtVector4R& p4km); /**< A_fxks is amplitude of fxks */
    EvtComplex A_chic0ks(const EvtVector4R& p4ks,
                         const EvtVector4R& p4kp, const EvtVector4R& p4km); /**< A_chic0ks is amplitude of chic0ks */
    EvtComplex A_kknr(const EvtVector4R& p4k1, const EvtVector4R& p4k2,
                      const double& m_alpha_kk);    /**< A_kknr is amplitude of kknr */

  private:

    /**<Variable names for form factors*/
    EvtComplex m_a_f0ks_;      /**< Variable member  m_a_f0ks_   */
    EvtComplex m_a_phiks_;     /**< Variable member  m_a_phiks_  */
    EvtComplex m_a_fxks_;      /**< Variable member  m_a_fxks_   */
    EvtComplex m_a_chic0ks_;   /**< Variable member  m_a_chic0ks_*/
    EvtComplex m_a_kpkmnr_;    /**< Variable member  m_a_kpkmnr_ */
    EvtComplex m_a_kskpnr_;    /**< Variable member  m_a_kskpnr_ */
    EvtComplex m_a_kskmnr_;    /**< Variable member  m_a_kskmnr_ */

    EvtComplex m_abar_f0ks_;    /**< Variable member m_abar_f0ks_  */
    EvtComplex m_abar_phiks_;   /**< Variable member m_abar_phiks_ */
    EvtComplex m_abar_fxks_;    /**< Variable member m_abar_fxks_  */
    EvtComplex m_abar_chic0ks_; /**< Variable member m_abar_chic0ks_ */
    EvtComplex m_abar_kpkmnr_;  /**< Variable member m_abar_kpkmnr_  */
    EvtComplex m_abar_kskpnr_;  /**< Variable member m_abar_kskpnr_  */
    EvtComplex m_abar_kskmnr_;  /**< Variable member m_abar_kskmnr_  */

    double m_alpha_kpkmnr;      /**< Variable member m_alpha_kpkmnr */
    double m_alpha_kskpnr;      /**< Variable member m_alpha_kskpnr */
    double m_alpha_kskmnr;      /**< Variable member m_alpha_kskmnr */

    std::ofstream m_debugfile_; /**< debuging stream */
  }; //! end of EvtDecayAmp

} // Belle 2 Namespace
