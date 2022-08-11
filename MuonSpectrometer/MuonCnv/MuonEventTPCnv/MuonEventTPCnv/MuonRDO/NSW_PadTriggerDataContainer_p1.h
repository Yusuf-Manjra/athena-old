#ifndef MUONEVENTTPCNV_NSW_PADTRIGGERDATACONTAINER_P1_H
#define MUONEVENTTPCNV_NSW_PADTRIGGERDATACONTAINER_P1_H

#include "MuonEventTPCnv/MuonRDO/NSW_PadTriggerData_p1.h"

#include <vector>

namespace Muon {
struct NSW_PadTriggerDataContainer_p1
{
    std::vector<NSW_PadTriggerData_p1> m_collections;
};
} // namespace Muon

#endif // MUONEVENTTPCNV_NSW_PADTRIGGERDATACONTAINER_P1_H
