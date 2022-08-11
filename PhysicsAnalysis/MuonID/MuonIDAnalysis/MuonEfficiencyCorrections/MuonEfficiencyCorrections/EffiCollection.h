/*
 Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */
#ifndef EFFICOLLECTION_H_
#define EFFICOLLECTION_H_

// @class EffiCollection
/// @brief Utility class to collect the separate histos
/// for Calo, Central and High Eta muon SF


#include <MuonEfficiencyCorrections/MuonEfficiencyType.h>
#include <MuonEfficiencyCorrections/EfficiencyScaleFactor.h>
#include <xAODMuon/Muon.h>
#include <map>
#include <string>
#include <iostream>

namespace CP {
    class MuonEfficiencyScaleFactors;
    class EfficiencyScaleFactor;
    class SystematicSet;
    class SystematicVariation;
    class CollectionContainer;
    /// The EffiCollection class handles the 5 different scale-factor maps binned in time. Each muon is piped to the correct map based 
    /// whether it's a calo-tag muon, belongs to the high-eta region or has low-pt. 
    /// There exists one instance of the EffiCollection foreach systematic variation and nominal. Scale-factor maps which are not affected by 
    /// a systematic, especially in the case of common vs. low-pt, are taken from the Nominal maps.
    class EffiCollection final {
        public:
            explicit EffiCollection(const MuonEfficiencyScaleFactors& ref_tool);
            ///Constructor with nominal as fallback..
            EffiCollection(const EffiCollection* Nominal, const MuonEfficiencyScaleFactors& ref_tool, const std::string& syst, int syst_bit_map, bool is_up);
          
            /// return the correct SF type to provide, depending on eta and the author
            EfficiencyScaleFactor* retrieveSF(const xAOD::Muon & mu, unsigned int RunNumber) const;
            enum CollectionType {
                /// The five different scale-factor maps
                Central = 1, 
                Calo   = 1<<1, 
                Forward = 1<<2, 
                CentralLowPt = 1<<3, 
                CaloLowPt = 1<<4, 
                
                ZAnalysis = Central | Calo | Forward,
                /// Distinguish these two because the systematics
                /// are named with an extra LOWPT
                JPsiAnalysis = CentralLowPt | CaloLowPt,                
               
               
            };
            enum Systematic{
                Symmetric = 1<<6,
                PtDependent = 1<<7,
                UnCorrelated = 1<<8,
                UpVariation = 1<<9,
            };
            static std::string FileTypeName(EffiCollection::CollectionType T);
            
            
            /// a consistency check of the scale-factor maps. All scale-factor maps must be present
            /// and there must no overlapping periods to pass this test.
            bool CheckConsistency();

            /// Get the number of all bins in the scale-factor maps including
            /// the overflow & underflow bins
            unsigned int nBins() const;
            
            /// If systematic decorrelation is activated then the user needs to loop
            /// manually over the syst bins. This method activates the i-th bin to 
            /// be active. For the remaining bins the nominal scale-factor is returned
            /// instead.
            bool SetSystematicBin(unsigned int Bin);
            
            /// Checks whether the i-th bin belongs to the low-pt map...
            bool IsLowPtBin(unsigned int Bin) const;
            /// Checks whether the i-th bin belongs to the forward map
            bool IsForwardBin(unsigned int Bin) const;

            /// Returns the global bin name conststucted from the axis
            /// titles and the bin borders
            std::string GetBinName(unsigned int bin) const;
            
            /// Returns the bin number from which the scale-factor of the muon
            /// is going to be retrieved...
            int getUnCorrelatedSystBin(const xAOD::Muon& mu) const;
            
            
            ///    Returns the systematic set affecting this collection.
            SystematicSet* getSystSet() const;
            ///    Returns whether the given set has variations affecting this Collection
            bool  isAffectedBySystematic(const SystematicVariation& variation) const;
            bool  isAffectedBySystematic(const SystematicSet& set) const;
            

