// -*- c++ -*-
#ifndef CALOREC_CALOTOWERGEOMETRY_H
#define CALOREC_CALOTOWERGEOMETRY_H

/* Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration */

#include "Identifier/IdentifierHash.h"

#include "CaloGeoHelpers/CaloPhiRange.h"

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "GaudiKernel/MsgStream.h"

#include <string>
#include <vector>
#include <cmath>
#include <array>
#include <tuple>
#include <fstream>

class CaloTowerGeometry {
public:
  ///@{
  typedef std::size_t                  uint_t;           ///< Type for unsigned integer
  typedef IdentifierHash::value_type   index_t;          ///< Type for scalar (pseudorapidity,azimuth) index (is an unsigned int type)
  typedef std::tuple<index_t,double>   element_t;        ///< Type storing tower index and geometrical weight
  typedef std::vector<element_t>       elementvector_t;  ///< Type for list of elements holding tower index and list of weights
  typedef std::vector<elementvector_t> elementmap_t;     ///< Type for lists of lists of elements (lookup table type) 
  ///@}

  friend class CaloTowerGeometryCondAlg;

  ///@brief Standard constructor
  CaloTowerGeometry(const CaloDetDescrManager* caloDDM);

  CaloTowerGeometry() = delete;
  StatusCode initialize(MsgStream& msg);                 ///< Initialize object 

  // --- Full documentation of this block after end of class definition!
  ///@name Retrieve towers for cells
  ///@{
  StatusCode      access(IdentifierHash cellHash,std::vector<index_t>& towerIdx,std::vector<double>& towerWghts) const;
  elementvector_t getTowers(IdentifierHash cellHash)                                                             const; 
  ///@}

  ///@name Tower bin descriptors and other size information
  ///@{
  uint_t maxCellHash()      const; ///< Maximum cell hash value
  uint_t totalNumberCells() const; ///< Total number of cells 
  uint_t etaBins()          const; ///< Number of pseudorapidity bins
  double etaMin()           const; ///< Lower boundary of pseudorapidity range
  double etaMax()           const; ///< Upper boundary of pseudorapidity range 
  double etaWidth()         const; ///< Width of pseudorapidity bin  
  uint_t phiBins()          const; ///< Number of azimuth bins
  double phiMin()           const; ///< Lower boundary of azimuth
  double phiMax()           const; ///< Upper boundary of azimuth
  double phiWidth()         const; ///< Width of azimuth bin
  uint_t towerBins()        const; ///< Total number of towers
  double towerArea()        const; ///< Area of individual tower 
  ///@}

  ///@name Tower index calculators, translaters, manipulators and converters
  ///@{
  index_t etaIndex(IdentifierHash cellHash)          const; ///< Get tower @f$ \eta @f$ bin index for a calorimeter cell referenced by its hash identifier
  index_t etaIndex(double eta)                       const; ///< Get tower @f$ \eta @f$ bin index for a given value of @f$ \eta @f$
  index_t etaIndexFromTowerIndex(index_t towerIdx)   const; ///< Get tower @f$ \eta @f$ bin index for a given global tower index
  index_t phiIndex(IdentifierHash cellHash)          const; ///< Get tower @f$ \phi @f$ bin index for a calorimeter cell referenced by its hash identifier
  index_t phiIndex(double phi)                       const; ///< Get tower @f$ \phi @f$ bin index for a given value of @f$ \phi @f$
  index_t phiIndexFromTowerIndex(index_t towerIdx)   const; ///< Get tower @f$ \phi @f$ bin index for a given global tower index
  index_t towerIndex(IdentifierHash cellHash)        const; ///< Get global tower index for a calorimeter cell referenced by its hash identifier
  index_t towerIndex(double eta,double phi)          const; ///< Get global tower index for a pair of @f$ (\eta,\phi) @f$ values
  index_t towerIndex(index_t etaIdx,index_t phiIdx)  const; ///< Get global tower index for a pair of @f$ (\eta,\phi) @f$ indices
  index_t towerIndex(const element_t& elm)           const; ///< Get global tower index from payload data
  index_t invalidIndex()                             const; ///< Returns value of invalid index
  bool    isInvalidIndex(index_t idx)                const; ///< Returns @c true if argument is equal to the value provided by @c invalidIndex() 
  bool    isInvalidIndex(index_t idx,index_t maxIdx) const; ///< Returns @c true if first argument is equal to the value provided by @c invalidIndex() or if first argument is not smaller than second argument
  bool    isInvalidEtaIndex(index_t idx)             const; ///< Returns @c true if argument is not a valid pseudorapidity index
  bool    isInvalidPhiIndex(index_t idx)             const; ///< Returns @c true if argumant is not a valid azimuth index
  bool    isInvalidTowerIndex(index_t idx)           const; ///< Returns @c true if argument is not a valid tower index
  ///@}

