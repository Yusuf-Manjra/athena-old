/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAM_MDTCSMREADOUT_H
#define MUONBYTESTREAM_MDTCSMREADOUT_H

#include <stdint.h>

#include "MdtReadOut.h"

// Decoding methods for an MDT Csm block
// S. Rosati Feb 2003

class MdtCsmReadOut : public MdtReadOut {
private:
    // Raw data word and word header
    uint32_t m_dataWord;
    uint16_t m_wordHeader;

    // Decoded data
    uint32_t m_lvl1Id;  // Lvl1 Id
    uint32_t m_clwc;    // Link word count
    uint16_t m_ftyp;    // flag type
    uint32_t m_flags;   // flags

    uint16_t m_ecnt;  // Event counter
    uint16_t m_ctwc;  // Trailer word count

    uint16_t m_mrodId;  // MROD number
    uint16_t m_csmId;   // CSM Link number
    bool m_error;       // Error flag
    bool m_zeroSupp;    // TDC zero-suppressed flag

    // Word header Position and values
    // The word header occupies the 8 highest bits of each CSM word
    static constexpr uint16_t s_headerPos = 24;
    static constexpr uint16_t s_headerBits = 0xff;

    // Beginning of buffer
    static constexpr uint16_t s_BOBvalue = 0x80;
    // Link Word Count: first word of a CSM link block
    static constexpr uint16_t s_LWCvalue = 0x81;
    // Beginning of Link: signals which CSM link is giving its data
    static constexpr uint16_t s_BOLvalue = 0x18;
    // TDC Link Present
    static constexpr uint16_t s_TLPvalue = 0x89;
    // Trailer Word Count: last word of a CSM link block
    static constexpr uint16_t s_TWCvalue = 0x8a;
    // End of Block (end of data from the CSMs)
    static constexpr uint16_t s_EOBvalue = 0xf0;

public:
    MdtCsmReadOut();
    ~MdtCsmReadOut() = default;

    void decodeWord(uint32_t dataWord);

    bool is_BOB() const { return m_wordHeader == s_BOBvalue; };
    bool is_LWC() const { return m_wordHeader == s_LWCvalue; };
    bool is_BOL() const { return m_wordHeader == s_BOLvalue; };
    bool is_TLP() const { return m_wordHeader == s_TLPvalue; };
    bool is_TWC() const { return m_wordHeader == s_TWCvalue; };
    bool is_EOB() const { return m_wordHeader == s_EOBvalue; };

    // Methods to access the decoded information
    uint32_t lvl1Id() const { return m_lvl1Id; }
    uint32_t clwc() const { return m_clwc; }
    uint16_t ftyp() const { return m_ftyp; }
    uint32_t flags() const { return m_flags; }
    uint16_t ecnt() const { return m_ecnt; }
    uint16_t ctwc() const { return m_ctwc; }
    uint16_t mrodId() const { return m_mrodId; }
    uint16_t csmId() const { return m_csmId; }

    // Word encoders
    uint32_t makeBOB(uint32_t lvl1Id);
    uint32_t makeLWC(uint32_t clwc);
    uint32_t makeBOL(uint16_t mrodId, uint16_t csmId);
    uint32_t makeTLP(uint32_t flags);
    uint32_t makeTWC(uint16_t ecnt, uint16_t ctwc);
    uint32_t makeEOB(uint32_t wcnt);

private:
    // Private methods
    void setZero();
};

#endif  // MUONBYTESTREAM_MDTCSMREADOUT_H
