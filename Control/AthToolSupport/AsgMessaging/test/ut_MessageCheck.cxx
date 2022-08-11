/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgMessaging/MessageCheck.h>

#include <iostream>
#include <assert.h>
#include <functional>

//
// unit test
//

template<typename T,typename T2>
void checkTypeSingle (const T2& scSuccess, const T2& scTest, bool expectedSuccess,
		      std::function<bool(const T&)> successTest)
{
  using namespace asg::msgUserCode;

  bool success = false;
  T mySC = [&] () -> T {
    ANA_CHECK_SET_TYPE (T);
    ANA_CHECK (T (scTest));
    success = true;
    return scSuccess;
  } ();
  assert (successTest (mySC) == expectedSuccess);
  assert (success == expectedSuccess);
}

template<typename T,typename T2>
void checkType (const T2& scSuccess, const T2& scFailure1, const T2& scFailure2,
		std::function<bool(const T&)> successTest)
{
  checkTypeSingle<T> (scSuccess, scSuccess, true, successTest);
  checkTypeSingle<T> (scSuccess, scFailure1, false, successTest);
  checkTypeSingle<T> (scSuccess, scFailure2, false, successTest);
}

int main ()
{
  StatusCode::enableFailure ();

  // checkType<StatusCode> (StatusCode::SUCCESS, StatusCode::FAILURE, StatusCode::FAILURE, [] (const StatusCode& sc) -> bool {return sc.isSuccess();});
  // checkType<StatusCode> (StatusCode::SUCCESS, StatusCode::FAILURE, StatusCode::RECOVERABLE, [] (const StatusCode& sc) -> bool {return sc.isSuccess();});
  checkType<bool> (true, false, false, [] (const bool& sc) -> bool {return sc;});

  return 0;
}
