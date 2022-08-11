/* emacs: this is -*- c++ -*- */
/**
 **     @file    simpletimer.h
 **
 **       these functions have a precision of about 0.001 ms
 **
 **     @author  mark sutton
 **     @date    Thu 22 Jan 2009 15:51:52 GMT 
 **
 **     Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef ANALYSIS_SIMPLETIMER_H
#define ANALYSIS_SIMPLETIMER_H

#include <time.h>
#include <sys/time.h>

//#ifdef __cplusplus
//extern "C" {
//#endif


inline struct timeval simpletimer_start(void) {
  struct timeval start_time;
  gettimeofday(&start_time, NULL);            
  return start_time;
}

inline double simpletimer_stop(const struct timeval& start_time)
{
  struct timeval stop_time;
  struct timeval diff_time;
  gettimeofday(&stop_time, NULL);            
  diff_time.tv_sec  = stop_time.tv_sec  - start_time.tv_sec;
  diff_time.tv_usec = stop_time.tv_usec - start_time.tv_usec;
  return (diff_time.tv_sec*1000.0) + (diff_time.tv_usec/1000.0);
}


//#ifdef __cplusplus
//}
//#endif

#endif /* ANALYSIS_SIMPLETIMER_H */










