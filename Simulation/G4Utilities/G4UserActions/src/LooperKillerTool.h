/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4USERACTIONS_G4UA_LOOPERKILLERTOOL_H
#define G4USERACTIONS_G4UA_LOOPERKILLERTOOL_H

// System includes
#include <string>

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "LooperKiller.h"

namespace G4UA
{

  /// @class LooperKillerTool
  /// @brief Tool which manages the LooperKiller action.
  ///
  /// Create the LooperKiller for each worker thread
  ///
  /// @author Andrea Di Simone
  ///
  class LooperKillerTool : public UserActionToolBase<LooperKiller>
  {

    public:

      /// Standard constructor
      LooperKillerTool(const std::string& type, const std::string& name,
		       const IInterface* parent);

      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;

    protected:

      /// Create action for this thread
      virtual std::unique_ptr<LooperKiller>
      makeAndFillAction(G4AtlasUserActions&) override final;

    private:

      /// Configuration parameters
      G4UA::LooperKiller::Config m_config;

  }; // class LooperKillerTool

} // namespace G4UA

#endif
