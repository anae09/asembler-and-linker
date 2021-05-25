#include "symbol.hpp"

std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
    os << symbol.name << "\t" << symbol.section << "\t" << symbol.offset << "\t"
    << (symbol.type == SymType::GLOBALSYM ? "G" : (symbol.type == SymType::LOCALSYM? "L" : ""));
    return os;
}

void Symbol::initSymbol(std::string name, std::string section, SymType type, int offset, unsigned int size,  bool is_section) {
    this->name = name;
    this->section = section;
    this->offset = offset;
    this->size = size;
    this->type = type;
    this->is_section = is_section;
}

void Symbol::initSymbol(std::string name, SymType type) {
    this->name = name;
    this->type = type;
}