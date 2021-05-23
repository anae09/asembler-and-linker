#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <list>

#include "parser.hpp"
#include "symbol.hpp"
#include "reloc_entry.hpp"

class Assembly
{
private:
    std::ifstream input;
    std::ofstream output;
    std::list<ParserResult *> resultList;
    std::unordered_map<std::string,struct Symbol> symtab;
    std::unordered_map<std::string, std::list<struct RelocationEntry>> reloctab;

    static unsigned int index_gen;
    static std::string undefined_section;
    static std::string absolute_section;
    static const int WORD_SIZE = 2;
    static const int PAYLOAD_SIZE = 2;
    unsigned int locationCounter = 0;
    std::string currentSection;


    int checkResult(ParserResult *res, std::string line);
    int parseDirectiveFirstPass(ParserResult *res);
    int parseDirectiveSecondPass(ParserResult *res);
    void outputSymbolTable();
    void outputRelocTable();
    void generateRelocEntry(ParserResult* res);
    void outputHex(std::string hexValue);

    void addToSymbolTable(std::string name, std::string section, SymType type, int offset, bool is_section);
public:
    Assembly(std::string inputFilename, std::string outputFilename);
    int assemble();
    void printSymbolTable();
    void printRelocTable();
    ~Assembly();
};