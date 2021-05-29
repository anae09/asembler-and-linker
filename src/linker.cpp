#include "linker.hpp"
#include <fstream>
#include <regex>
#include <map>
#include <sstream>

void Linker::addSection(std::string section_name, unsigned int start_addr)
{
    section_table[section_name].name = section_name;
    section_table[section_name].start_addr = start_addr;
}

void Linker::loadFiles(std::list<std::string> inputFilenames)
{
    std::list<std::string>::iterator filenamesIter;
    for (filenamesIter = inputFilenames.begin(); filenamesIter != inputFilenames.end(); filenamesIter++)
    {
        std::ifstream input("tests/" + *filenamesIter, std::ios::binary);
        if (!input)
        {
            std::cout << "File " << *filenamesIter << " not opened" << std::endl;
            exit(-2);
        }

        struct FileInfo *file = new FileInfo();
        file->filename = *filenamesIter;

        input.read((char *)&(file->numSections), sizeof(int));
        input.read((char *)&file->numRelTables, sizeof(int));
        input.read((char *)&file->numSymtabEntries, sizeof(int));

        for (int i = 0; i < file->numSections; i++)
        {
            struct Section s;
            struct RelocationEntry entry;

            std::getline(input, s.name, '\0');
            input.read((char *)&s.size, sizeof(int));
            std::getline(input, s.machine_code, '\0');
            input.read((char *)&s.numRelocEntries, sizeof(int));

            s.machine_code = std::regex_replace(s.machine_code, std::regex("\\s+"), "");

            //std::cout << s << std::endl;

            for (int j = 0; j < s.numRelocEntries; j++)
            {
                input.read((char *)&entry.offset, sizeof(int));
                input.read((char *)&entry.type, sizeof(RelocType));
                std::getline(input, entry.symbol, '\0');
                // std::cout << entry << std::endl;
                s.relocTable.push_back(entry);
            }
            file->section_table[s.name] = s;
        }

        struct Symbol symbol;

        for (int i = 0; i < file->numSymtabEntries; i++)
        {
            std::getline(input, symbol.name, '\0');
            std::getline(input, symbol.section, '\0');
            input.read((char *)&symbol.offset, sizeof(int));
            input.read((char *)&symbol.type, sizeof(SymType));

            //std::cout << symbol << std::endl;

            if (symbol.type != SymType::GLOBALSYM)
                continue;

            if (symbol_table.find(symbol.name) != symbol_table.end())
            {
                if (symbol.section.compare("UND") != 0 && symbol_table[symbol.name].section.compare("UND") != 0)
                {
                    std::cout << "Error: Symbol [" << symbol.name << "] already defined!" << std::endl;
                    exit(-1);
                }
                else if (symbol.section.compare("UND") != 0 && symbol_table[symbol.name].section.compare("UND") == 0)
                {
                    symbol_table[symbol.name].section = symbol.section;
                    symbol_table[symbol.name].offset = symbol.offset;
                }
            }
            else
            {
                symbol_table[symbol.name] = symbol;
            }

            file->symbols.push_back(symbol.name);
        }

        if (input)
            input.close();

        files.push_back(file);
    }
}

void Linker::checkIfUndef()
{
    std::unordered_map<std::string, struct Symbol>::iterator iter;
    for (iter = symbol_table.begin(); iter != symbol_table.end(); iter++)
    {
        if (!iter->second.section.compare("UND"))
        {
            std::cout << "Error: symbol " << iter->second.name << " undefined";
            exit(-1);
        }
    }
}

Linker::Linker(std::list<std::string> inputFilenames)
{
    loadFiles(inputFilenames);
}

void Linker::updateSymbolTable(FileInfo *file, std::string &section_name)
{
    std::list<std::string>::iterator iter;
    for (iter = file->symbols.begin(); iter != file->symbols.end(); iter++)
    {
        if (symbol_table[*iter].section.compare("ABS"))
            symbol_table[*iter].offset = file->section_table[section_name].start_addr;
    }
}

void Linker::updateRelocationTable(FileInfo *file, std::string &section_name)
{
    std::list<struct RelocationEntry>::iterator iter;
    for (iter = file->section_table[section_name].relocTable.begin();
         iter != file->section_table[section_name].relocTable.end(); iter++)
    {
        iter->offset += file->section_table[section_name].start_addr;
    }
}

