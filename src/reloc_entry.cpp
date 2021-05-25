#include "reloc_entry.hpp"

void RelocationEntry::initEntry(int offset, std::string symbol, RelocType type)
{
    this->symbol = symbol;
    this->offset = offset;
    this->type = type;
}

std::ostream &operator<<(std::ostream &os, const RelocationEntry &entry)
{
    os << entry.offset << "\t" << (entry.type == RelocType::ABSOLUTE ? "R_16" : "R_PC16")
       << "\t" << entry.symbol;
    return os;
}