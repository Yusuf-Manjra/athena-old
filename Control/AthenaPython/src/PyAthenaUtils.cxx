///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// PyAthenaUtils.cxx 
// Implementation file for various PyAthena helpers
// Author: S.Binet<binet@cern.ch>
// Modified: Wim Lavrijsen <WLavrijsen@lbl.gov>
/////////////////////////////////////////////////////////////////// 

// Python includes
#include "Python.h"

// AthenaPython includes
#include "AthenaPython/PyAthenaUtils.h"
#include "RootUtils/PyAthenaGILStateEnsure.h"
#include "RootUtils/PyGetString.h"

// Framework includes
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/StatusCode.h"

// PyROOT includes
#include "TPython.h"
#include "CPyCppyy/PyException.h"

// Reflex includes
#include "DataModelRoot/RootType.h"

// ROOT includes
#include "TClassRef.h"

// STL includes
#include <string>
#include <iostream>
#include <sstream>

namespace {

  struct MyObjProxy {
    PyObject_HEAD
    void* m_object;
    TClassRef m_class;
  };

StatusCode 
fetchInterfaceId( PyObject* klass, 
		  unsigned long& id,
		  unsigned long& major,
		  unsigned long& minor )
{
  Py_INCREF( klass );

  RootUtils::PyGILStateEnsure ensure;
  PyObject* idObj = PyObject_CallMethod( klass, 
					 const_cast<char*>("interfaceID"), 
					 const_cast<char*>("") );
  Py_DECREF (klass);
  if ( 0 == idObj ) {
    PyErr_Clear();
    return StatusCode::FAILURE;
  }

  PyObject* pyId = PyObject_CallMethod( idObj,
					const_cast<char*>("id"),
					const_cast<char*>("") );
  if ( 0 == pyId ) {
    PyErr_Clear();
    Py_DECREF (idObj);
    return StatusCode::FAILURE;
  }
  id = PyLong_AsUnsignedLong(pyId);
  Py_DECREF(pyId);

  PyObject* pyMajor = PyObject_CallMethod( idObj,
					   const_cast<char*>("majorVersion"),
					   const_cast<char*>("") );
  if ( 0 == pyMajor ) {
    PyErr_Clear();
    Py_DECREF (idObj);
    return StatusCode::FAILURE;
  }
  major = PyLong_AsUnsignedLong(pyMajor);
  Py_DECREF(pyMajor);

  PyObject* pyMinor = PyObject_CallMethod( idObj,
					   const_cast<char*>("minorVersion"),
					   const_cast<char*>("") );
  if ( 0 == pyMinor ) {
    PyErr_Clear();
    Py_DECREF (idObj);
    return StatusCode::FAILURE;
  }
  minor = PyLong_AsUnsignedLong(pyMinor);
  Py_DECREF(pyMajor);

  Py_DECREF (idObj);
  return StatusCode::SUCCESS;
}
} //> anonymous namespace

std::string 
PyAthena::repr( PyObject* o )
{
  // PyObject_Repr returns a new ref.
  RootUtils::PyGILStateEnsure ensure;
  PyObject* py_repr = PyObject_Repr( o );

  auto [cpp_repr, flag] = RootUtils::PyGetString (py_repr);
  Py_DECREF( py_repr );
  if (!flag) {
    PyAthena::throw_py_exception();
  }
  return cpp_repr;
}

std::string 
PyAthena::str( PyObject* o )
{
  // PyObject_Str returns a new ref.
  RootUtils::PyGILStateEnsure ensure;
  PyObject* py_str = PyObject_Str( o );
  auto [cpp_str, flag] = RootUtils::PyGetString (py_str);
  Py_DECREF( py_str );
  if ( !flag ) {
    PyAthena::throw_py_exception();
  }
  return cpp_str;
}

