#pragma once
#include <iostream>
#include "symtype.hpp"

struct Symbol {
    std::string name;
    std::string section;
    int offset = 0;
    unsigned int size = 0;
    unsigned int index;
    SymType type = SymType::LOCALSYM;
    bool is_section = false;
    void initSymbol(std::string name, std::string section, unsigned int index, SymType type, int offset = 0, unsigned int size = 0,  bool is_section=false);
    void initSymbol(std::string name, SymType type, unsigned int index);
    friend std::ostream& operator<<(std::ostream& os, const Symbol& symbol);
};