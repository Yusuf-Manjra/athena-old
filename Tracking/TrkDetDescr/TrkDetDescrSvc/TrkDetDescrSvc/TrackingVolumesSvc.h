/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackingVolumesSvc.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_TRACKINGVOLUMESSERVICE_H
#define TRKDETDESCRINTERFACES_TRACKINGVOLUMESSERVICE_H

#include "TrkDetDescrInterfaces/ITrackingVolumesSvc.h"
#include "TrkDetDescrUtils/LayerIndex.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include "AthenaBaseComps/AthService.h"
#include <string>
#include <vector>

class StoreGateSvc;
class ISvcLocator;

template <class TYPE> class SvcFactory;


namespace Trk {
  
  class Volume;


   /** @class TrackingVolumeSvc 
   
      This service provides the detector boundaries in term
      of tracking volume descriptions.
   
      @author Andreas.Salzburger@cern.ch, Edward.Moyse@cern.ch
   */
  class TrackingVolumesSvc : public AthService, virtual public ITrackingVolumesSvc {
  
    public:
  
      //!< Retrieve interface ID
      static const InterfaceID& interfaceID() { return IID_ITrackingVolumesSvc; }
  
      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
  
      /** Query the interfaces.
      /   Input: riid, Requested interface ID
      /          ppvInterface, Pointer to requested interface
      /   Return: StatusCode indicating SUCCESS or FAILURE.
      / N.B. Don't forget to release the interface after use!!! **/
      virtual StatusCode queryInterface( const InterfaceID& riid, void** ppvInterface ) override;
  
      /** @copydoc ITrackingVolumesSvc::volume() */
      virtual const Trk::Volume& volume(const TrackingVolumeIdentifier& volumeId) const override;
  
      /** @copydoc ITrackingVolumesSvc::volumeName() */
      virtual const std::string& volumeName(const TrackingVolumeIdentifier& volumeId) const override;
  
      friend class SvcFactory<TrackingVolumesSvc>;
  
      /** Standard Constructor */
      TrackingVolumesSvc(const std::string& name, ISvcLocator* svc);
  
      /** Standard Destructor */
      virtual ~TrackingVolumesSvc();
  
    
    private:
        
      ServiceHandle<StoreGateSvc>                 m_pDetStore;
  
      //!< the cached volumes
      std::vector<const Trk::Volume*>             m_volumes;
      /** the names of the TrackingVolumes */
      std::vector<std::string >                   m_volumeNames;
  
  };
}

inline const Trk::Volume& Trk::TrackingVolumesSvc::volume(const Trk::ITrackingVolumesSvc::TrackingVolumeIdentifier& volumeId) const
  { return *(m_volumes[volumeId]); }

inline const std::string& Trk::TrackingVolumesSvc::volumeName(const Trk::ITrackingVolumesSvc::TrackingVolumeIdentifier& volumeId) const
  { return m_volumeNames[volumeId]; }

#endif 