void PyAthena::throw_py_exception (bool display)
{
  if (display) {
    RootUtils::PyGILStateEnsure ensure;
    // fetch error
    PyObject* pytype = 0, *pyvalue = 0, *pytrace = 0;
    PyErr_Fetch (&pytype, &pyvalue, &pytrace);
    Py_XINCREF  (pytype);
    Py_XINCREF  (pyvalue);
    Py_XINCREF  (pytrace);
    // restore...
    PyErr_Restore (pytype, pyvalue, pytrace);
    // and print
    PyErr_Print();
    {
      // With py3, need to explicitly flush the python stderr
      // for the error to be visible.
      PyObject* f = PySys_GetObject (const_cast<char*>("stderr"));
#if PY_VERSION_HEX < 0x03000000
      PyObject* fstr = PyString_FromString ("flush");
#else
      PyObject* fstr = PyUnicode_FromString ("flush");
#endif
      PyObject* x = PyObject_CallMethodObjArgs (f, fstr, NULL);
      Py_XDECREF (x);
      Py_XDECREF (fstr);
      Py_XDECREF (f);
    }
  }
  throw CPyCppyy::PyException();
}

StatusCode 
PyAthena::callPyMethod ATLAS_NOT_THREAD_SAFE ( PyObject* self,
                                               const char* methodName,
                                               PyObject* arg /*= nullptr*/)
{
  // that's a bit ugly...
  char* method = const_cast<char*>(methodName);

  // check arguments 
  if ( 0 == self || 0 == method ) { return StatusCode::FAILURE; }
  
  // call Python 
  RootUtils::PyGILStateEnsure ensure;
  PyObject* r;
  if (arg)
    r = PyObject_CallMethod( self, method, const_cast<char*>("O"), arg );
  else
    r = PyObject_CallMethod( self, method, const_cast<char*>("") );
  
  if ( 0 == r ) { 
    throw_py_exception();
  }
  
#if PY_VERSION_HEX < 0x03000000
  if ( PyInt_Check( r ) || PyLong_Check(r) ) {
    StatusCode sc(PyInt_AS_LONG( r ));
#else
  if ( PyLong_Check( r ) ) {
    StatusCode sc(PyLong_AS_LONG( r ));
#endif
    Py_DECREF( r );
    return sc;
  }
  
  // look for the method getCode with the signature: 
  //  ' int getCode() '
  PyObject* c = 0;
  if ( PyObject_HasAttrString (r, (char*)"getCode") ) {
    c = PyObject_CallMethod( r, 
			     const_cast<char*>("getCode"), 
			     const_cast<char*>("") );
    Py_DECREF (r);
  } 

  else {
    std::ostringstream msg;
    msg << "unexpected returned type from (python) function '"
	<< method << "()' [got "
	<< PyAthena::repr ((PyObject*)r->ob_type)
	<< "]";
    PyErr_SetString (PyExc_TypeError, msg.str().c_str());
    Py_XDECREF (c);
    throw_py_exception();
  }
  
  if ( !c ) {
    // error on python side...
    throw_py_exception();
  }

  if ( PyLong_Check (c) ) { 
    StatusCode sc(PyLong_AsLong (c));
    Py_DECREF (c);
    return sc;
  } 

  else {
    std::ostringstream msg;
    msg << "unexpected returned type from (python) function '"
	<< method << "().getCode()' [got "
	<< PyAthena::repr ((PyObject*)c->ob_type)
	<< "]";
    PyErr_SetString (PyExc_TypeError, msg.str().c_str());
    Py_DECREF (c);
    throw_py_exception();
  }
  return StatusCode::FAILURE;
}

StatusCode PyAthena::queryInterface ATLAS_NOT_THREAD_SAFE
  ( PyObject* self,
    const InterfaceID& riid,
    void** ppvInterface )
{
  StatusCode sc = StatusCode::FAILURE;

  {
    std::cout 
      << "==============[ " << PyAthena::repr(self) << " ]=============="
      << std::endl;
  }

  RootUtils::PyGILStateEnsure ensure;
  PyObject* type = PyObject_GetAttrString( self, 
					   const_cast<char*>("__class__") );
  if ( !type ) {
    Py_XDECREF(type);
    return sc;
  }

  PyObject* bases;
  bases = PyObject_CallMethod( type, 
			       const_cast<char*>("mro"), 
 			       const_cast<char*>("") );
  if ( !bases || !PySequence_Check( bases ) ) {
    Py_XDECREF(type);
    Py_XDECREF(bases);
    return sc;
  }
  
  const int nBases = PySequence_Size( bases );
  if ( -1 == nBases ) {
    Py_XDECREF(type);
    Py_XDECREF(bases);
    return sc;
  }

  for ( int i = 0; i < nBases; ++i ) {
    PyObject* base = PySequence_GetItem( bases, i );
    if ( !base ) {
      Py_XDECREF( base );
      continue;
    }
    unsigned long id = 0;
    unsigned long major = 0;
    unsigned long minor = 0;
    if ( !fetchInterfaceId( base, id, major, minor ).isSuccess() ) {
      Py_DECREF( base );
      continue;
    }

    InterfaceID pyID( id, major, minor );
    if ( !pyID.versionMatch( riid ) ) {
      Py_DECREF( base );
      continue;
    }

    PyObject* pyname = 0;
    pyname = PyObject_GetAttrString( base,
				     const_cast<char*>("__name__") );
    auto [cppBaseName, flag] = RootUtils::PyGetString (pyname);
    Py_DECREF(pyname);
    if ( !flag ) {
      Py_DECREF ( base   );
      continue;
    }

    const std::string cppName = ((MyObjProxy*)self)->m_class->GetName();

    std::cout << "::: would like to do: *ppvInterface = static_cast<"
	      << cppBaseName << "*>( " 
	      << cppName << "|m_self );"
	      << std::endl;
    
    const RootType fromType( cppName );
    const RootType toType( cppBaseName );
    void* objProxy = TPython::CPPInstance_AsVoidPtr(self);
    *ppvInterface = objProxy;
    if (fromType.Class() && toType.Class())
      *ppvInterface = fromType.Class()->DynamicCast (toType.Class(), objProxy);
    std::cout << "::: [" << cppName << "]: " 
	      << ( (bool)fromType ? " OK" : "ERR" ) 
	      << " " << objProxy
	      << "\n"
	      << "::: [" << cppBaseName << "]: "
	      << ( (bool)toType ? " OK" : "ERR" )
	      << " " << *ppvInterface
	      << "\n";
    std::cout << "::: *ppvInterface: " << *ppvInterface << std::endl;
    if ( *ppvInterface ) {
      PyObject* c = PyObject_CallMethod( self, 
					 const_cast<char*>("addRef"), 
					 const_cast<char*>("") );
      if ( c && PyLong_Check(c) ) {
	sc = StatusCode::SUCCESS;
      }
      Py_DECREF(c);
    }
    Py_DECREF( base );
    if ( sc.isSuccess() ) {
      break;
    }
  }

  Py_DECREF(type);
  Py_DECREF(bases);
  return sc;
}

  /// helper function for PyAthena::Aud
void PyAthena::pyAudit ATLAS_NOT_THREAD_SAFE
 ( PyObject* self,
   const char* method,
   const char* evt, 
   const char* component )
{
  RootUtils::PyGILStateEnsure ensure;
  PyObject* call = PyObject_CallMethod(self,
				       (char*)method,
				       (char*)"ss", evt, component);
  if ( !call ) {
    Py_XDECREF(call);
    throw_py_exception();
  }
  Py_DECREF(call);
  return;
}

  /// helper function for PyAthena::Aud
void PyAthena::pyAudit ATLAS_NOT_THREAD_SAFE
  ( PyObject* self, 
    const char* method, 
    const char* evt,
    const char* component,
    const StatusCode& sc )
{
  RootUtils::PyGILStateEnsure ensure;
  PyObject* pySc = TPython::CPPInstance_FromVoidPtr((void*)&sc,
						    "StatusCode");
  if ( !pySc ) {
    throw CPyCppyy::PyException();
  }

  PyObject* call = PyObject_CallMethod(self,
				       (char*)method,
				       (char*)"ssO", evt, component, pySc);
  Py_DECREF(pySc);

  if ( !call ) {
    Py_XDECREF(call);
    throw_py_exception();
  }
  Py_DECREF(call);
  return;
}

