#include "statement_parts.hpp"

std::ostream& operator<<(std::ostream& os, const StatementParts* stm){
    os << stm->intrDescr[1] << stm->intrDescr[0];

    if (stm->regsDescr[1] != 0 && stm->regsDescr[0] != 0) {
        os << " " << stm->regsDescr[1] << stm->regsDescr[0];
    }
    if (stm->addrMode[1] != 0 && stm->addrMode[0] != 0) {
        os << " " << stm->addrMode[1] << stm->addrMode[0];
    }
    if (stm->dataHigh[1] != 0 && stm->dataHigh[0] != 0) {
        os << " " << stm->dataHigh[1] << stm->dataHigh[0];
    }
    if (stm->dataLow[1] != 0 && stm->dataLow[0] != 0) {
        os << " " << stm->dataLow[1] << stm->dataLow[0];
    }
    return os;
}