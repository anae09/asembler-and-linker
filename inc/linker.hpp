#pragma once
#include <iostream>
#include <fstream>
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
    std::ofstream outputFile;
    int locationCounter = 0;

    void loadFiles(std::list<std::string> inputFilenames, bool linkable);
    void checkIfUndef();
    void updateSymbolTable(FileInfo* file, std::string& section_name);
    void updateRelocationTable(FileInfo* file, std::string& section_name);
    void sectionPlacement();
    void referenceRelocation();
    void resolveRelocationLinkable();
    void printSectionTable();
    void printSymbolTable();
    void printRelocTables();
    void printHexOutput();
    void writeHexOutput();
    void writeToOutputFile();
    void addSectionToSymtab();
public:
    Linker(std::list<std::string> inputFilenames, std::string outputname, bool linkable = false);
    void addSection(std::string section_name, unsigned int start_addr);
    void runHex();
    void runLinkable();
    ~Linker();
};