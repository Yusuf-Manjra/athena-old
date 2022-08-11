/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "AGDDHandlers/UbeamHandler.h"
#include "AGDDModel/AGDDUbeam.h"
#include "AGDDControl/AGDDController.h"
#include <iostream>


UbeamHandler::UbeamHandler(const std::string& s,
                           AGDDController& c)
  : XMLHandler(s, c)
{
//	std::cout<<"Creating handler for Ubeam"<<std::endl;
}

void UbeamHandler::ElementHandle(AGDDController& c,
                                 xercesc::DOMNode *t)
{
	bool res;
	std::string name=getAttributeAsString(c, t, "name",res);
	std::string material=getAttributeAsString(c, t, "material",res);

	double l=getAttributeAsDouble(c, t, "length",res);
	double w=getAttributeAsDouble(c, t, "width",res);
	double sw=getAttributeAsDouble(c, t, "small_width",res);
	double h=getAttributeAsDouble(c, t, "heigth",res);
	double sh=getAttributeAsDouble(c, t, "small_heigth",res);
	
	AGDDUbeam *b=new AGDDUbeam(name, c.GetVolumeStore(), c.GetSectionStore());
	b->SetMaterial(material);
	b->SetLength(l);
	b->SetWidth(w);
	b->SetSmallWidth(sw);
	b->SetHeigth(h);
	b->SetSmallHeigth(sh);
	
	std::string col=getAttributeAsString(c, t, "color",res);
	if (!col.empty())
		b->SetColor(col);
}
