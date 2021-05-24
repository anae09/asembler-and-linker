#include "reloc_entry.hpp"

void RelocationEntry::initEntry(int offset, int index, RelocType type)
{
    this->index = index;
    this->offset = offset;
    this->type = type;
}

std::ostream &operator<<(std::ostream &os, const RelocationEntry &entry)
{
    os << entry.offset << "\t" << (entry.type == RelocType::ABSOLUTE ? "R_16" : "R_PC16")
       << "\t" << entry.index;
    return os;
}