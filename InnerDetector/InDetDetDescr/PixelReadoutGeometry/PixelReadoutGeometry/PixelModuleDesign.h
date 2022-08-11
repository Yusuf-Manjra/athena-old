/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelModuleDesign.h
///////////////////////////////////////////////////////////////////
// (c) ATLAS Pixel Detector software
///////////////////////////////////////////////////////////////////


#ifndef PIXELREADOUTGEOMETRY_PIXELMODULEDESIGN_H
#define PIXELREADOUTGEOMETRY_PIXELMODULEDESIGN_H

// Base class
#include "InDetReadoutGeometry/SiDetectorDesign.h"

// Data member classes
#include "ReadoutGeometryBase/PixelDiodeMap.h"
#include "ReadoutGeometryBase/PixelReadoutScheme.h"
#include "PixelReadoutDefinitions/PixelReadoutDefinitions.h"

// Other includes
#include "CxxUtils/CachedUniquePtr.h"
#include "InDetIdentifier/PixelID.h"
#include "TrkSurfaces/RectangleBounds.h"

#include <memory>

namespace Trk{
 class SurfaceBounds;
}

namespace InDetDD {

    class PixelDiodeMatrix;
    
    /** @class PixelModuleDesign
     Class used to describe the design of a module
      (diode segmentation and readout scheme)
      
      - to get the PixelReadoutCell connected to a PixelDiode
      - to get the list of PixelDiodes connected to a PixelReadoutCell
      
      @author Alessandro Fornaini
      - modified: Grant Gorfine, Nick Styles, Andreas Salzburger
    */
    class PixelModuleDesign : public SiDetectorDesign {
      
      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////
    public:
    
      ///////////////////////////////////////////////////////////////////
      // Constructors/destructor:
      ///////////////////////////////////////////////////////////////////
      
      /** Constructor with parameters:
           local axis corresponding to eta direction
           local axis corresponding to phi direction
           local axis corresponding to depth direction
           thickness of silicon sensor
           number of circuits in one circuit column
           number of circuits in one circuit row
           number of cell columns per circuit
           number of cell rows per circuit
           number of diode columns connected to one circuit
           number of diode rows connected to one circuit */
      
      PixelModuleDesign(const double thickness,
                        const int circuitsPerColumn,
                        const int circuitsPerRow,
                        const int cellColumnsPerCircuit,
                        const int cellRowsPerCircuit,
                        const int diodeColumnsPerCircuit,
                        const int diodeRowsPerCircuit,
                        std::shared_ptr<const PixelDiodeMatrix> matrix,
                        InDetDD::CarrierType carrierType,
                        int readoutSide = -1,
                        bool is3D=false,
                        InDetDD::DetectorType detectorType = InDetDD::Undefined);
    
      // Destructor:
      virtual ~PixelModuleDesign() = default;
      
      ///////////////////////////////////////////////////////////////////
      // Const methods:
      ///////////////////////////////////////////////////////////////////
    
      /** Returns distance to nearest detector active edge 
       +ve = inside
       -ve = outside */
      virtual void distanceToDetectorEdge(const SiLocalPosition & localPosition, 
    				      double & etaDist, double & phiDist) const;
    
      /** readout or diode id -> position, size */
      virtual SiDiodesParameters parameters(const SiCellId & cellId) const;
      virtual SiLocalPosition localPositionOfCell(const SiCellId & cellId) const;
    
      /** readout id -> id of connected diodes */
      virtual int numberOfConnectedCells(const SiReadoutCellId & readoutId) const;
      virtual SiCellId connectedCell(const SiReadoutCellId & readoutId, int number) const;
    
      /** If cell is ganged return the other cell, otherwise return an invalid id. */
      virtual SiCellId gangedCell(const SiCellId & cellId) const;
    
      /** diode id -> readout id */
      virtual SiReadoutCellId readoutIdOfCell(const SiCellId & cellId) const;
    
      /** position -> id */
      virtual SiReadoutCellId readoutIdOfPosition(const SiLocalPosition & localPos) const;
      virtual SiCellId cellIdOfPosition(const SiLocalPosition & localPos) const;
    
      /** Check if cell is in range. Returns the original cellId if it is in range, otherwise it returns an invalid id. */
      virtual SiCellId cellIdInRange(const SiCellId & cellId) const;
    
