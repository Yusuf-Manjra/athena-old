/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RegionSelectorBase_H
#define RegionSelectorBase_H

#include "GaudiKernel/MsgStream.h"
#include "iostream"
#include "string"

namespace MuonCalib {

    class MuonFixedId;

    class RegionSelectorBase {
    public:
        /** Constructor */
        RegionSelectorBase() = default;
        /** Destructor */
        virtual ~RegionSelectorBase() = default;
        /** return the result of the logical operation */
        virtual bool Result(const MuonFixedId& region) const = 0;
        /** character output of region */
        virtual void Print(std::ostream& os) const = 0;
        /** create a region from a string*/
        static std::unique_ptr<RegionSelectorBase> GetRegion(const std::string& input);

    private:
        static std::unique_ptr<RegionSelectorBase> process_region(const std::string& input, unsigned int& i, bool is_in_braces);
        static void print_position(const std::string& input, const unsigned int& position, MsgStream* msgStr);
    };

}  // namespace MuonCalib

#endif
