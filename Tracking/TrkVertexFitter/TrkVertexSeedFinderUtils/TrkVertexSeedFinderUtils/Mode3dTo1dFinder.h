/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKVERTEXSEEDFINDERUTILS_MODE3DTO1DFINDER_H
#define TRKVERTEXSEEDFINDERUTILS_MODE3DTO1DFINDER_H


#include "GaudiKernel/ToolHandle.h"
#include "TrkVertexSeedFinderUtils/IMode3dFinder.h"
#include "AthenaBaseComps/AthAlgTool.h"


namespace Trk
{

  /**
   @class Mode3dFinder

   Algotool which calculates the mode of a 
   threedimensional distribution splitting it into three 
   monodimensional distribution and calculating the mode of these 
   distribution through the 1dModeFinder that you specify in the 
   job option file.

   @author Giacinto.Piacquadio@physik.uni-freiburg.de

   */

  class IMode1dFinder;
  
  class Mode3dTo1dFinder final : public extends<AthAlgTool, IMode3dFinder>
  {
  public:
    // Standard Athena constructor.
    Mode3dTo1dFinder (const std::string& t,
                      const std::string& n,
                      const IInterface*  p);


    virtual StatusCode initialize() override;

    
    /**
     * @brief Obtain the 3d-mode (position) from a list of positions
     *        (distribution in space)
     * @param vx Primary vertex x-coordinate.
     * @param vy Primary vertex y-coordinate.
     * @param points List of points with weights.
     */
    virtual Amg::Vector3D
    getMode (const double vx,
             const double vy,
             const std::vector<Trk::PositionAndWeight>& points) const override final;
    

    /**
     * @brief Obtain the 3d-mode (position) from a list of positions
     *        (distribution in space)
     * @param vx Primary vertex x-coordinate.
     * @param vy Primary vertex y-coordinate.
     * @param points List of points with weights.
     * @param info[out] Optionally returns an object for retrieving
     *                  additional information.  May be left null if additional
     *                  information is not available.
     */
    virtual Amg::Vector3D
    getMode (const double vx,
             const double vy,
             const std::vector<Trk::PositionAndWeight>& points,
             std::unique_ptr<IMode3dInfo>& info) const override final;
                                         

    /**
     * @brief Obtain the 3d-mode (position) from a list of positions
     *        (distribution in space)
     * @param vx Primary vertex x-coordinate.
     * @param vy Primary vertex y-coordinate.
     * @param points List of points --- unweighted!
     */
    virtual Amg::Vector3D
    getMode (const double vx,
             const double vy,
             const std::vector<Amg::Vector3D>& points) const override final;


    /**
     * @brief Obtain the 3d-mode (position) from a list of positions
     *        (distribution in space)
     * @param vx Primary vertex x-coordinate.
     * @param vy Primary vertex y-coordinate.
     * @param points List of points --- unweighted!
     * @param info[out] Optionally returns an object for retrieving
     *                  additional information.  May be left null if additional
     *                  information is not available.
     */
    virtual Amg::Vector3D
    getMode (const double vx,
             const double vy,
             const std::vector<Amg::Vector3D>& points,
             std::unique_ptr<IMode3dInfo>& info) const override final;
     

  private:
    ToolHandle< IMode1dFinder > m_mode1dfinder;
  };
}
#endif
