/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_GEOMODEL_TRTDETECTORTOOL_H
#define TRT_GEOMODEL_TRTDETECTORTOOL_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h" //for Argon
#include "CxxUtils/checker_macros.h"
#include <string>

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Please consult the README for more information about which options to set in your joboptions file. //
////////////////////////////////////////////////////////////////////////////////////////////////////////

class IGeoDbTagSvc;
class IGeometryDBSvc;

namespace InDetDD {
  class TRT_DetectorManager;
  class AthenaComps;
}

class TRT_DetectorTool final : public GeoModelTool {

public:

    // Standard Constructor
    TRT_DetectorTool( const std::string& type, const std::string& name, const IInterface* parent );

    // Standard Destructor
    virtual ~TRT_DetectorTool() override final;

    virtual StatusCode create() override final;
    virtual StatusCode clear() override final;

    // Register callback function on CondDB object
    virtual StatusCode registerCallback ATLAS_NOT_THREAD_SAFE () override final;

    // Callback function itself
    virtual StatusCode align(IOVSVC_CALLBACK_ARGS) override final;

 private:

    bool m_useOldActiveGasMixture;
    bool m_DC2CompatibleBarrelCoordinates;
    bool m_initialLayout;
    int m_overridedigversion;
    bool m_alignable;

    ServiceHandle< IGeoDbTagSvc > m_geoDbTagSvc;
    ServiceHandle< IGeometryDBSvc > m_geometryDBSvc;
    ToolHandle<ITRT_StrawStatusSummaryTool> m_sumTool; // added for Argon
 
    bool m_doArgonMixture;
    bool m_doKryptonMixture;
    bool m_useDynamicAlignFolders;
 
    const InDetDD::TRT_DetectorManager * m_manager;
    InDetDD::AthenaComps * m_athenaComps;

};

#endif // TRT_GEOMODEL_TRTDETECTORTOOL_H
