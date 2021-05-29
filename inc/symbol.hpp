#pragma once
#include <iostream>
#include "symtype.hpp"

struct Symbol {
    SymType type = SymType::NOSYMTYPE;
    std::string name;
    std::string section;
    unsigned int size;
    int offset;
    
    void initSymbol(std::string name, std::string section, SymType type, int offset = 0, unsigned int size = 0);
    void initSymbol(std::string name, SymType type);
    
    void setType(SymType type);

    friend std::ostream& operator<<(std::ostream& os, const Symbol& symbol);
};