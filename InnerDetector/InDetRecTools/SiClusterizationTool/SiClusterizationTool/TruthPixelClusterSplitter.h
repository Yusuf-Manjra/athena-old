/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TruthPixelClusterSplitter.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef INDETRECTOOLS_TruthPixelClusterSplitter_H
#define INDETRECTOOLS_TruthPixelClusterSplitter_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "InDetRecToolInterfaces/IPixelClusterSplitter.h"
#include "InDetPrepRawData/PixelClusterParts.h"
#include "InDetPrepRawData/PixelClusterSplitProb.h"

namespace InDet
{
    
  class TruthClusterizationFactory;
  class PixelCluster;
    
    /** @class TruthPixelClusterSplitter
        @author Roland.Jansky@cern.ch
    */
    class TruthPixelClusterSplitter : public AthAlgTool, virtual public IPixelClusterSplitter {
    public :
      /** Constructor*/
      TruthPixelClusterSplitter(const std::string &type,
                             const std::string &name,
                             const IInterface *parent);
      
      /** Destructor*/
      ~TruthPixelClusterSplitter() = default;
      
      /** AthAlgTool interface methods */
      StatusCode initialize();            
      StatusCode finalize();            
      
      /** take one, give zero or many */
      std::vector<InDet::PixelClusterParts> splitCluster(const InDet::PixelCluster& origCluster ) const;
      
      /** take one, give zero or many - with split probability object */
      std::vector<InDet::PixelClusterParts> splitCluster(const InDet::PixelCluster& origCluster, 
                                                         const InDet::PixelClusterSplitProb& spo) const;
      
    private:

      ToolHandle<TruthClusterizationFactory> m_truthClusterizationFactory{this, "NnClusterizationFactory", "InDet::NnClusterizationFactory/TruthClusterizationFactory"};

      DoubleProperty m_thresholdSplittingIntoTwoClusters{this, "ThresholdSplittingIntoTwoClusters", 0.95};
      DoubleProperty m_thresholdSplittingIntoThreeClusters{this, "ThresholdSplittingIntoThreeClusters", 0.90};
      BooleanProperty m_splitOnlyOnBLayer{this, "SplitOnlyOnBLayer", true};

    };
}
#endif
