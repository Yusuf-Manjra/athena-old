/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSERVICES_TRIGCOOLUPDATEHELPER_H
#define TRIGSERVICES_TRIGCOOLUPDATEHELPER_H
/**
 * @file   TrigCOOLUpdateHelper.h
 * @brief  Helper tool for COOL updates
 * @author Frank Winklmeier
 */

// STL includes
#include <map>
#include <string>
#include <vector>

// Framework includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/StatusCode.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/EventIDBase.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

// TDAQ includes
#include "CTPfragment/CTPExtraWordsFormat.h"

class IIOVSvc;
class IIOVDbSvc;

/**
 * Struct to hold CLID <-> folder name mapping
 */
struct FolderInfo {
  CLID clid{CLID_NULL};
  std::string key;
};

/**
 * Folder update entry
 */
struct FolderUpdate {
  FolderUpdate() : lumiBlock(0), folderIndex(0), needsUpdate(true) {}
  explicit FolderUpdate(const CTPfragment::FolderEntry& entry) :
    lumiBlock(entry.lumiBlock),
    folderIndex(entry.folderIndex),
    needsUpdate(true) {}

  EventIDBase::number_type lumiBlock;
  CTPfragment::FolderIndex folderIndex;
  bool needsUpdate;
};

/**
 * @brief Tool to perform COOL updates during the run
 */
class TrigCOOLUpdateHelper : public AthAlgTool {
public:
  TrigCOOLUpdateHelper(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual StatusCode start() override;
  virtual StatusCode stop() override;

  /**
   * @brief Perform COOL udpates if needed
   * @param ctx  Event context
   */
  StatusCode hltCoolUpdate(const EventContext& ctx);

  /**
   * @brief Update specified COOL folder
   * @param folder  Folder name
   */
  StatusCode hltCoolUpdate(const std::string& folder);

  /**
   * @brief Return folder name to index
   * @param idx         Folder index
   * @param folderName  Returns folder name
   */
  StatusCode getFolderName(CTPfragment::FolderIndex idx, std::string& folderName) const;

  /**
   * @brief Decode COOL folder updates according to extra payload in CTP fragment
   */
  StatusCode extractFolderUpdates(const EventContext& ctx);

  /**
   * @brief Read information about existing COOL folders
   */
  StatusCode readFolderInfo ATLAS_NOT_THREAD_SAFE ();

private:
  /**
   * @brief Reset COOL folder
   * @param folder COOL folder name
   */
  StatusCode resetFolder(const std::string& folder);

  /**
   * @brief Reset list of COOL folders
   * @param folders List of folder names
   */
  StatusCode resetFolders(const std::vector<std::string>& folders);

  /// CLID/name mapping of COOL folders
  std::map<std::string, FolderInfo> m_folderInfo;

  /// Map to store scheduled/done COOL folder updates
  std::map<CTPfragment::FolderIndex, FolderUpdate> m_folderUpdates;

  /// Map to store the folder update index -> name mapping
  std::map<CTPfragment::FolderIndex, std::string> m_folderNames;

  // Services and Tools
  IIOVSvc*                           m_iovSvc{nullptr};
  IIOVDbSvc*                         m_iovDbSvc{nullptr};
  ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc;

  // Properties
  ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", {}, "Monitoring tool"};

  Gaudi::Property<std::string> m_coolFolderName{
    this, "CoolFolderMap", {}, "Name of COOL folder containing folder map"};

  Gaudi::Property<std::vector<std::string>> m_folders{
    this, "Folders", {}, "List of folders that can be updated during the run"};

  Gaudi::Property<uint32_t> m_ctpRobId{
    this, "CtpRobId", 0x770001, "ROB ID of CTP fragment containing the extra payload"};
};

#endif