void Linker::sectionPlacement()
{
    std::unordered_map<std::string, struct Section>::iterator iter;
    std::map<unsigned int, std::string> sections_ordered;

    for (iter = section_table.begin(); iter != section_table.end(); iter++)
    {
        sections_ordered.insert(std::pair<unsigned int, std::string>(iter->second.start_addr, iter->first));
    }

    std::map<unsigned int, std::string>::iterator map_iter;

    for (map_iter = sections_ordered.begin(); map_iter != sections_ordered.end(); map_iter++)
    {
        std::cout << map_iter->second << "\t" << map_iter->first << std::endl;
        if (map_iter->first < locationCounter)
        {
            std::cout << "Error: " << map_iter->second << " address overlap" << std::endl;
            exit(-3);
        }
        locationCounter = map_iter->first;
        std::string &currentSection = map_iter->second;
        for (int i = 0; i < files.size(); i++)
        {
            if (files[i]->section_table.find(currentSection) == files[i]->section_table.end())
                continue;
            files[i]->section_table[currentSection].start_addr = locationCounter;
            //section_table[currentSection].machine_code += files[i]->section_table[currentSection].machine_code;
            output.resize(2 * (locationCounter + files[i]->section_table[currentSection].size));
            for (int j = 0; j < files[i]->section_table[currentSection].machine_code.size(); j++)
            {
                output[locationCounter * 2 + j] = files[i]->section_table[currentSection].machine_code[j];
            }
            section_table[currentSection].size += files[i]->section_table[currentSection].size;
            locationCounter += files[i]->section_table[currentSection].size;

            updateSymbolTable(files[i], currentSection);
            updateRelocationTable(files[i], currentSection);
        }
    }
}

void Linker::run()
{
    checkIfUndef();
    // locationCounter = 0;
    sectionPlacement();

    for (int i = 0; i < files.size(); i++)
    {
        std::unordered_map<std::string, struct Section>::iterator section_iter;

        for (section_iter = files[i]->section_table.begin(); section_iter != files[i]->section_table.end(); section_iter++)
        {
            if (section_table.find(section_iter->first) != section_table.end())
                continue;
            struct Section &currentSection = section_table[section_iter->first];
            currentSection.name = section_iter->first;
            currentSection.start_addr = locationCounter;
            // currentSection.machine_code += section_iter->second.machine_code;
            output.resize(2 * (locationCounter + section_iter->second.size));
            for (int j = 0; j < section_iter->second.machine_code.size(); j++)
            {
                output[locationCounter * 2 + j] = section_iter->second.machine_code[j];
            }
            currentSection.size += section_iter->second.size;

            locationCounter += section_iter->second.size;

            updateSymbolTable(files[i], currentSection.name);
            updateRelocationTable(files[i], currentSection.name);

            for (int j = i + 1; j < files.size(); j++)
            {

                if (files[j]->section_table.find(currentSection.name) != files[j]->section_table.end())
                {
                    files[j]->section_table[currentSection.name].start_addr = locationCounter;
                    // currentSection.machine_code += files[j]->section_table[currentSection.name].machine_code;
                    output.resize(2 * (locationCounter + files[j]->section_table[currentSection.name].size));
                    for (int k = 0; k < files[j]->section_table[currentSection.name].machine_code.size(); k++)
                    {
                        output[locationCounter * 2 + k] = files[j]->section_table[currentSection.name].machine_code[k];
                    }
                    currentSection.size += files[j]->section_table[currentSection.name].size;
                    locationCounter += files[j]->section_table[currentSection.name].size;

                    updateSymbolTable(files[j], currentSection.name);
                    updateRelocationTable(files[i], currentSection.name);
                }
            }
        }
    }
    printOutput();
    //printSectionTable();
    //printSymbolTable();
    //printRelocTables();

    std::unordered_map<std::string, struct Section>::iterator iter;
    struct Symbol symbol;
    for (iter = section_table.begin(); iter != section_table.end(); iter++) {
        struct Section& section = iter->second;
        symbol.initSymbol(section.name, section.name, SymType::LOCALSYM, section.start_addr, section.size);
        if (symbol_table.find(section.name) == symbol_table.end()) {
            symbol_table[section.name] = symbol;
        } else {
            std::cout << "SECTION IN SYMBOL TABLE" << std::endl;
            exit(-2);
        }
    }

    referenceRelocation();
    printOutput();
}