  ///@name Variable generators using tower indices
  ///@{
  double towerEtaLocal(index_t etaIndex)    const; ///< Return pseudorapdity from local index (bin center) 
  double towerPhiLocal(index_t phiIndex)    const; ///< Return azimuth from local index (bin center)
  double towerEta(index_t towerIndex)       const; ///< Return pseudorapidity from global tower index (bin center)
  double towerPhi(index_t towerIndex)       const; ///< Return azimuth from global tower index (bin center)
  double invalidValue()                     const; ///< Return invalid value
  bool   isInvalidValue(double val)         const; ///< Return @c true if given value is invalid
  ///@}

  ///@name Helper functions
  ///@{
  double  cellWeight(const element_t& elm)                     const; ///< Retrieve cell signal weight from lookup table entry
  double  cellWeight(IdentifierHash cellHash,index_t towerIdx) const; ///< Retrieve cell signal weight from cell identifier and tower index
  ///@}

  ///@name Access to storage
  ///@{
  elementmap_t::const_iterator begin() const;  ///< Iterator points to first entry in internal look-up table (only @c const access!)
  elementmap_t::const_iterator end()   const;  ///< Iterator marks end of internal look-up table (only @c const access)  
  size_t                       size()  const;  ///< Size of internal look-up table
  bool                         empty() const;  ///< Internal look-up table is empty if @c true
  ///@}

private:

  ///@name Helpers
  ///@{
  StatusCode f_setupTowerGrid(MsgStream& msg);                                                              ///< Internally used function setting up the lookup store
  StatusCode f_setupTowerGridFCal(const CaloDetDescrElement* pCaloDDE, MsgStream& msg); ///< Internally used function mapping an FCal cell onto the tower grid
  StatusCode f_setupTowerGridProj(const CaloDetDescrElement* pCaloDDE, MsgStream& msg); ///< Internally used function mapping a projective cell onto the tower grid
  double     f_assign(IdentifierHash cellHash,index_t towerIdx,double wgt);                   ///< Internally used function assigning tower to cell with update of weight if same tower is already assigned
  ///@}

  ///@name Access to detector store and other services and stores
  ///@{
  const CaloDetDescrManager* m_caloDDM;               ///< Pointer to calorimeter detector description 
  ///@}

protected:

  ///@name Internal stores and derived parameters
  ///@{
  elementmap_t m_towerLookup;   ///< Cell-to-tower mapping lookup store
  double       m_towerEtaWidth; ///< Width of tower bin in pseudorapidity
  double       m_towerPhiWidth; ///< Width of tower bin in azimuth 
  double       m_towerArea;     ///< Area of individual tower
  uint_t       m_towerBins;     ///< Maximum number of towers
  uint_t       m_maxCellHash;   ///< Maximum cell hash value
  uint_t       m_numberOfCells{}; ///< Total number of cells 
  ///@}
  
  ///@name Properties
  ///@{
  ///@brief Internally stored tower grid descriptors
  uint_t m_towerEtaBins; ///< Number of @f$ \eta @f$ bins 
  double m_towerEtaMin;  ///< Lower boundary @f$ \eta_{\rm min} @f$
  double m_towerEtaMax;  ///< Upper boundary @f$ \eta_{\rm max} @f$
  bool   m_adjustEta;    ///< Adjust FCal cells to eta boundary (default @c true ) 
  uint_t m_towerPhiBins; ///< Number of @f$ \phi @f$ bins 
  double m_towerPhiMin;  ///< Lower boundary @f$ \phi_{\rm min} @f$
  double m_towerPhiMax;  ///< Upper boundary @f$ \phi_{\rm max} @f$
  double m_fcal1Xslice;  ///< Number of x slices for cells in FCal1
  double m_fcal1Yslice;  ///< Number of y slices for cells in FCal1
  double m_fcal2Xslice;  ///< Number of x slices for cells in FCal2
  double m_fcal2Yslice;  ///< Number of y slices for cells in FCal2
  double m_fcal3Xslice;  ///< Number of x slices for cells in FCal3
  double m_fcal3Yslice;  ///< Number of y slices for cells in FCal3
  ///@} 

