// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file CaloDetDescr/CaloSuperCellAlignTool.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2012
 * @brief Propagate alignent changes to supercell geometry.
 */


#ifndef CALOALIGNMENTTOOLS_CALOSUPERCELLALIGNTOOL_H
#define CALOALIGNMENTTOOLS_CALOSUPERCELLALIGNTOOL_H


#include "GeoModelInterfaces/IGeoAlignTool.h"
#include "CaloDetDescr/ICaloSuperCellAlignTool.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "CxxUtils/checker_macros.h"

class CaloSuperCellDetDescrManager;
class CaloDetDescrManager;
class CaloDetDescriptor;

/**
 * @brief Propagate alignent changes to supercell geometry.
 */
class ATLAS_NOT_THREAD_SAFE CaloSuperCellAlignTool
  : public extends2<AthAlgTool, ICaloSuperCellAlignTool, IGeoAlignTool>
{
public:
  /**
   * @brief Standard Gaudi tool constructor.
   * @param type The name of the tool type.
   * @param name The tool name.
   * @param parent The tool's Gaudi parent.
   */
  CaloSuperCellAlignTool (const std::string& type,
                          const std::string& name,
                          const IInterface* parent);


  /// Standard Gaudi initialize method.
  virtual StatusCode initialize();


  /**
   * @brief Callback function triggered when alignment constants are updated
   * in StoreGate.  It is called after CaloAlignTool to propagate
   * geometry changes from the offline to supercell versions.
   */
  virtual StatusCode align(IOVSVC_CALLBACK_ARGS);


  /**
   * @brief Update supercell geometry from the offline geometry.
   * @param mgr The supercell geometry manager.
   * @param cellmgr The offline geometry manager.
   */
  virtual StatusCode doUpdate (CaloSuperCellDetDescrManager* mgr,
                               const CaloDetDescrManager* cellmgr);


  /**
   * @brief Standard Gaudi @c queryInterface method.
   *
   * We need do implement this by hand because 
   * @c IGeoAlignTool doesn't use @c DeclareInterfaceID.
   */
  StatusCode queryInterface (const InterfaceID& riid, void** ppvIf);


private:
  /// Property: The CaloAlignTool that triggers us.
  ToolHandle<IGeoAlignTool> m_caloAlignTool;

  /// Property: Offline / SuperCell ID mapping tool
  ToolHandle<ICaloSuperCellIDTool> m_scidTool;

  /// Property: SG key for the supercell geometry manager.
  std::string m_scMgrKey;

  /// Property: SG key for the offline geometry manager.
  std::string m_mgrKey;


  /// Not implemented.
  CaloSuperCellAlignTool() = delete;
  CaloSuperCellAlignTool (const CaloSuperCellAlignTool&) = delete;
  CaloSuperCellAlignTool& operator= (const CaloSuperCellAlignTool&) = delete;
};


#endif // not CALODETDESCR_CALOSUPERCELLALIGNTOOL_H
