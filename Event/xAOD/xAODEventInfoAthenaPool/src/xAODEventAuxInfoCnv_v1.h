// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODEVENTINFOATHENAPOOL_XAODEVENAUXINFOCNV_V1_H
#define XAODEVENTINFOATHENAPOOL_XAODEVENAUXINFOCNV_V1_H

// Gaudi/Athena include(s).
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// EDM include(s).
#include "xAODEventInfo/versions/EventAuxInfo_v1.h"
#include "xAODEventInfo/EventAuxInfo.h"

/// Converter for reading @c xAOD::EventAuxInfo_v1
///
/// This converter takes care of reading @c xAOD::EventAuxInfo_v1 into
/// the latest version of @c xAOD::EventAuxInfo.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class xAODEventAuxInfoCnv_v1 :
   public T_AthenaPoolTPCnvConstBase< xAOD::EventAuxInfo,
                                      xAOD::EventAuxInfo_v1 > {

public:
   /// Inherit the base class's converter functions.
   using base_class::transToPers;
   using base_class::persToTrans;

   /// Default constructor
   xAODEventAuxInfoCnv_v1() = default;

   /// Function converting from the old type to the current one
   virtual void persToTrans( const xAOD::EventAuxInfo_v1* oldObj,
                             xAOD::EventAuxInfo* newObj,
                             MsgStream& log ) const override;
   /// Dummy function inherited from the base class
   virtual void transToPers( const xAOD::EventAuxInfo*,
                             xAOD::EventAuxInfo_v1*,
                             MsgStream& log ) const override;

}; // class xAODEventAuxInfoCnv_v1

#endif // XAODEVENTINFOATHENAPOOL_XAODEVENAUXINFOCNV_V1_H
