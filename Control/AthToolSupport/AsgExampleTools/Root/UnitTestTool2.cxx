/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgExampleTools/UnitTestTool2.h>

#include <AsgExampleTools/UnitTestTool1.h>

//
// method implementations
//

namespace asg
{
  UnitTestTool2 ::
  UnitTestTool2 (const std::string& val_name)
    : AsgTool (val_name),
      m_regPublicHandle ("", nullptr),
      m_regPrivateHandle ("", this),
      m_anaPublicHandle ("asg::UnitTestTool1/anaPublicHandle", nullptr),
      m_anaPrivateHandle ("asg::UnitTestTool1/anaPrivateHandle", this)
  {
    declareProperty ("allowEmpty", m_allowEmpty);

    declareProperty ("regPublicHandle", m_regPublicHandle);
    declareProperty ("regPrivateHandle", m_regPrivateHandle);
    m_anaPublicHandle.declarePropertyFor (this, "anaPublicHandle");
    m_anaPrivateHandle.declarePropertyFor (this, "anaPrivateHandle");
  }



  StatusCode UnitTestTool2 ::
  initialize ()
  {
    m_anaPublicHandle.setAllowEmpty (m_allowEmpty);
    m_anaPrivateHandle.setAllowEmpty (m_allowEmpty);

    m_wasUserConfiguredPublic = m_anaPublicHandle.isUserConfigured();
    ANA_CHECK (m_anaPublicHandle.setProperty ("propertyInt", 111));
    ANA_CHECK (m_anaPublicHandle.initialize ());

    m_wasUserConfiguredPrivate = m_anaPrivateHandle.isUserConfigured();
    ANA_CHECK (m_anaPrivateHandle.setProperty ("propertyInt", 222));
    ANA_CHECK (m_anaPrivateHandle.initialize ());

    for (auto& handle : m_regPrivateArray)
      ANA_CHECK (handle.retrieve());

    return StatusCode::SUCCESS;
  }



  bool UnitTestTool2 ::
  toolHandleEmpty (const std::string& handleName) const
  {
    if (handleName == "regPublicHandle")
      return m_regPublicHandle.empty();
    if (handleName == "regPrivateHandle")
      return m_regPrivateHandle.empty();
    if (handleName == "anaPublicHandle")
      return m_anaPublicHandle.empty();
    if (handleName == "anaPrivateHandle")
      return m_anaPrivateHandle.empty();
    throw std::runtime_error ("unknown handle: " + handleName);
  }



  const IUnitTestTool1 *UnitTestTool2 ::
  getToolHandle (const std::string& handleName) const
  {
    if (handleName == "regPublicHandle")
      return &*m_regPublicHandle;
    if (handleName == "regPrivateHandle")
      return &*m_regPrivateHandle;
    if (handleName == "anaPublicHandle")
      return &*m_anaPublicHandle;
    if (handleName == "anaPrivateHandle")
      return &*m_anaPrivateHandle;
    throw std::runtime_error ("unknown handle: " + handleName);
  }



  StatusCode UnitTestTool2 ::
  retrieveToolHandle (const std::string& handleName) const
  {
    if (handleName == "regPublicHandle")
      return m_regPublicHandle.retrieve ();
    if (handleName == "regPrivateHandle")
      return m_regPrivateHandle.retrieve ();
    throw std::runtime_error ("unknown handle: " + handleName);
  }



  bool UnitTestTool2 ::
  wasUserConfigured (const std::string& handleName) const
  {
    if (handleName == "anaPublicHandle")
      return m_wasUserConfiguredPublic;
    if (handleName == "anaPrivateHandle")
      return m_wasUserConfiguredPrivate;
    throw std::runtime_error ("unknown handle: " + handleName);
  }



  const ToolHandleArray<IUnitTestTool1>& UnitTestTool2 ::
  getArray () const
  {
    return m_regPrivateArray;
  }
}
