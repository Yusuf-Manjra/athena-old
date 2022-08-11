/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_DIGITIZATION_TRTDIGCONDFAKEMAP_H
#define TRT_DIGITIZATION_TRTDIGCONDFAKEMAP_H

#include "TRTDigCondBase.h"
#include "CLHEP/Random/RandomEngine.h"

/**
 * "Fake" straw map until "real" map is known.
 */
class TRTDigCondFakeMap : public TRTDigCondBase {

public:
  /** Constructor */
  TRTDigCondFakeMap( const TRTDigSettings*,
                     const InDetDD::TRT_DetectorManager*,
                     const TRT_ID* trt_id,
                     int UseGasMix,
                     ToolHandle<ITRT_StrawStatusSummaryTool> sumTool
                     );

protected:

  void setStrawStateInfo(Identifier& TRT_Identifier,
                         const double& strawlength,
                         double& noiselevel,
                         double& relative_noiseamplitude,
                         CLHEP::HepRandomEngine *rndmEngine);

private:

  float m_average_noiselevel; /**< Average noise level     */

};

#endif
