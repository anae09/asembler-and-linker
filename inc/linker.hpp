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
    int locationCounter;

    void loadFiles(std::list<std::string> inputFilenames);
    void checkIfUndef();
    void updateSymbolTable(FileInfo* file, std::string& section_name);
    void updateRelocationTable(FileInfo* file, std::string& section_name);
public:
    Linker(std::list<std::string> inputFilenames);
    void run();
    ~Linker();
};