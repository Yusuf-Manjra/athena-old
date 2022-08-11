/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EGSelectionToolWrapper.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Giovanni Marchiori (giovanni.marchiori@cern.ch)
//

#include "DerivationFrameworkEGamma/EGSelectionToolWrapper.h"
#include "PATCore/AcceptData.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"

namespace DerivationFramework {

EGSelectionToolWrapper::EGSelectionToolWrapper(const std::string& t,
                                               const std::string& n,
                                               const IInterface* p)
  : AthAlgTool(t, n, p)
{
  declareInterface<DerivationFramework::IAugmentationTool>(this);
}

StatusCode
EGSelectionToolWrapper::initialize()
{
  if (m_sgName.empty()) {
    ATH_MSG_ERROR(
      "No SG name provided for the output of EGElectronLikelihoodToolWrapper!");
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_tool.retrieve());

  if (!(m_fudgeMCTool.name().empty())) {
    ATH_CHECK(m_fudgeMCTool.retrieve());
  } else {
    m_fudgeMCTool.disable();
  }

  ATH_CHECK(m_ContainerName.initialize());
  //
  m_decoratorPass = m_ContainerName.key() + "." + m_sgName;
  m_decoratorIsEM = m_ContainerName.key() + "." + m_sgName + "IsEMValue";
  ATH_CHECK(m_decoratorPass.initialize());
  ATH_CHECK(m_decoratorIsEM.initialize());

  return StatusCode::SUCCESS;
}

StatusCode
EGSelectionToolWrapper::addBranches() const
{
  // retrieve container
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::EgammaContainer> particles{ m_ContainerName, ctx };

  // Decorators
  SG::WriteDecorHandle<xAOD::EgammaContainer, char> decoratorPass{
    m_decoratorPass, ctx
  };
  SG::WriteDecorHandle<xAOD::EgammaContainer, unsigned int> decoratorIsEM{
    m_decoratorIsEM, ctx
  };

  bool applyFF = (!m_fudgeMCTool.empty());
  // Write mask for each element and record to SG for subsequent selection
  for (const xAOD::Egamma* par : *particles) {
    const xAOD::Egamma* pCopy = par;
    if (applyFF) {
      xAOD::Type::ObjectType type = par->type();
      CP::CorrectionCode correctionCode = CP::CorrectionCode::Ok;
      if (type == xAOD::Type::Electron) {
        const xAOD::Electron* eg = static_cast<const xAOD::Electron*>(par);
        xAOD::Electron* el = nullptr;
        correctionCode = m_fudgeMCTool->correctedCopy(*eg, el);
        pCopy = el;
      } else {
        const xAOD::Photon* eg = static_cast<const xAOD::Photon*>(par);
        xAOD::Photon* ph = nullptr;
        correctionCode = m_fudgeMCTool->correctedCopy(*eg, ph);
        pCopy = ph;
      }
      if (correctionCode == CP::CorrectionCode::Ok) {
        // all OK
      } else if (correctionCode == CP::CorrectionCode::Error) {
        Error("addBranches()",
              "Error applying fudge factors to current photon");
      } else if (correctionCode == CP::CorrectionCode::OutOfValidityRange) {
        Warning(
          "addBranches()",
          "Current photon has no valid fudge factors due to out-of-range");
      } else {
        Warning(
          "addBranches()",
          "Unknown correction code %d from ElectronPhotonShowerShapeFudgeTool",
          (int)correctionCode);
      }
    }
    // compute the output of the selector
    asg::AcceptData theAccept(m_tool->accept(ctx, pCopy));
    unsigned int isEM = (unsigned int)theAccept.getCutResultInvertedBitSet()
                          .to_ulong(); // this should work for both the
                                       // cut-based and the LH selectors

    // decorate the original object
    if (m_cut == "") {
      bool pass_selection = (bool)theAccept;
      if (pass_selection) {
        decoratorPass(*par) = 1;
      } else {
        decoratorPass(*par) = 0;
      }
      decoratorIsEM(*par) = isEM;
    } else {
      if (theAccept.getCutResult(m_cut)) {
        decoratorPass(*par) = 1;
      } else {
        decoratorPass(*par) = 0;
      }
      decoratorIsEM(*par) = isEM;
    }

    // delete the particle copy
    if (applyFF) {
      delete pCopy;
    }
  }

  return StatusCode::SUCCESS;
}
}
