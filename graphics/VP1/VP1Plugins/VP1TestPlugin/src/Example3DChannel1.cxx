/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
//                                                         //
//  Implementation of class Example3DChannel1              //
//                                                         //
//  Author: Thomas Kittelmann <Thomas.Kittelmann@cern.ch>  //
//                                                         //
//  Initial version: June 2007                             //
//                                                         //
/////////////////////////////////////////////////////////////

#include "VP1TestPlugin/Example3DChannel1.h"

#include "VP1TestSystems/Example3DSystem1.h"

Example3DChannel1::Example3DChannel1()
  : IVP13DStandardChannelWidget(VP1CHANNELNAMEINPLUGIN(Example3DChannel1,"Example 1"),
				"This channel is an example of"
				" a 3D channel which displays the"
				" 3D scene delivered by the Example3DSystem1.",
				"Thomas.Kittelmann@cern.ch")
{
}

void Example3DChannel1::init()
{
  addSystem(new Example3DSystem1);
}
