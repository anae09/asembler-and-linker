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
    static const int UNDEF = 0;
    static const int WORD_SIZE = 2;
    static const int PAYLOAD_SIZE = 2;
    unsigned int locationCounter = 0;
    std::string currentSection;


    int checkResult(ParserResult *res, std::string line);
    int parseDirectiveFirstPass(ParserResult *res);
    int parseDirectiveSecondPass(ParserResult *res);
    int checkGlobals();
    void outputSymbolTable();
    void outputRelocTable();
    void generateRelocEntry(ParserResult* res);
public:
    Assembly(std::string inputFilename, std::string outputFilename);
    int assemble();
    void printSymbolTable();
    void printRelocTable();
    ~Assembly();
};