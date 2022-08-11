// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODCORE_TOOLS_PERFSTATS_H
#define XAODCORE_TOOLS_PERFSTATS_H

// ROOT include(s):
#include <TVirtualPerfStats.h>

// Athena include(s).
#include "CxxUtils/checker_macros.h"

// System include(s).
#include <mutex>

// Forward declaration(s):
class TTree;

namespace xAOD {

   /// Class for collecting information about the xAOD file access pattern
   ///
   /// This class is used for collecting the basic information about the file
   /// access during an xAOD analysis. It was first developed for the
   /// D3PDReader code, and was later adopted to be used with xAOD files.
   ///
   /// Note: One can't use @c override on the virtual functions of this class,
   /// as it generates a lot of warnings because of how @c ClassDef(...) is
   /// implemented. :-(
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class PerfStats : public ::TVirtualPerfStats {

   public:
      /// Destructor, sometimes called by PROOF
      ~PerfStats();

      /// Function accessing the singleton instance
      static PerfStats& instance();

      /// Start the statistics collection
      void start( bool clear = true );
      /// Stop the statistics collection
      void stop();

      /// @name Functions inherited from TVirtualPerfStats:
      /// @{

      /// Generic function called when a specified event happens
      virtual void SimpleEvent( EEventType type );
      /// PROOF specific function, not implemented here
      virtual void PacketEvent( const char *slave, const char *slavename,
                                const char *filename,
                                ::Long64_t eventsprocessed,
                                ::Double_t latency, ::Double_t proctime,
                                ::Double_t cputime, ::Long64_t bytesRead );
      /// PROOF specific function, not implemented here
      virtual void FileEvent( const char *slave, const char *slavename,
                              const char *nodename, const char *filename,
                              ::Bool_t isStart );
      /// Function called by PROOF when a file is opened
      virtual void FileOpenEvent( ::TFile *file, const char *filename,
                                  ::Double_t start );
      /// Function called in general when a file reading operation happens
      virtual void FileReadEvent( ::TFile *file, ::Int_t len,
                                  ::Double_t start );
      /// Function called in general when a file unzipping operation happens
      virtual void UnzipEvent( ::TObject *tree, ::Long64_t pos,
                               ::Double_t start, ::Int_t complen,
                               ::Int_t objlen );
      /// PROOF specific function, not implemented here
      virtual void RateEvent( ::Double_t proctime, ::Double_t deltatime,
                              ::Long64_t eventsprocessed,
                              ::Long64_t bytesRead );
      /// Function used by PROOF to set the read bytes correctly on the master
      virtual void SetBytesRead( ::Long64_t num );
      /// Function used by PROOF to set the read bytes correctly on the master
      virtual ::Long64_t GetBytesRead() const;
      /// Function used by PROOF to set the number of processed events correctly
      virtual void SetNumEvents( ::Long64_t num );
      /// Function used by PROOF to set the number of processed events correctly
      virtual ::Long64_t GetNumEvents() const;

      /// Print the TTree basket read caching statistics
      virtual void PrintBasketInfo(Option_t *option = "") const;

      /// Increase by 1 the counter of how many times a basket was cached
      virtual void SetLoaded(TBranch *b, size_t basketNumber);
      virtual void SetLoaded(size_t bi, size_t basketNumber);

      /// Increase by 1 the counter of how many times a basket was cached on access
      virtual void SetLoadedMiss(TBranch *b, size_t basketNumber);
      virtual void SetLoadedMiss(size_t bi, size_t basketNumber);

      /// Increase count (by 1) of not cached basket reads
      virtual void SetMissed(TBranch *b, size_t basketNumber);
      virtual void SetMissed(size_t bi, size_t basketNumber);

      /// Mark a basket as accessed
      virtual void SetUsed(TBranch *b, size_t basketNumber);
      virtual void SetUsed(size_t bi, size_t basketNumber);

      /// Update the fBranchIndexCache collection to match the current TTree given
      /// the ordered list of branch names.
      virtual void UpdateBranchIndices(TObjArray *branches);

      /// @}

   protected:
      /// The constructor is protected, as it's a singleton
      PerfStats();

   private:
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 23, 2 )
      /// Function letting us know that a new file was opened
      virtual void SetFile( TFile* file );
#endif // ROOT version

      /// The single instance of the object
      static PerfStats* s_instance ATLAS_THREAD_SAFE;

      /// Lock controlling access to the singleton.
      static std::mutex s_mutex;
      typedef std::lock_guard<std::mutex> lock_t;

      /// Another performance monitoring object
      ::TVirtualPerfStats* m_otherPerfStats;

      /// Flag showing whether the statistic collection is ongoing or not
      bool m_running;
      /// Time when the statistics collection was started
      ::Double_t m_startTime;

      /// The tree we're currently investigating
      ::TTree* m_tree;
      /// The currently open xAOD file
      ::TFile* m_file;

      /// Flag showing whether some information message has already been printed
      bool m_treeWarningPrinted;

      ClassDef( xAOD::PerfStats, 0 )

   }; // class PerfStats

} // namespace xAOD

#endif // XAODCORE_TOOLS_PERFSTATS_H
