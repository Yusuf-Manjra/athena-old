/*
Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#include <mutex>
#include <map>

#include "tbb/global_control.h"
#include "TestTools/expect.h"
#include "TestTools/ParallelCallTest.h"
#include <unistd.h>


/**
 * This is an example of how to use the ParallelCallTest utility.
 **/



/**
 * @brief Specimen of service caching expensive calculations.
 * It uses internally std::map which is not thread-safely expandable.
 **/
class IService { };
class ExpensiveCompCacheSvc : public IService {
public:
  int compute( int x ) {
    // should the lock be here, or can be later only when we update the map
    std::lock_guard<std::mutex> lock( m_mutex );

    std::map<int,int>::const_iterator i =  m_cache.find( x );
    if ( i != m_cache.end() ) 
      return i->second;
    int p = std::pow( x, 3 ) + std::sqrt( x );
    // maybe it is enough to have the lock here, (covering much shorter code section and allowing expensive calculations to be run in parallel)
    // In this particular case reading documentation may be useful.
    // std::lock_guard<std::mutex> lock( m_mutex );
    m_cache[x] = p;
    return p;    
  }

private:
  std::mutex m_mutex;
  std::map<int, int> m_cache;
};


class SvcTestScenario : public ParallelCallTest {
public:

  SvcTestScenario ( ExpensiveCompCacheSvc* svc, int request )
    : m_svc(svc),
      m_request(request),
      m_firstResponse(0) {}
  

  virtual void firstCall() override {
    m_firstResponse = m_svc->compute( m_request );    
  }
  
  virtual bool callAndCompare() const override {
    int nextResponse =  m_svc->compute( m_request );
    VALUE( nextResponse ) EXPECTED ( m_firstResponse ); // if we get wrong response we will throw an exception 
    //    VALUE( nextResponse ) EXPECTED ( m_firstResponse ); // that should fail
    return nextResponse == m_firstResponse; // above assertions can be replaced by printouts and then we should be returning "true" if the results are correct 
  }

private:
  ExpensiveCompCacheSvc* m_svc;
  int m_request;
  int m_firstResponse;

};


int main() {
  // use 10 threads, 
  // for testing in athena this is not needed as the scheduler 
  // sets it according to the athena runtime options 
  // example: athena --concurrent-events=2 --threads=4 jo.py

  tbb::global_control (tbb::global_control::max_allowed_parallelism, 10);
  
  ExpensiveCompCacheSvc svc;
  
  SvcTestScenario t1( &svc, 3000 );
  SvcTestScenario t2( &svc, 3000 ); // identical request started in parallel
  SvcTestScenario t3( &svc, 4001 ); // different request
  bool status = ParallelCallTest::launchTests( 100, { &t1, &t2, &t3 } );

  // Without this, we get occasional segfaults in global dtors.
  sleep(1);

  if ( status == false ) {
    std::cerr << "ERROR in the test" << std::endl;
    return -1;
  }
  std::cout << "Test passed OK" << std::endl;
  return 0;
}
