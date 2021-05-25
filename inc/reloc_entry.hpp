#pragma once

#include <iostream>
#include "reloctype.hpp"

struct RelocationEntry {
    int offset;
    std::string symbol;
    RelocType type;
    void initEntry(int offset, std::string symbol, RelocType type);
    friend std::ostream& operator<<(std::ostream& os, const RelocationEntry& entry);
};