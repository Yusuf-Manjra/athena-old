/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLROOTACCESS_TEVENT_H
#define POOLROOTACCESS_TEVENT_H

#include "StoreGate/StoreGateSvc.h"

#include "GaudiKernel/Bootstrap.h"

#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "AthenaKernel/IEvtSelectorSeek.h"
#include "AthenaKernel/IEventSeek.h"

#include "GaudiKernel/IAppMgrUI.h"

//include the CLIDs of simple types so that we don't need to do this ourselves from ROOT prompt
//when reading in kTreeAccess
#include "SGTools/BuiltinsClids.h"
#include "SGTools/StlVectorClids.h"
#include "SGTools/StlMapClids.h"

class TFile;
class TChain;
class IEventProcessor;

//Bootstraps the minimal gaudi environment + a few extra defaults (see basic.opts)
namespace Gaudi {
  IAppMgrUI* Init();
}

namespace POOL {

   ///Bootstraps (creates and configures) the Gaudi Application with the provided options file
   IAppMgrUI* Init( const char* options = "POOLRootAccess/basic.opts" );


   class TEvent {
      public:
         enum EReadMode {
	   kTreeAccess = -2, //read the TTree directly
           kPOOLAccess = -1, //cannot be used in the same session as the other access modes
           kBranchAccess = 0,
           kClassAccess = 1,
           kAthenaAccess = 2,
           kUndefinedAccess = 3
         };
      
         static IAppMgrUI* Init( const char* options = "POOLRootAccess/basic.opts" ) { return POOL::Init(options); }
  

         TEvent( const std::string& name = "StoreGateSvc" );
         TEvent( EReadMode mode, const std::string& name = "StoreGateSvc" );
         
         ~TEvent();

         ServiceHandle<IEventSeek>& evtLoop() { return m_evtLoop; }
         ServiceHandle<StoreGateSvc>& evtStore() { return m_evtStore; }
         ServiceHandle<StoreGateSvc>& inputMetaStore() { return m_inputMetaStore; }

         template<typename T> void setEvtSelProperty( const char* name, const T& val ) {
            if constexpr (std::is_convertible_v<T, std::string>) {
               // Gaudi::Utils::toString adds extra quotes, don't do this for strings:
               m_joSvc->set(m_evtSelect.name() + "." + name, std::string(val));
            }
            else {
               m_joSvc->set(m_evtSelect.name() + "." + name, Gaudi::Utils::toString(val));
            }
         }

         //forward retrieve calls to the evtStore
         template<typename T> StatusCode retrieve( const T*& obj ) {
            return evtStore()->retrieve( obj );
         }

         template<typename T> StatusCode retrieve( T*& obj ) {
            return evtStore()->retrieve( obj );
         }

         template<typename T> StatusCode retrieve( const T*& obj , const std::string& key ){
            return evtStore()->retrieve( obj, key );
         }
         template<typename T> StatusCode retrieve( T*& obj , const std::string& key ){
            return evtStore()->retrieve( obj, key );
         }

         //forward calls to metadata store
         template<typename T> StatusCode retrieveMetaInput( const T*& obj, const std::string& key ) {
            return inputMetaStore()->retrieve( obj, key );
         }

         int getEntry( long entry ); //return -1 if failure. otherwise 0
         long getEntries();

         StatusCode readFrom( TFile* file );
         StatusCode readFrom( const char* file ); //also takes a comma-separated list and can take wild cards
	 StatusCode readFrom( TChain* files ); //multi-file input

         ///ensure we are the active store, for the purpose of element links etc
         void setActive();


	 ///retrieve and record method for python
	 PyObject* retrieve( PyObject* tp, PyObject* pykey );
	 PyObject* get_item( PyObject* pykey ); //typess retrieve, which is slower than retrieve above
	 PyObject* contains( PyObject* tp, PyObject* pykey );
	 PyObject* retrieveMetaInput(PyObject* tp, PyObject* pykey);
	 PyObject* record  (PyObject* obj, PyObject* pykey,bool allowMods = true,bool resetOnly = true,bool noHist = false);

      private:
         long m_curEntry = -1; //if this becomes >=0 then the TEvent is considered 'initialized'
         long m_size = -1; //cache of the event size, filled on first call to getEntries

         IEventProcessor* m_evtProcessor;
         ServiceHandle<Gaudi::Interfaces::IOptionsSvc> m_joSvc;

         ServiceHandle<IEventSeek> m_evtLoop; //the AthenaEventLoopMgr
         ServiceHandle<IEvtSelectorSeek> m_evtSelect; //the EventSelectorAthenaPool
         ServiceHandle<StoreGateSvc> m_evtStore; //the storegate

         ServiceHandle<StoreGateSvc> m_inputMetaStore; //input metadata storegate

   };

}


#endif
