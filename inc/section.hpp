#pragma once
#include <iostream>

struct Section {
        std::string name;
        int size;
        // unsigned int start_addr;
        std::string machine_code = "";
        int numRelocEntries;
        Section() {}
        Section(std::string name, int size);
        friend std::ostream& operator<<(std::ostream& os, struct Section& s);
};