#pragma once

#include <iostream>
#include "reloctype.hpp"

struct RelocationEntry {
    int offset;
    int index;
    RelocType type;
    void initEntry(int offset, int index, RelocType type);
    friend std::ostream& operator<<(std::ostream& os, const RelocationEntry& entry);
};