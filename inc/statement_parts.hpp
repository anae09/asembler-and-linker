#pragma once

#include <iostream>
#include "reloctype.hpp"

struct StatementParts {
    char intrDescr[2]; 
    char regsDescr[2]; // regsDescr[1] = DST_REG, regsDescr[0] = SRC_REG
    char addrMode[2];
    char dataHigh[2];
    char dataLow[2];
    RelocType reloc_type = RelocType::NONE;

    void setDataZero();
    void setDataPCRel();
    friend std::ostream& operator<<(std::ostream& os, const StatementParts* stm);
};