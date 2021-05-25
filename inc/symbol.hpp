#pragma once
#include <iostream>
#include "symtype.hpp"

struct Symbol {
    SymType type = SymType::LOCALSYM;
    std::string name;
    std::string section;
    bool is_section = false;
    unsigned int size;
    int offset;
    
    void initSymbol(std::string name, std::string section, SymType type, int offset = 0, unsigned int size = 0,  bool is_section=false);
    void initSymbol(std::string name, SymType type);
    
    void setType(SymType type);

    friend std::ostream& operator<<(std::ostream& os, const Symbol& symbol);
};