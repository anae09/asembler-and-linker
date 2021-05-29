#pragma once
#include <iostream>
#include <unordered_map>
#include <list>
#include "section.hpp"
#include "symbol.hpp"

struct FileInfo {
    std::string filename;
    std::unordered_map<std::string, struct Section> section_table;
    std::list<std::string> symbols;
    int numSections, numRelTables, numSymtabEntries;
};