/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELRAWDATABYTESTREAM_PIXEL_RODDECODER_H
#define PIXELRAWDATABYTESTREAM_PIXEL_RODDECODER_H


#include "PixelRawDataByteStreamCnv/IPixelRodDecoder.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ServiceHandle.h"
#include "eformat/SourceIdentifier.h"

#include "PixelConditionsData/PixelCablingCondData.h"
#include "PixelConditionsData/PixelHitDiscCnfgData.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "PixelConditionsData/PixelByteStreamErrors.h"
#include "StoreGate/ReadCondHandleKey.h"
#include <atomic>
class PixelID;

class PixelRodDecoder : virtual public IPixelRodDecoder, public AthAlgTool {

  public:

    // Collection type
    typedef Pixel1RawData RDO;
    typedef InDetRawDataCollection< PixelRDORawData > PixelRawCollection;

    // constructor
    PixelRodDecoder(const std::string& type, const std::string& name,
        const IInterface* parent ) ;

    // destructor
    ~PixelRodDecoder();

    StatusCode initialize() override;
    StatusCode finalize() override;

    StatusCode fillCollection  (const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment *robFrag,
				IPixelRDO_Container* rdoIdc,
				IDCInDetBSErrContainer& decodingErrors,
				std::vector<IdentifierHash>* vecHash = NULL) const override;

    uint32_t getDataType(unsigned int rawDataWord, bool link_start) const;   // determine module word type

    ////// decode IBL module header word
    uint32_t decodeBCID_IBL(const uint32_t word) const; // decode IBL BCID from header word
    uint32_t decodeL1ID_IBL(const uint32_t word) const; // decode IBL L1ID from header word
    uint32_t decodeFeI4Bflag_IBL(const uint32_t word) const; // decode IBL FeI4 flag bit
    uint32_t decodeModule_IBL(const uint32_t word) const; // decode IBL module link number from header word

    ////// decode Pixel module header word
    uint32_t decodeBCID(const uint32_t word) const;   // decode BCID from header word
    uint32_t decodeL1ID(const uint32_t word) const;   // decode L1ID from header word
    uint32_t decodeL1IDskip(const uint32_t word) const;   // decode L1ID skips from header word
    uint32_t decodeModule(const uint32_t word) const;   // decode module link number from header word
    uint32_t decodeHeaderErrors(const uint32_t word) const;   // decode header errors from header word


    ////// decode Pixel and IBL module hit word
    uint32_t decodeRow(const uint32_t word, bool isIBLorDBM ) const;   // decode row number from hit word
    uint32_t decodeColumn(const uint32_t word, bool isIBLorDBM) const;   // decode column number from hit word
    uint32_t decodeToT(const uint32_t word, bool isIBLorDBM ) const;   // decode TimeOverThreshold value from hit word


    ////// decode IBL-only module (not-condensed) hit word
    uint32_t decodeLinkNumHit_IBL(const uint32_t word) const; // decode Link Number in the IBL not-condensed Hit word // At least temporarily not used, because IBL data format is not clear (Franconi, 17.06.2014)


    ////// decode Pixel-only module hit word
    uint32_t decodeFE(const uint32_t word) const;   // decode FE number from hit word


    ////// decode IBL module trailer word
    uint32_t decodeSkippedTrigTrailer_IBL(const uint32_t word) const; // decode the skipped trigger counter in the IBL trailer
    uint32_t decodeTrailerErrors_IBL (const uint32_t word) const; // decode IBL trailer errors, all together
    uint32_t decodeLinkNumTrailer_IBL(const uint32_t word) const; // decode the link number in the IBL trailer // At least temporarily not used, because IBL data format is not clear (Franconi, 17.06.2014)
    uint32_t decodeCondensedModeBit_IBL(const uint32_t word) const; // decode the Condensed mode bit in the IBL trailer


    ////// decode Pixel module trailer word
    uint32_t decodeTrailerErrors(const uint32_t word) const;   // decode trailer errors from trailer word
    uint32_t decodeTrailerBitflips(const uint32_t word) const;   // search for bitflips in wordparts supposed to be empty
    uint32_t decodeBcidTrailer_IBL(const uint32_t word) const;


    ////// decode Pixel module FE flags
    uint32_t decodeFEFlags2(const uint32_t word) const;   // decode FE flags from flag type 2 word
    uint32_t decodeMCCFlags(const uint32_t word) const;   // decode MCC flags from flag type 2 word


    ////// decode IBL module FE flags
    uint32_t decodeServiceCodeCounter_IBL(const uint32_t word) const; // decode the Service Code Counter from the "non-short" IBL FE flag word
    uint32_t decodeServiceCode_IBL(const uint32_t word) const; // decode the Service code from the "non-short" IBL FE Flag word
    uint32_t decodeLinkNumFEFlag_IBL(const uint32_t word) const; // decode the Link Number present in the IBL FE Flag word // At least temporarily not used, because IBL data format is not clear (Franconi, 17.06.2014)
    //   uint32_t decodeFEFlagBitFlips_IBL(const uint32_t word); // search for bitflips in word partes supposed to be empty // At least temporarily not used, because IBL data format is not clear (Franconi, 17.06.2014)