void Linker::referenceRelocation()
{
    std::unordered_map<std::string, struct Section>::iterator section_iter;
    std::list<struct RelocationEntry>::iterator iter;
    std::stringstream stream;

    for (int i = 0; i < files.size(); i++)
    {
        for (section_iter = files[i]->section_table.begin(); section_iter != files[i]->section_table.end(); section_iter++)
        {
            std::list<struct RelocationEntry> &reloctab = section_iter->second.relocTable;
            if (reloctab.size() == 0)
                continue;
            struct Section &currentSection = section_iter->second;
            for (iter = reloctab.begin(); iter != reloctab.end(); iter++) // foreach reloc. entry where section = section_iter-second
            {
                stream.str(std::string());
                struct RelocationEntry &reloc_entry = *iter;

                int refptr = reloc_entry.offset * 2;

                if (reloc_entry.type == RelocType::RELATIVE)
                {
                    std::string addend = "";
                    for (int i = 0; i < 4; i++)
                    {
                        addend += output[refptr + i];
                    }
                    int x;
                    stream << std::hex << addend;
                    stream >> x;
                    stream.str(std::string());
                    stream << std::uppercase << std::hex << (x + symbol_table[reloc_entry.symbol].offset);
                }
                if (reloc_entry.type == RelocType::ABSOLUTE)
                {
                    stream << std::uppercase << std::hex << symbol_table[reloc_entry.symbol].offset;
                }
                std::string hex_value = stream.str();
                for (int i = 3; i >= 0; i--)
                {
                    if (i < hex_value.length())
                        output[refptr] = hex_value[hex_value.length() - 1 - i];
                    else
                        output[refptr] = '0';
                    refptr++;
                }
            }
        }
    }
}

void Linker::printSectionTable()
{
    std::cout << "### sekcije" << std::endl;
    std::unordered_map<std::string, struct Section>::iterator iter;
    std::cout << "# name"
              << "\t"
              << "size"
              << "\t"
              << "start_address" << std::endl;
    for (iter = section_table.begin(); iter != section_table.end(); iter++)
    {
        std::cout << iter->second.name << "\t" << iter->second.size << "\t" << iter->second.start_addr << std::endl;
    }
}

void Linker::printSymbolTable()
{
    std::unordered_map<std::string, struct Symbol>::iterator iter;
    std::cout << "# tabela simbola" << std::endl;
    std::cout << "ime"
              << "\t"
              << "sekcija"
              << "\t"
              << "vr"
              << "\t"
              << "vidlj"
              << std::endl;
    for (iter = symbol_table.begin(); iter != symbol_table.end(); iter++)
    {
        std::cout << iter->second << std::endl;
    }
}

void Linker::printRelocTables()
{
    std::unordered_map<std::string, struct Section>::iterator section_iter;
    std::list<struct RelocationEntry>::iterator iter;

    for (int i = 0; i < files.size(); i++)
    {
        for (section_iter = files[i]->section_table.begin(); section_iter != files[i]->section_table.end(); section_iter++)
        {
            std::list<struct RelocationEntry> &reloctab = section_iter->second.relocTable;
            if (reloctab.size() == 0)
                continue;
            std::cout << files[i]->filename << " #rel." << section_iter->first << std::endl;
            std::cout << "offset"
                      << "\t"
                      << "tip"
                      << "\t"
                      << "simbol" << std::endl;

            for (iter = reloctab.begin(); iter != reloctab.end(); iter++)
            {
                std::cout << (*iter) << std::endl;
            }
        }
    }
}

void Linker::printOutput()
{
    for (int i = 0; i < output.size(); i++)
    {
        if (!output[i])
            continue;
        if (i > 0 && i % 8 == 0)
            std::cout << std::endl;
        else if (i > 0 && i % 2 == 0)
            std::cout << " ";
        std::cout << output[i];
    }
    std::cout << std::endl;
}

Linker::~Linker()
{
    std::vector<struct FileInfo *>::iterator iter;
    for (iter = files.begin(); iter != files.end(); iter++)
    {
        delete *iter;
    }
}