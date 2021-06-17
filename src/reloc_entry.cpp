#include "reloc_entry.hpp"

void RelocationEntry::initEntry(int offset, std::string symbol, RelocType type)
{
    this->symbol = symbol;
    this->offset = offset;
    this->type = type;
}

std::ostream &operator<<(std::ostream &os, const RelocationEntry &entry)
{
    std::string reloc_type;
    if (entry.type == RelocType::R_DATA_16) {
        reloc_type = "R_DATA_16";
    } else if (entry.type == RelocType::R_INSTR_16) {
        reloc_type = "R_INSTR_16";
    } else if (entry.type == RelocType::R_INSTR_PC16) {
        reloc_type = "R_INSTR_PC16";
    } else {
        reloc_type = "NONE";
    }
    os << entry.offset << "\t" << reloc_type << "\t" << entry.symbol;
    return os;
}