    ////// decode Pixel RawData word
    uint32_t decodeRawData(const uint32_t word) const; // decode raw data word for IBL and Pixel

    uint32_t extractFefromLinkNum (const uint32_t linkNum) const;
    uint32_t extractSLinkfromLinkNum (const uint32_t linkNum) const;

    bool CheckLinkStart(const bool Link_start, const int rodId, const unsigned int mLink, const unsigned int mBCID, const unsigned int mLVL1ID, const int LVL1A);

    uint32_t treatmentFEFlagInfo(unsigned int serviceCode, unsigned int serviceCodeCounter) const;

    constexpr static size_t ERROR_CONTAINER_MAX = 56506;  // see explanation in PixelRawDataProviderTool.cxx. Increased to accommodate space for the SvcCounter
  private:
    mutable std::atomic_uint m_masked_errors{};
    mutable std::atomic_uint m_numGenWarnings{};
    mutable std::atomic_uint m_numDuplicatedPixels{0};

    const unsigned m_maxNumGenWarnings{200};     // Maximum number of general warnings to print
    mutable std::atomic_uint m_numBCIDWarnings{};
    const unsigned m_maxNumBCIDWarnings{50};    // Maximum number of BCID and LVL1ID warnings to print
    BooleanProperty m_checkDuplicatedPixel{this, "CheckDuplicatedPixel", true, "Check duplicated pixels in fillCollection method"};

    mutable std::atomic_uint m_numInvalidIdentifiers{0};
    mutable std::atomic_uint m_numPreambleErrors{0};
    mutable std::atomic_uint m_numTimeOutErrors{0};
    mutable std::atomic_uint m_numLVL1IDErrors{0};
    mutable std::atomic_uint m_numBCIDErrors{0};
    mutable std::atomic_uint m_numFlaggedErrors{0};
    mutable std::atomic_uint m_numTrailerErrors{0};
    mutable std::atomic_uint m_numDisabledFEErrors{0};
    mutable std::atomic_uint m_numDecodingErrors{0};
    mutable std::atomic_uint m_numRODErrors{0};
    mutable std::atomic_uint m_numLinkMaskedByPPC{0};
    mutable std::atomic_uint m_numLimitError{0};

    const PixelID*              m_pixel_id=nullptr;

    ServiceHandle<InDetDD::IPixelReadoutManager> m_pixelReadout
    {this, "PixelReadoutManager", "PixelReadoutManager", "Pixel readout manager" };

    SG::ReadCondHandleKey<PixelCablingCondData> m_condCablingKey
    {this, "PixelCablingCondData", "PixelCablingCondData", "Pixel cabling key"};

    SG::ReadCondHandleKey<PixelHitDiscCnfgData> m_condHitDiscCnfgKey
    {this, "PixelHitDiscCnfgData", "PixelHitDiscCnfgData", "Pixel FEI4 HitDiscConfig key"};

    //! checks if data words do not look like header & trailer markers, return true if so, this is sign of data corruption
    bool checkDataWordsCorruption( uint32_t word ) const;

    //!< flags concerning the detector configuration; set at config time
    bool m_is_ibl_present{};
    bool m_is_ibl_module{};
    bool m_is_dbm_module{};

    //!< if the flag is set to true appropriate bits are set in event info
    StatusCode updateEventInfoIfEventCorrupted( bool isCorrupted ) const;

    //!< checks if all FEs have sent the same number of headers, if not, generate warning message
    void checkUnequalNumberOfHeaders( const unsigned int nFragmentsPerFE[8], uint32_t robId ) const;
    
    //!< if the check duplicated RDO flag is true, check that this RDO is unique (returns true if unique)
    inline bool thisRdoIsUnique(const Identifier & pixelRdo, std::unordered_set<Identifier> & pixelRdosSeenSoFar) const;

    //!< get local FEI4
    unsigned int getLocalFEI4(const uint32_t fe, const uint64_t onlineId) const;

    //!< in case of invalid ROB fragments set corresponding error flags in all linked modules.
    void propagateROBErrorsToModules(const PixelCablingCondData *pixCabling,
                                     uint32_t robId,
                                     std::array<uint64_t, PixelRodDecoder::ERROR_CONTAINER_MAX> &bsErrWord,
                                     IDCInDetBSErrContainer& decodingErrors,
                                     PixelByteStreamErrors::PixelErrorsEnum error_code,
                                     const char *error_description) const;
};

bool 
PixelRodDecoder::thisRdoIsUnique(const Identifier & pixelRdo, std::unordered_set<Identifier> & pixelRdosSeenSoFar) const{
  //The 'second' element of the pair returned by a set insert indicates whether the insert was successful.
  //If the element is NOT already in the set (has not been seen already), then this element is true.
  return (m_checkDuplicatedPixel and pixelRdosSeenSoFar.insert(pixelRdo).second);
}


#endif