      /** Helper method for stereo angle computation */
      virtual HepGeom::Vector3D<double> phiMeasureSegment(const SiLocalPosition&) const;
    
      /** Special method for SCT (irrelevant here): */
      virtual std::pair<SiLocalPosition,SiLocalPosition> endsOfStrip(const SiLocalPosition &position) const;
    
      /** Given row and column index of a diode, return position of diode center
         ALTERNATIVE/PREFERED way is to use localPositionOfCell(const SiCellId & cellId) or 
         rawLocalPositionOfCell method in SiDetectorElement.
         DEPRECATED */
       SiLocalPosition positionFromColumnRow(const int column, const int row) const;
      
      /** Get the neighbouring diodes of a given diode:
          Cell for which the neighbours must be found
          List of cells which are neighbours of the given one */
      virtual void neighboursOfCell(const SiCellId & cellId,
    				std::vector<SiCellId> &neighbours) const;
        
      /** Compute the intersection length of two diodes:
          return: the intersection length when the two diodes are projected on one
           of the two axis, or 0 in case of no intersection or problem
          input: the two diodes for which the intersection length is computed */
      double intersectionLength(const SiCellId &diode1, const SiCellId &diode2) const;
    
      /** Global sensor size: */
      double sensorLeftColumn() const;
      double sensorRightColumn() const;
      double sensorLeftRow() const;
      double sensorRightRow() const;
    
      /** Total number of diodes: */
      int numberOfDiodes() const;
    
      /** Total number of circuits:*/
      int numberOfCircuits() const;

      /** Number of circuits per column: */
      int numberOfCircuitsPerColumn() const;
    
      /** Number of circuits per row: */
      int numberOfCircuitsPerRow() const;

      /** Number of cell columns per circuit:*/
      int columnsPerCircuit() const;
    
      /** Number of cell rows per circuit:*/
      int rowsPerCircuit() const;
    
      /** Number of cell columns per module:*/
      int columns() const;
    
      /** Number of cell rows per module:*/
      int rows() const;
    
      /** Method to calculate length of a module*/
      virtual double length() const;
     
      /** Method to calculate average width of a module*/
      virtual double width() const;
     
      /** Method to calculate minimum width of a module*/
      virtual double minWidth() const;
     
      /** Method to calculate maximum width of a module*/
      virtual double maxWidth() const;
    
      /** Pitch in phi direction*/
      virtual double phiPitch() const;
    
      /** Pitch in phi direction*/
      virtual double phiPitch(const SiLocalPosition & localPosition) const;
    
      /** Pitch in eta direction*/
      virtual double etaPitch() const;
    
      /** Method to calculate eta width from a column range*/
      double widthFromColumnRange(const int colMin, const int colMax) const;
    
      /** Method to calculate phi width from a row range*/
      double widthFromRowRange(const int rowMin, const int rowMax) const;
    
      /** Test if near bond gap within tolerances
         Only relevant for SCT. Always returns false for pixels.*/
      virtual bool nearBondGap(const SiLocalPosition & localPosition, double etaTol) const;
    
      /** Return true if hit local direction is the same as readout direction.*/
      virtual bool swapHitPhiReadoutDirection() const;
      virtual bool swapHitEtaReadoutDirection() const;
    
      /** Element boundary*/
      virtual const Trk::SurfaceBounds & bounds() const; 
    
      virtual bool is3D() const;

      PixelReadoutTechnology getReadoutTechnology() const;

      virtual DetectorType type() const final;

      ///////////////////////////////////////////////////////////////////
      // Non-const methods:
      ///////////////////////////////////////////////////////////////////
    
    
      /** Add a new multiple connection for rows:
         lower diode row for which the connection scheme is given
         vector containing, for each diode, the readout cell row number 
         to which the corresponding diode is connected */
      void addMultipleRowConnection(const int lowerRow,
    				const std::vector<int> &connections);
    
      /** Indicate that it is a more complex layout where cells are not 
         lined up with each other. Eg bricking. Probably never will be needed. */
      void setGeneralLayout();

      /** Debug string representation */
      std::string debugStringRepr() const;


      ///////////////////////////////////////////////////////////////////
      // Private methods:
      ///////////////////////////////////////////////////////////////////
    private:
    
      PixelModuleDesign();
    
      // Copy constructor:
      PixelModuleDesign(const PixelModuleDesign &design);
      
