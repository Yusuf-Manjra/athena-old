/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef gvxysxHandler_H
#define gvxysxHandler_H

#include "AGDDControl/XMLHandler.h"
#include <string>

class gvxysxHandler:public XMLHandler {
public:
	gvxysxHandler(const std::string&,
                      AGDDController& c);
	virtual void ElementHandle(AGDDController& c,
                                   xercesc::DOMNode *t) override;
};

#endif