  ///@name Process flags, helpers and numerical constants
  ///@{
  const static index_t m_invalidIndex;                                       ///< Invalid index indicator
  const static double  m_invalidValue;                                       ///< Return value for out-of-range indices andother invalid conversions to a physical quantity
  const CaloDetDescrManager* f_caloDDM()                        const; ///< Pointer to calorimeter detector description manager

  const CaloDetDescrElement* f_caloDDE(IdentifierHash cellHash) const; ///< Retrieve calorimeter detector description element for a given cell hash identifier

  double f_cellEta(IdentifierHash cellHash)                     const; ///< Retrieve calorimeter cell pseudorapidity for a given cell hash identifier

  double f_cellPhi(IdentifierHash cellHash)                     const; ///< Retrieve calorimeter cell azimuth for a given cell hash identifier
  ///@}

  ///@name Stores
  ///@{
  std::array<double,3> m_ndxFCal{}; ///< Stores number of fragments along x for each FCal module
  std::array<double,3> m_ndyFCal{}; ///< Stores number of fragments along y for each FCal module
  std::array<double,3> m_wgtFCal{}; ///< Stores geometrical weights
  ///@}
};

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( CaloTowerGeometry, 9072807, 1 )
#include "AthenaKernel/CondCont.h"
CONDCONT_DEF(CaloTowerGeometry, 52208619);



//-------------------------------------------------//
// Documentation for grouped methods and functions //
// (removed from before/after method for better    //
// formatting by doxygen in html).                 //
//-------------------------------------------------//

///@fn StatusCode CaloTowerGeometry::access(IdentifierHash cellHash,std::vector<index_t>& towerIdx,std::vector<double>& towerWghts) const;
///
///@brief Retrieve the list of towers associated with a calorimeter cell referenced its hash identifier
///
/// The tower indices and weights are returned in two index-parallel vectors. 
/// Previous content of these two vectors is removed if this method finds towers for the cell. 
/// 
///@return Returns @c StatusCode::SUCCESS if list of towers found, else @s StatusCode::FAILURE.
///
///@param[in] cellHash   hash identifier referencing a calorimeter cell.
///@param[in] towerIdx   reference to modifiable vector of indices (payload type @c index_t ); vector is filled if cell is successfully mapped. 
///@param[in] towerWghts reference to modifiable vector of weights (payload type @c double ); vector is filled if cell is successfully mapped.

///@fn CaloTowerGeometry::elementvector_t CaloTowerGeometry::getTowers(IdentifierHash cellHash) const; 
///
///@brief Retrieve the list of towers associated with a calorimeter cell referenced by its hash identifier
///
///@return Returns a vector of (index,weight) pairs as a @c elementvector_t container. The container is empty
/// if the cell does not have any overlap with a tower.
///
///@param[in] cellHash   hash identifier referencing a calorimeter cell.

//---------------------//
// Class documentation //
//---------------------//

