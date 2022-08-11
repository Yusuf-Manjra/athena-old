/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelReadoutScheme.cxx
//   Implementation file for class PixelReadoutScheme
///////////////////////////////////////////////////////////////////
// (c) ATLAS Pixel Detector software
///////////////////////////////////////////////////////////////////
// Version 2.1 08/06/2001 David Calvet
// Modified: Grant Gorfine
///////////////////////////////////////////////////////////////////

#include "ReadoutGeometryBase/PixelReadoutScheme.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "ReadoutGeometryBase/SiReadoutCellId.h"

namespace InDetDD {

// Constructor with parameters:
PixelReadoutScheme::PixelReadoutScheme(const int circuitsPerColumn,
				       const int circuitsPerRow,
				       const int cellColumnsPerCircuit,
				       const int cellRowsPerCircuit,
				       const int diodeColumnsPerCircuit,
				       const int diodeRowsPerCircuit) :
  m_numberOfCircuits(circuitsPerColumn*circuitsPerRow),
  m_numberOfCircuitsPerColumn(circuitsPerColumn),
  m_numberOfCircuitsPerRow(circuitsPerRow),
  m_columnsPerCircuit(cellColumnsPerCircuit),
  m_rowsPerCircuit(cellRowsPerCircuit),
  m_columns(circuitsPerRow*diodeColumnsPerCircuit),
  m_rows(circuitsPerColumn*diodeRowsPerCircuit)
{
  // protect against 0 columns or rows
  if (!m_columns) throw std::runtime_error("PixelReadoutScheme: Number of columns is 0");
  if (!m_rows) throw std::runtime_error("PixelReadoutScheme: Number of columns is 0");
}



// Number of cells connected to this readout. 2 means its ganged.
int 
PixelReadoutScheme::numberOfConnectedCells(const SiReadoutCellId & readoutId) const
{
  if (!readoutId.isValid()) return 0;
  
  // Do quick check to see if it is outside ganged pixel range.
  int phiIndex = readoutId.phiIndex();
  if (m_rowConnections.outsideReadout(phiIndex)) return 1;
  
  return m_rowConnections.numberOfConnectedCells(phiIndex);
  
}

// Id of diodes connected to this readout. 0 is the primary cell.
SiCellId 
PixelReadoutScheme::connectedCell(const SiReadoutCellId & readoutId, unsigned int number) const
{
  if (!readoutId.isValid()) return SiCellId();
  
  // Do quick check to see if it is outside ganged pixel range.
  int phiIndex = readoutId.phiIndex();
  if (m_rowConnections.outsideReadout(phiIndex)) return readoutId;

  int numConnected = m_rowConnections.numberOfConnectedCells(phiIndex);
  if (!numConnected || static_cast<int>(number) >= numConnected) return SiCellId();

  int newPhiIndex = m_rowConnections.connectedCell(phiIndex, number);
  return SiCellId(newPhiIndex, readoutId.etaIndex());
}  

// Id of the readout cell for this diode.
SiReadoutCellId
PixelReadoutScheme::readoutIdOfCell(const SiCellId & cellId) const
{
  if (!cellId.isValid()) return SiReadoutCellId();

  int phiIndex = cellId.phiIndex();
  int newPhiIndex = phiIndex;
  if (!m_rowConnections.outsideDiode(phiIndex)) {
    newPhiIndex = m_rowConnections.readoutOfDiode(phiIndex);
  } 
  
  return SiReadoutCellId(newPhiIndex, cellId.etaIndex()); 

}

SiCellId
PixelReadoutScheme::gangedCell(const SiCellId & cellId) const
{
  if (!cellId.isValid()) return cellId;
  int phiIndex = cellId.phiIndex();
  if (m_rowConnections.outsideReadout(phiIndex)) return SiCellId(); // Invalid ID
  int newPhiIndex = m_rowConnections.gangedCell(phiIndex); 
  if (newPhiIndex == phiIndex) return SiCellId(); // Invalid ID
  return SiCellId(newPhiIndex, cellId.etaIndex());
}


void PixelReadoutScheme::addMultipleRowConnection(const int lowerIndex,
						  const std::vector<int> &connections)
{
 
  m_rowConnections.set(lowerIndex, connections);
}


} // namespace InDetDD
