// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODCORE_TOOLS_DICTHELPERS_H
#define XAODCORE_TOOLS_DICTHELPERS_H

// EDM include(s).
#include "AthLinks/DataLink.h"
#include "AthLinks/ElementLink.h"
#include "AthContainers/tools/AuxTypeVectorFactory.h"

// System include(s).
#include <vector>

#ifdef XAOD_STANDALONE
# define XAOD_BASEINFO_TYPE(TYPE) int
#else
# define XAOD_BASEINFO_TYPE(TYPE) SG::BaseInfo<TYPE>
#endif

/// Macro instantiating all the container "interface" and smart pointer types
/// that we need a dictionary for.
#define XAOD_INSTANTIATE_CONTAINER_TYPES( TYPE )                               \
   TYPE dummy_##TYPE##_1;                                                      \
   DataLink< TYPE > dummy_##TYPE##_2;                                          \
   ElementLink< TYPE > dummy_##TYPE##_3;                                       \
   std::vector< DataLink< TYPE > > dummy_##TYPE##_4;                           \
   std::vector< ElementLink< TYPE > > dummy_##TYPE##_5;                        \
   std::vector< std::vector< ElementLink< TYPE > > > dummy_##TYPE##_6;         \
   SG::AuxTypeVectorFactory< DataLink< TYPE > > dummy_##TYPE##_7;              \
   SG::AuxTypeVectorFactory< ElementLink< TYPE > > dummy_##TYPE##_8;           \
   SG::AuxTypeVectorFactory< std::vector< ElementLink< TYPE > > >              \
     dummy_##TYPE##_9;                                                         \
   XAOD_BASEINFO_TYPE(TYPE) dummy_##TYPE##_10;                                 \
   DataVectorBase< TYPE::base_value_type > dummy_##TYPE##_11

/// Macro instantiating all the container "interface" and smart pointer types
/// that we need a dictionary for.
#define XAOD_INSTANTIATE_NS_CONTAINER_TYPES( NS, TYPE )                        \
   NS::TYPE dummy_##NS##_##TYPE##_1;                                           \
   DataLink< NS::TYPE > dummy_##NS##_##TYPE##_2;                               \
   ElementLink< NS::TYPE > dummy_##NS##_##TYPE##_3;                            \
   std::vector< DataLink< NS::TYPE > > dummy_##NS##_##TYPE##_4;                \
   std::vector< ElementLink< NS::TYPE > > dummy_##NS##_##TYPE##_5;             \
   std::vector< std::vector< ElementLink< NS::TYPE > > >                       \
      dummy_##NS##_##TYPE##_6;                                                 \
   SG::AuxTypeVectorFactory< DataLink< NS::TYPE > > dummy_##NS##_##TYPE##_7;   \
   SG::AuxTypeVectorFactory< ElementLink< NS::TYPE > > dummy_##NS##_##TYPE##_8;\
   SG::AuxTypeVectorFactory< std::vector< ElementLink< NS::TYPE > > >          \
      dummy_##NS##_##TYPE##_9;                                                 \
   XAOD_BASEINFO_TYPE(NS::TYPE) dummy_##NS##_##TYPE##_10;                      \
   DataVectorBase< NS::TYPE::base_value_type > dummy_##NS##_##TYPE##_11

/// Macro instantiating all the single object smart pointer types that we need
/// a dictionary for.
#define XAOD_INSTANTIATE_OBJECT_TYPES( TYPE )                                  \
   TYPE dummy_##TYPE##_1;                                                      \
   DataLink< TYPE > dummy_##TYPE##_2;                                          \
   std::vector< DataLink< TYPE > > dummy_##TYPE##_3;                           \
   SG::AuxTypeVectorFactory< DataLink< TYPE > > dummy_##TYPE##_4

/// Macro instantiating all the single object smart pointer types that we need
/// a dictionary for.
#define XAOD_INSTANTIATE_NS_OBJECT_TYPES( NS, TYPE )                           \
   NS::TYPE dummy_##NS##_##TYPE##_1;                                           \
   DataLink< NS::TYPE > dummy_##NS##_##TYPE##_2;                               \
   std::vector< DataLink< NS::TYPE > > dummy_##NS##_##TYPE##_3;                \
   SG::AuxTypeVectorFactory< DataLink< NS::TYPE > > dummy_##NS##_##TYPE##_4

#endif // XAODCORE_TOOLS_DICTHELPERS_H