/// @class CaloTowerGeometry
///
/// @brief Tower geometry store and description provider
///
/// This service sets up a lookup table storing the geometrical area overlap fraction of a calorimeter cell
/// with towers in a given grid. This lookup table is set up at instantiation of the service. It can only be 
/// defined at that time. The default setup is a @f$ \Delta\eta\times\Delta\phi = 0.1 \times \pi/32 @f$ grid.
/// Any regular grid can be constructed. The grid definition can be provided as property.
///
/// The cell-to-tower information is stored internally as a (random access) lookup table. For a given cell,
/// the hash index is used to retrieve a list of towers this cell overlaps with, and the overlap paramater
/// (area fraction used as a geometrical weight). This indices and geometrical weights are represented by
/// a list of pairs of @c int and @c double numbers. Each cell can potential overlap with more than one 
/// tower. A more detailed description of towers and the geometrical overlap is available on the
/// <a href="https://twiki.cern.ch/twiki/bin/view/AtlasSandboxProtected/CaloTowerPerformance">calorimeter tower project page</a>.  
///
/// The lookup table is implemented for random access and using the cell hash identifier to retrieve the 
/// requested list of twoer indices and weights. Several retrieval mechanisms are supported (see documentation
/// of the corresponding methods). 
///
/// To map the azimuth of a cell to a tower, @f$ -\pi < \phi < \pi @f$ is used (ATLAS standard). For
/// consistency, all @f$ \phi @f$ values are mapped into this range. 
///
/// The service inherits from @c AthService and thus from the @c Service base class in Gaudi. The managed tower grid
/// is defined by service properties, with the following naming convention:  
/// - pseudorapidity range
///     - number of bins <tt>TowerEtaBins</tt> (default 100)
///     - lower boundary of pseudorapidity range <tt>TowerEtaMin</tt> (default -5.0)
///     - upper boundary of pseudorapidity range <tt>TowerEtaMax</tt> (default  5.0)
/// - azimuth range
///     - number of bins <tt>TowerPhiBins</tt> (default 64)
///     - lower boundary of azimuthal range <tt>TowerPhiMin</tt> (default -&pi;)
///     - upper boundary of azimuthal range <tt>TowerPhiMax</tt> (default &pi;) 
/// 
/// Addtional properties of this service define the granularity of the cell splitting in the ATLAS FCal. This
/// is used to map the FCal readout cells (rectangular slabs) onto the tower grid and calculate the geometrical
/// (area) overlap fraction, which is used to distribute the cell energy to the towers. 
/// - horizontal FCal cell splitting (along @a x axis)
///     - number of @a x slices in FCal1 <tt>FCal1NSlicesX</tt> (default 4)
///     - number of @a x slices in FCal2 <tt>FCal2NSlicesX</tt> (default 4)
///     - number of @a x slices in FCal3 <tt>FCal3NSlicesX</tt> (default 6)
/// - vertical FCal cell splitting (along @a y axis)
///     - number of @a y slices in FCal1 <tt>FCal1NSlicesY</tt> (default 4)
///     - number of @a y slices in FCal2 <tt>FCal2NSlicesY</tt> (default 6)
///     - number of @a y slices in FCal3 <tt>FCal3NSlicesY</tt> (default 6)
///
/// @warning It is recommended to @b not change the parameters for the FCal cell slicing. This configuration option is provided for expert use for R & D purposes only.   
///  
/// @todo Allow regional grids (varying segmentation as function of @f$ \eta @f$ . This requires additional interfaces (or interface changes) and 
///       and modifications of the index construction.
///
/// @author Peter Loch <loch@physics.arizona.edu>
///

//------------------//
// Inline Functions //
//------------------//

//---------------------------//
// Control and configuration //
//---------------------------//
inline CaloTowerGeometry::index_t CaloTowerGeometry::invalidIndex() const { return m_invalidIndex; }

//------------------------------------//
// Public access to tower descriptors //
//------------------------------------//
inline CaloTowerGeometry::uint_t CaloTowerGeometry::maxCellHash()      const { return m_maxCellHash;   }
inline CaloTowerGeometry::uint_t CaloTowerGeometry::totalNumberCells() const { return m_numberOfCells; }  

inline CaloTowerGeometry::uint_t CaloTowerGeometry::etaBins()          const { return m_towerEtaBins;  }
inline double                       CaloTowerGeometry::etaMin()           const { return m_towerEtaMin;   }
inline double                       CaloTowerGeometry::etaMax()           const { return m_towerEtaMax;   }
inline double                       CaloTowerGeometry::etaWidth()         const { return m_towerEtaWidth; }

inline CaloTowerGeometry::uint_t CaloTowerGeometry::phiBins()          const { return m_towerPhiBins;  }
inline double                       CaloTowerGeometry::phiMin()           const { return m_towerPhiMin;   }
inline double                       CaloTowerGeometry::phiMax()           const { return m_towerPhiMax;   }
inline double                       CaloTowerGeometry::phiWidth()         const { return m_towerPhiWidth; }

inline CaloTowerGeometry::uint_t CaloTowerGeometry::towerBins()        const { return m_towerBins;     }
inline double                       CaloTowerGeometry::towerArea()        const { return m_towerArea;     } 

