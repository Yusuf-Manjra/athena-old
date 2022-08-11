// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL1CALOCNV_CMMCPHITSCNVTOOL_H
#define XAODTRIGL1CALOCNV_CMMCPHITSCNVTOOL_H

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthAlgTool.h"

// Local include(s):
#include "xAODTrigL1CaloCnv/ICMMCPHitsCnvTool.h"

namespace xAODMaker {

   /**
    *  @short Implementation for the ICMMCPHitsCnvTool interface
    *
    *         This is a simple tool that implements the converter function
    *         defined in ICMMCPHitsCnvTool.
    *
    * @author John Morris <john.morris@cern.ch>
    */
   class CMMCPHitsCnvTool : public AthAlgTool,
                            public virtual ICMMCPHitsCnvTool {

   public:
      /// Regular AlgTool constructor
      CMMCPHitsCnvTool( const std::string& type, const std::string& name,
                        const IInterface* parent );

      /// Function that fills an existing xAOD::EmTauRoIContainer
      virtual StatusCode convert( const DataVector<LVL1::CMMCPHits>* esd,
                                  xAOD::CMMCPHitsContainer* xaod ) override;

   }; // class CMMCPHitsCnvTool

} // namespace xAODMaker

#endif // XAODTRIGL1CALOCNV_CMMCPHITSCNVTOOL_H
