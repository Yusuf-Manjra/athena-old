/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//          
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (krumnack@iastate.edu) for bug
// reports, feature suggestions, praise and complaints.


//
// includes
//

#include <EventLoop/Global.h>

#include <EventLoop/LocalDriver.h>
#include <EventLoop/Job.h>
#include <EventLoopTest/UnitTest.h>

//
// main program
//

using namespace EL;

int main ()
{
  LocalDriver driver;
  driver.options()->setBool (Job::optLocalNoUnsetup, true);
  UnitTest ut ("local");
  ut.cleanup = false;
  return ut.run (driver);
}
