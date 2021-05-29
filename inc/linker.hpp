#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <list>
#include "symbol.hpp"
#include "file_info.hpp"

class Linker {
private:
    std::unordered_map<std::string, struct Symbol> symbol_table;
    std::unordered_map<std::string, struct Section> section_table;
    std::vector<struct FileInfo*> files;
    std::vector<char> output;
    int locationCounter = 0;

    void loadFiles(std::list<std::string> inputFilenames);
    void checkIfUndef();
    void updateSymbolTable(FileInfo* file, std::string& section_name);
    void updateRelocationTable(FileInfo* file, std::string& section_name);
    void sectionPlacement();
    void referenceRelocation();
    void printSectionTable();
    void printSymbolTable();
    void printRelocTables();
    void printOutput();
public:
    Linker(std::list<std::string> inputFilenames);
    void addSection(std::string section_name, unsigned int start_addr);
    void run();
    ~Linker();
};