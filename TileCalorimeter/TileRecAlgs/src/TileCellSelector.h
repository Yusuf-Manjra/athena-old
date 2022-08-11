// -*- C++ -*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECALGS_TILESELECTOR_H
#define TILERECALGS_TILESELECTOR_H

// Tile includes
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileDQstatus.h"
#include "TileConditions/ITileDCSTool.h"
#include "TileConditions/ITileBadChanTool.h"

// Calo includes
#include "CaloEvent/CaloCellContainer.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

// Gaudi incldues
#include "GaudiKernel/ToolHandle.h"

#include <string>
#include <vector>

class TileID;
class TileHWID;
class TileCell;
class TileCablingService;
class TileInfo;


/////////////////////////////////////////////////////////////////////////////

class TileCellSelector: public AthAlgorithm {
  public:
    TileCellSelector(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TileCellSelector();

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:

    int Are3FF(std::vector<float> & OptFilterDigits, int OptFilterGain, int ch_type);
    void printCell(const TileCell * cell);

    unsigned int m_counter;
    unsigned int m_accept;
    unsigned int m_minCell;
    unsigned int m_maxCell;
    unsigned int m_minChan;
    unsigned int m_maxChan;
    unsigned int m_jump;
    unsigned int m_const;
    unsigned int m_overLG;
    unsigned int m_overHG;
    unsigned int m_underLG;
    unsigned int m_underHG;
    unsigned int m_dqerr;
    unsigned int m_dmuerr;
    unsigned int m_warnerr;

    const TileID* m_tileID;
    const TileHWID* m_tileHWID;
    const TileCablingService* m_cabling;
    ToolHandle<ITileBadChanTool> m_tileBadChanTool{this, "TileBadChanTool", "TileBadChanTool", "Tile bad channel tool"};
    SG::ReadHandleKey<TileDQstatus> m_dqStatusKey{this, "TileDQstatus", "TileDQstatus", "TileDQstatus key"};
    ToolHandle<ITileDCSTool> m_tileDCS{this, "TileDCSTool", "TileDCSTool", "Tile DCS tool"};

    unsigned int m_runNum;
    unsigned int m_lumiBlock;
    unsigned int m_evtNum;
    unsigned int m_evtBCID;
    // flag will contain status of a given event
    // every 4 bits - status of partitions LBA,LBC,EBA,EBC
    // bits 0-3   - there is a signal above threshold in partitions
    // bits 4-7   - there are channels with underflow (sample=0) in partition (since rel 17.2.6.4)
    // bits 8-11  - there are channels with overflow (sample=1023) in partition (since rel 17.2.6.4)
    // bits 12-15 - there are at least 16 drawers with bad quality in partition
    // bits 16-19 - maximal length of consecutive bad area (since rel 17.2.6.5)
    // bits 20-23 - there are at least 16 drawers which are completely masked in partition
    // bits 24-27 - there are at least 16 drawers which do not send data in partition
    // bits 28-31 - reserved for global good/warning/bad status
    // bits 20-27 are also used for module number which gives warning status (since release 17.2.6.5)
    //            in case of warning we are sure that bits which indicates error are not filled
    unsigned int m_tileFlag;
    // global status: 0=OK, 1=warning, 2=error
    unsigned int m_tileError;

    std::vector<bool> m_chanBad;
    std::vector<float> m_chanEne;
    std::vector<float> m_chanTime;
    std::vector<float> m_chanDsp;
    std::vector<float> m_chanTDsp;
    std::vector<float> m_chanQua;
    std::vector<bool> m_chanSel;
    std::vector<bool> m_chanToSkip;
    std::vector<bool> m_drawerToSkip;

    bool m_readCells;
    bool m_readRawChannels;
    bool m_readDigits;

    SG::ReadHandleKey<CaloCellContainer> m_cellContainerKey{this,"CellContainerName",
                                                            "AllCalo", "Input Calo cell container key"};

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,"DigitsContainerName",
                                                               "TileDigitsFlt", "Input Tile digits container key"};

    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,"RawChannelContainerName",
                                                                        "TileRawChannelCnt", 
                                                                        "Input Tile raw channel container key"};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfo",
                                                      "EventInfo", "Input event info key"};

    float m_minEneCell;
    float m_maxEneCell;
    float m_minEneChan[3]{};
    float m_maxEneChan[3]{};
    float m_minTimeCell;
    float m_maxTimeCell;
    float m_minTimeChan[3]{};
    float m_maxTimeChan[3]{};
    int m_ptnEneCell;
    int m_ptnEneChan[3]{};
    int m_ptnTimeCell;
    int m_ptnTimeChan[3]{};
    int m_selectGain;
    bool m_skipGain[2]{};
#define ptnlength 5
    bool m_bitEneCell[ptnlength]{};
    bool m_bitTimeCell[ptnlength]{};
    bool m_bitEneChan[3][ptnlength]{};
    bool m_bitTimeChan[3][ptnlength]{};
    float m_secondMaxLevel;
    float m_jumpDeltaHG;
    float m_jumpDeltaLG;
    float m_pedDeltaHG;
    float m_pedDeltaLG;
    int m_constLength;
    int m_minBadDMU;
    int m_maxBadDMU;
    int m_minBadMB;
    bool m_skipEmpty;
    bool m_skipMasked;
    bool m_skipMBTS;
    bool m_checkDCS;
    bool m_checkJumps;
    bool m_checkDMUs;
    bool m_checkOverLG;
    bool m_checkOverHG;
    bool m_checkUnderLG;
    bool m_checkUnderHG;
    float m_overflowLG;
    float m_overflowHG;
    float m_underflowLG;
    float m_underflowHG;
    bool m_checkWarning;
    bool m_checkError;
    bool m_printOnly;

    std::vector<int> m_drawer;
    std::vector<int> m_drawerToCheck;
    std::vector<int> m_chanToCheck;

    int m_maxVerboseCnt;
    std::vector<int> m_nDrawerOff;

    std::string m_infoName;
    const TileInfo* m_tileInfo;
    float m_ADCmaxMinusEps = 0.0F;
    float m_ADCmaskValueMinusEps = 0.0F;
};

#endif // TILERECALGS_TILESELECTOR_H