//----------------//
// Index checking //
//----------------//
inline bool CaloTowerGeometry::isInvalidIndex(index_t idx)                const { return idx == invalidIndex(); }
inline bool CaloTowerGeometry::isInvalidIndex(index_t idx,index_t maxIdx) const { return idx == invalidIndex() || idx >= maxIdx; }
inline bool CaloTowerGeometry::isInvalidEtaIndex(index_t idx)             const { return isInvalidIndex(idx,m_towerEtaBins); }
inline bool CaloTowerGeometry::isInvalidPhiIndex(index_t idx)             const { return isInvalidIndex(idx,m_towerPhiBins); }
inline bool CaloTowerGeometry::isInvalidTowerIndex(index_t idx)           const { return isInvalidIndex(idx,m_towerBins); }

//------------------------------//
// Index retrieval/construction //
//------------------------------// 

inline CaloTowerGeometry::index_t CaloTowerGeometry::towerIndex(IdentifierHash cellHash)       const { return towerIndex(etaIndex(cellHash),phiIndex(cellHash)); }
inline CaloTowerGeometry::index_t CaloTowerGeometry::towerIndex(double eta,double phi)         const { return towerIndex(etaIndex(eta),phiIndex(phi)); }
inline CaloTowerGeometry::index_t CaloTowerGeometry::towerIndex(index_t etaIdx,index_t phiIdx) const { return !isInvalidEtaIndex(etaIdx) && !isInvalidPhiIndex(phiIdx) ? phiIdx+etaIdx*m_towerPhiBins :  invalidIndex(); } 

inline CaloTowerGeometry::index_t CaloTowerGeometry::towerIndex(const element_t& elm)          const { return std::get<0>(elm); }

inline CaloTowerGeometry::index_t CaloTowerGeometry::etaIndexFromTowerIndex(index_t towerIdx)  const { return (index_t)(towerIdx/phiBins()); }
inline CaloTowerGeometry::index_t CaloTowerGeometry::phiIndexFromTowerIndex(index_t towerIdx)  const { return (index_t)(towerIdx%phiBins()); } 

//-----------------------------//
// Access to tower description //
//-----------------------------//

inline double CaloTowerGeometry::invalidValue()                const { return m_invalidValue; }
inline bool   CaloTowerGeometry::isInvalidValue(double val)    const { return val == invalidValue(); }
inline double CaloTowerGeometry::towerEtaLocal(index_t etaIdx) const { return !isInvalidEtaIndex(etaIdx) ? etaMin()+(static_cast<double>(etaIdx)+0.5)*etaWidth() : invalidValue(); }
inline double CaloTowerGeometry::towerPhiLocal(index_t phiIdx) const { return !isInvalidPhiIndex(phiIdx) ? phiMin()+(static_cast<double>(phiIdx)+0.5)*phiWidth() : invalidValue(); }
inline double CaloTowerGeometry::towerEta(index_t towerIdx)    const { return towerEtaLocal(etaIndexFromTowerIndex(towerIdx)); }
inline double CaloTowerGeometry::towerPhi(index_t towerIdx)    const { return towerPhiLocal(phiIndexFromTowerIndex(towerIdx)); }  

inline CaloTowerGeometry::elementmap_t::const_iterator CaloTowerGeometry::begin() const { return m_towerLookup.begin(); }
inline CaloTowerGeometry::elementmap_t::const_iterator CaloTowerGeometry::end()   const { return m_towerLookup.end(); }
inline size_t                                             CaloTowerGeometry::size()  const { return m_towerLookup.size(); }
inline bool                                               CaloTowerGeometry::empty() const { return m_towerLookup.empty(); }

//-------------------//
// Other data access //
//-------------------//
inline double CaloTowerGeometry::cellWeight(const element_t& elm)                   const { return std::get<1>(elm); }


//----------------------------------//
// Internal functions and accessors //
//----------------------------------//
inline const CaloDetDescrManager* CaloTowerGeometry::f_caloDDM()                        const { return m_caloDDM; } 
inline const CaloDetDescrElement* CaloTowerGeometry::f_caloDDE(IdentifierHash cellHash) const { return f_caloDDM()->get_element(cellHash); }

inline double CaloTowerGeometry::f_cellEta(IdentifierHash cellHash) const { return f_caloDDE(cellHash)->eta(); }
inline double CaloTowerGeometry::f_cellPhi(IdentifierHash cellHash) const { return CaloPhiRange::fix(f_caloDDE(cellHash)->phi()); }

#endif
