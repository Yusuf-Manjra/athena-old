/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#include "PhysicsValidationUserActionTool.h"

namespace G4UA
{

  namespace iGeant4
  {

    PhysicsValidationUserActionTool::
    PhysicsValidationUserActionTool(const std::string& type,
                                    const std::string& name,
                                    const IInterface* parent)
      : UserActionToolBase<PhysicsValidationUserAction>(type, name, parent)
    {
      declareProperty("ValidationOutput",
                      m_config.validationOutput,
                      "If turned on, write out a ROOT tree.");
      declareProperty("ValidationStreamName",
                      m_config.validationStream,
                      "Name of the output stream");
      declareProperty("THistService",
                      m_config.thistSvc,
                      "The THistSvc");
      declareProperty("GeoIDSvc"      , m_config.geoIDSvc      , "ISF GeoID Svc"          );
      declareProperty("ID_Rmax",m_config.idR);
      declareProperty("ID_Zmax",m_config.idZ);
      declareProperty("CaloRmean",m_config.caloRmean);
      declareProperty("CaloZmean",m_config.caloZmean);
      declareProperty("MuonRmean",m_config.muonRmean);
      declareProperty("MuonZmean",m_config.muonZmean);
      declareProperty("CavernRmean",m_config.cavernRmean);
      declareProperty("CavernZmean",m_config.cavernZmean);
    }

    std::unique_ptr<PhysicsValidationUserAction>
    PhysicsValidationUserActionTool::makeAndFillAction(G4AtlasUserActions& actionList)
    {
      ATH_MSG_DEBUG("Constructing a PhysicsValidationUserAction");
      if(msgLvl(MSG::VERBOSE))      { m_config.verboseLevel = MSG::VERBOSE; }
      else if(msgLvl(MSG::DEBUG))   { m_config.verboseLevel = MSG::DEBUG;   }
      else if(msgLvl(MSG::INFO))    { m_config.verboseLevel = MSG::INFO;    }
      else if(msgLvl(MSG::WARNING)) { m_config.verboseLevel = MSG::WARNING; }
      else if(msgLvl(MSG::ERROR))   { m_config.verboseLevel = MSG::ERROR;   }
      else if(msgLvl(MSG::FATAL))   { m_config.verboseLevel = MSG::FATAL;   }
      auto action = std::make_unique<PhysicsValidationUserAction>(m_config);
      actionList.runActions.push_back( action.get() );
      actionList.eventActions.push_back( action.get() );
      actionList.trackingActions.push_back( action.get() );
      actionList.steppingActions.push_back( action.get() );
      return action;
    }

  } // iGeant4

} // namespace G4UA