      // Assignment operator:
      PixelModuleDesign &operator=(const PixelModuleDesign &design);
     
      
      ///////////////////////////////////////////////////////////////////
      // Private data:
      ///////////////////////////////////////////////////////////////////
    private:
      PixelDiodeMap m_diodeMap;
      PixelReadoutScheme m_readoutScheme;
      CxxUtils::CachedUniquePtr<Trk::RectangleBounds> m_bounds;
      bool m_is3D;
      InDetDD::DetectorType m_detectorType;
    
    };
    
    ///////////////////////////////////////////////////////////////////
    // Inline methods:
    ///////////////////////////////////////////////////////////////////
    
    inline void PixelModuleDesign::addMultipleRowConnection(const int lowerRow,
    							const std::vector<int> &connections)
    {
      m_readoutScheme.addMultipleRowConnection(lowerRow,connections);
    }
    
    inline double PixelModuleDesign::sensorLeftColumn() const
    {
      return m_diodeMap.leftColumn();
    }
    
    inline double PixelModuleDesign::sensorRightColumn() const
    {
      return m_diodeMap.rightColumn();
    }
    
    inline double PixelModuleDesign::sensorLeftRow() const
    {
      return m_diodeMap.leftRow();
    }
    
    inline double PixelModuleDesign::sensorRightRow() const
    {
      return m_diodeMap.rightRow();
    }
    
    inline int PixelModuleDesign::numberOfDiodes() const
    {
      return m_diodeMap.diodes();
    }
    
    inline int PixelModuleDesign::numberOfCircuits() const
    {
      return m_readoutScheme.numberOfCircuits();
    }

    inline int PixelModuleDesign::numberOfCircuitsPerColumn() const
    {
      return m_readoutScheme.numberOfCircuitsPerColumn();
    }

    inline int PixelModuleDesign::numberOfCircuitsPerRow() const
    {
      return m_readoutScheme.numberOfCircuitsPerRow();
    }

    inline int PixelModuleDesign::columnsPerCircuit() const
    {
      return m_readoutScheme.columnsPerCircuit();
    }
    
    inline int PixelModuleDesign::rowsPerCircuit() const
    {
      return m_readoutScheme.rowsPerCircuit();
    }
    
    inline int PixelModuleDesign::columns() const
    {
      return m_readoutScheme.columns();
    }
    
    inline int PixelModuleDesign::rows() const
    {
      return m_readoutScheme.rows();
    }
    
    inline bool PixelModuleDesign::nearBondGap(const SiLocalPosition &, double) const
    {
      // No bond gap in pixel module
      return false;
    }
    
    inline bool PixelModuleDesign::swapHitPhiReadoutDirection() const
    {
      return true;
    }
    
    inline bool PixelModuleDesign::swapHitEtaReadoutDirection() const
    {
      return true;
    }
    
    
    inline void PixelModuleDesign::neighboursOfCell(const SiCellId & cellId,
    						std::vector<SiCellId> &neighbours) const
    {
      return m_diodeMap.neighboursOfCell(cellId, neighbours);
    }
    
    inline double PixelModuleDesign::intersectionLength(const SiCellId &diode1,
    						    const SiCellId &diode2) const
    {
      return m_diodeMap.intersectionLength(diode1, diode2);
    }  


    
    inline bool PixelModuleDesign::is3D() const 
    { 
      return m_is3D; 
    } 

    inline PixelReadoutTechnology PixelModuleDesign::getReadoutTechnology() const {
      if (m_detectorType == InDetDD::DetectorType::PixelBarrel
          || m_detectorType == InDetDD::DetectorType::PixelEndcap
          || m_detectorType == InDetDD::DetectorType::PixelInclined)
      {
        return PixelReadoutTechnology::RD53;
      }

      const int maxRow = m_readoutScheme.rowsPerCircuit();
      const int maxCol = m_readoutScheme.columnsPerCircuit();
      if (maxRow*maxCol>26000) { return PixelReadoutTechnology::FEI4; }
      else                     { return PixelReadoutTechnology::FEI3; }
    }

    inline std::string PixelModuleDesign::debugStringRepr() const
    {
      return m_diodeMap.debugStringRepr();
    }

} // namespace InDetDD

#endif // READOUTGEOMETRYBASE_PIXELMODULEDESIGN_H
