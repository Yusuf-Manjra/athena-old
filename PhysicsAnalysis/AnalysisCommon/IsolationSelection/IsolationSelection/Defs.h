/*
 Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_DEFS_H
#define ISOLATIONSELECTION_DEFS_H

// EDM include(s):
#include <AthContainers/AuxElement.h>
#include <xAODPrimitives/IsolationType.h>

#include <memory>
#include <vector>
namespace CP{
    
    typedef SG::AuxElement::ConstAccessor<char> CharAccessor;
    typedef SG::AuxElement::ConstAccessor<float> FloatAccessor;

    typedef SG::AuxElement::Decorator<char> CharDecorator;
    typedef SG::AuxElement::Decorator<float> FloatDecorator;  
    
    typedef std::unique_ptr<CharAccessor> SelectionAccessor;
    typedef std::unique_ptr<CharDecorator> SelectionDecorator;
    
    typedef SG::AuxElement::Decorator<bool> BoolDecorator;
    typedef SG::AuxElement::ConstAccessor<bool> BoolAccessor;
    
    typedef SG::AuxElement::Decorator<int> IntDecorator;
    typedef SG::AuxElement::ConstAccessor<int> IntAccessor;
    
    typedef SG::AuxElement::Decorator<short> ShortDecorator;
    typedef SG::AuxElement::ConstAccessor<short> ShortAccessor;
    
    typedef xAOD::Iso::IsolationType IsoType;
    typedef std::vector<IsoType> IsoVector;


}

#endif

