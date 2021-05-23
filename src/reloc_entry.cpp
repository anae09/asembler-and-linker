#include "reloc_entry.hpp"

void RelocationEntry::initEntry(int offset, int index, RelocType type)
{
    this->index = index;
    this->offset = offset;
    this->type = type;
}

std::ostream &operator<<(std::ostream &os, const RelocationEntry &entry)
{
    os << entry.offset << "\t" << (entry.type == RelocType::ABSOLUTE ? "abs" : "rel")
       << "\t" << entry.index;
    return os;
}