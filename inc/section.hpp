#pragma once
#include <iostream>
#include <list>
#include "reloc_entry.hpp"

struct Section {
        std::string name;
        int size = 0;
        unsigned int start_addr = 0;
        std::string machine_code = "";
        int numRelocEntries = 0;

        std::list<struct RelocationEntry> relocTable;

        Section() {}
        Section(std::string name, int size);

        friend std::ostream& operator<<(std::ostream& os, struct Section& s);
};