            /// Method to retrieve a container from the class ordered by a collection type
            /// This method is mainly used to propagate the nominal maps to the variations
            /// as fallback maps if no variation has been defined in this situation
            std::shared_ptr<CollectionContainer> retrieveContainer(CollectionType Type) const;

        private:
            CollectionContainer* FindContainer(unsigned int bin) const;
            CollectionContainer* FindContainer(const xAOD::Muon& mu) const;
            
            const MuonEfficiencyScaleFactors& m_ref_tool;
            
            /// Make the collection container shared ptr to allow that a systematic EffiCollection
            /// can use the same container as the nominal one if the current systematic has no
            /// effect on that particular container....
            std::shared_ptr<CollectionContainer> m_central_eff;
            std::shared_ptr<CollectionContainer> m_calo_eff;
            std::shared_ptr<CollectionContainer> m_forward_eff;
            std::shared_ptr<CollectionContainer> m_lowpt_central_eff;
            std::shared_ptr<CollectionContainer> m_lowpt_calo_eff;

            /// The systematic set is returned back to the MuonEfficiencyScaleFactors instance to register
            /// The known systematics to the global service
            std::unique_ptr<SystematicSet> m_syst_set;        
    };
    
    /// The collection container manages the time binning of a particular scale-factor map. For a given runNumber,
    /// it pipes the right map to the upstream tools.
    class CollectionContainer final {
        public:
            /// Nominal constructor... Only needs to know about it's type and the file to load
            CollectionContainer(const MuonEfficiencyScaleFactors& ref_tool, EffiCollection::CollectionType FileType);
            CollectionContainer(const MuonEfficiencyScaleFactors& ref_tool, CollectionContainer* Nominal, const std::string& syst_name, unsigned int syst_bit_map);
          
            /// Retrieve the scale-factor map belonging to that particular run of data-taking
            EfficiencyScaleFactor* retrieve(unsigned int RunNumer) const;
            
            /// Checks if the global bin number belongs to this map
            bool isBinInMap (unsigned int bin) const;
            /// Consistency check of all scale-factor maps managed by the container instance
            bool CheckConsistency();
            
            /// Returns  MUON_EFF_<sysname()>
            std::string sysname() const;
            
            /// Activate this bin to run in the uncorrelated systematic mode
            bool SetSystematicBin(unsigned int Bin);
            
            /// Sets the global offset to align the order in the map into a
            /// global numbering scheme
            void SetGlobalOffSet(unsigned int OffSet);
            
            /// Number of bins of the map itself
            unsigned int nBins() const;
            /// Number of overflow bins in the map
            unsigned int nOverFlowBins() const;
            /// Check whether the bin is overflow or not
            bool isOverFlowBin(int b) const;

            
            /// Global offset of the bin numbers
            unsigned int globalOffSet() const;
           
            /// Name of the i-th bin 
            std::string GetBinName(unsigned int Bin) const;                    
            
            /// Returns the global bin number corresponding to the
            /// muon kinematics. In case of failures -1 is returned
            int FindBinSF(const xAOD::Muon &mu) const;
            
            /// File type of the map
            EffiCollection::CollectionType type() const;
            
            bool isNominal() const;
            bool isUpVariation() const;
            bool separateBinSyst() const;
            
        private:
            std::map<std::string, std::pair<unsigned int, unsigned int>> findPeriods(const MuonEfficiencyScaleFactors& ref_tool) const;
            std::string fileName(const MuonEfficiencyScaleFactors& ref_tool) const;
          
            bool LoadPeriod(unsigned int RunNumber) const;
           
            std::vector<std::shared_ptr<EfficiencyScaleFactor>> m_SF;
            mutable EfficiencyScaleFactor* m_currentSF;
            
            EffiCollection::CollectionType m_FileType;
            /// Offset to translate between the bin-numbers in the bin numbers
            /// of each file against the global bin-number
            unsigned int m_binOffSet;
        
    };

}
#endif /* EFFICOLLECTION_H_ */
