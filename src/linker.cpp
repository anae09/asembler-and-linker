#include "linker.hpp"
#include <regex>
#include <map>
#include <sstream>
#include <string>
#include <iomanip>

void Linker::addSection(std::string section_name, unsigned int start_addr)
{
    section_table[section_name].name = section_name;
    section_table[section_name].start_addr = start_addr;
}

void Linker::loadFiles(std::list<std::string> inputFilenames, bool linkable)
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

            if (!linkable && symbol.type != SymType::GLOBALSYM)
                continue;
            else if (linkable && symbol.type == SymType::LOCALSYM)
            {
                if (symbol.name.compare(symbol.section))
                {
                    if (symbol_table.find(symbol.name) == symbol_table.end())
                    {
                        file->symbols.push_back(symbol.name);
                        symbol_table[symbol.name] = symbol;
                    }
                    else
                    {
                        std::cout << "Error: Symbol [" << symbol.name << "] already in symbol table!" << std::endl;
                        exit(-1);
                    }
                }
                continue;
            }

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
            if (symbol.section.compare("UND"))
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

Linker::Linker(std::list<std::string> inputFilenames, std::string outputname, bool linkable)
{
    loadFiles(inputFilenames, linkable);
    outputFile.open("tests/" + outputname);
    if (!outputFile)
    {
        std::cout << "Error: file " << outputname << std::endl;
        exit(-1);
    }
}

void Linker::updateSymbolTable(FileInfo *file, std::string &section_name)
{
    std::list<std::string>::iterator iter;
    for (iter = file->symbols.begin(); iter != file->symbols.end(); iter++)
    {
        if (symbol_table[*iter].section.compare("ABS") && !symbol_table[*iter].section.compare(section_name))
            symbol_table[*iter].offset += file->section_table[section_name].start_addr;
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
        if (map_iter == sections_ordered.begin()) {
            start_address = map_iter->first;
        }
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

void Linker::runHex()
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
            files[i]->section_table[currentSection.name].start_addr = locationCounter;
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
                    updateRelocationTable(files[j], currentSection.name);
                }
            }
        }
    }
    //printOutput();
    //printSectionTable();
    //printSymbolTable();
    //printRelocTables();

    addSectionToSymtab();

    referenceRelocation();
    // printHexOutput();

    writeHexOutput();
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
                stream.clear();
                stream.str(std::string());
                struct RelocationEntry &reloc_entry = *iter;

                int refptr = reloc_entry.offset * 2;

                std::string addend = "";
                for (int i = 0; i < 4; i++)
                {
                    addend += output[refptr + i];
                }
                int x;
                stream << std::hex << addend;
                stream >> x;
                stream.clear();
                stream.str(std::string());
                if (reloc_entry.type == RelocType::RELATIVE)
                {
                    if (symbol_table[reloc_entry.symbol].type == SymType::LOCALSYM)
                    {
                        stream << std::uppercase << std::hex << (files[i]->section_table[reloc_entry.symbol].start_addr + x - reloc_entry.offset);
                    }
                    else
                    {
                        stream << std::uppercase << std::hex << (symbol_table[reloc_entry.symbol].offset + x - reloc_entry.offset);
                    }
                }
                else if (reloc_entry.type == RelocType::ABSOLUTE)
                {
                    if (symbol_table[reloc_entry.symbol].type == SymType::LOCALSYM)
                    {
                        stream << std::uppercase << std::hex << (files[i]->section_table[reloc_entry.symbol].start_addr + x);
                    }
                    else
                    {
                        stream << std::uppercase << std::hex << symbol_table[reloc_entry.symbol].offset;
                    }
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
    std::cout << "# sekcije" << std::endl;
    std::unordered_map<std::string, struct Section>::iterator iter;
    std::cout << "ime"
              << "\t"
              << "vel."
              << std::endl;
    for (iter = section_table.begin(); iter != section_table.end(); iter++)
    {
        std::cout << iter->second.name << "\t" << iter->second.size << std::endl;
    }
}

void Linker::writeSectionTable()
{
    outputFile << "# sekcije" << std::endl;
    std::unordered_map<std::string, struct Section>::iterator iter;
    outputFile << "ime"
              << "\t"
              << "vel."
              << std::endl;
    for (iter = section_table.begin(); iter != section_table.end(); iter++)
    {
        outputFile << iter->second.name << "\t" << iter->second.size << std::endl;
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

void Linker::printHexOutput()
{
    std::cout << std::setfill('0') << std::setw(4) << std::hex << start_address << ": "; // pocetna adresa
    for (unsigned int i = 0; i < output.size(); i++)
    {
        if (!output[i])
            continue;
        if (i > 0 && i % 16 == 0)
        {
            std::cout << std::endl;
            std::cout << std::setfill('0') << std::setw(4) << std::hex << (start_address+i) << ": ";
        }
        else if (i > 0 && i % 2 == 0)
            std::cout << " ";
        std::cout << output[i];
    }
    std::cout << std::endl;
}

void Linker::writeHexOutput()
{
    outputFile << std::setfill('0') << std::setw(4) << std::hex << start_address << ": "; // pocetna adresa
    for (unsigned int i = 0; i < output.size(); i++)
    {
        if (i > 0 && i % 16 == 0)
        {
            outputFile << std::endl;
            outputFile << std::setfill('0') << std::setw(4) << std::hex << (start_address+i) << ": ";
        }
        else if (i > 0 && i % 2 == 0)
            outputFile << " ";
        if (output[i])  outputFile << output[i];
        else outputFile << "0";
    }
    outputFile << std::endl;
}

Linker::~Linker()
{
    std::vector<struct FileInfo *>::iterator iter;
    for (iter = files.begin(); iter != files.end(); iter++)
    {
        delete *iter;
    }
    if (outputFile)
        outputFile.close();
}

void Linker::runLinkable()
{
    locationCounter = 0;
    for (int i = 0; i < files.size(); i++)
    {
        std::unordered_map<std::string, struct Section>::iterator section_iter;

        for (section_iter = files[i]->section_table.begin(); section_iter != files[i]->section_table.end(); section_iter++)
        {
            if (section_table.find(section_iter->first) != section_table.end())
                continue;
            locationCounter = 0;
            struct Section &currentSection = section_table[section_iter->first];

            currentSection.name = section_iter->first;
            currentSection.start_addr = locationCounter;
            currentSection.machine_code += section_iter->second.machine_code;
            currentSection.size += section_iter->second.size;
            locationCounter += section_iter->second.size;

            //updateSymbolTable(files[i], currentSection.name);
            //updateRelocationTable(files[i], currentSection.name);

            for (int j = i + 1; j < files.size(); j++)
            {

                if (files[j]->section_table.find(currentSection.name) != files[j]->section_table.end())
                {
                    files[j]->section_table[currentSection.name].start_addr = locationCounter;
                    currentSection.machine_code += files[j]->section_table[currentSection.name].machine_code;
                    currentSection.size += files[j]->section_table[currentSection.name].size;
                    locationCounter += files[j]->section_table[currentSection.name].size;

                    updateSymbolTable(files[j], currentSection.name);
                    updateRelocationTable(files[j], currentSection.name);
                }
            }
        }
    }
    printSymbolTable();
    printSectionTable();
    resolveRelocationLinkable();
    addSectionToSymtab();
    writeToOutputFile();
}

void Linker::resolveRelocationLinkable()
{
    for (int i = 0; i < files.size(); i++)
    {
        std::unordered_map<std::string, struct Section>::iterator section_iter;

        for (section_iter = files[i]->section_table.begin(); section_iter != files[i]->section_table.end(); section_iter++)
        {
            struct Section &currentSection = section_table[section_iter->first];
            struct Section &fileSection = section_iter->second;
            std::list<struct RelocationEntry>::iterator reloc_iter;
            for (reloc_iter = fileSection.relocTable.begin(); reloc_iter != fileSection.relocTable.end(); reloc_iter++)
            {
                if (symbol_table.find(reloc_iter->symbol) != symbol_table.end() && symbol_table[reloc_iter->symbol].type == SymType::GLOBALSYM) // u TS svi globalni
                {
                    if (!symbol_table[reloc_iter->symbol].section.compare(currentSection.name) && reloc_iter->type == RelocType::RELATIVE)
                    {
                        // sekcija simbola u relokac. zapisu == trenutnoj sekciji && PCREL ==> ne dodaje se relok. zapis
                        // izracunava se pomeraj odmah
                        int refptr = reloc_iter->offset * 2, num;
                        std::string hex_value = "";
                        for (int i = 0; i < 4; i++)
                        {
                            hex_value += currentSection.machine_code[refptr++];
                        }
                        std::stringstream stream;
                        stream << std::hex << hex_value;
                        stream >> num;
                        num = num + symbol_table[reloc_iter->symbol].offset - reloc_iter->offset;
                        stream.clear();
                        stream.str(std::string()); // clear
                        stream << std::uppercase << std::hex << num;
                        hex_value = stream.str();
                        refptr = reloc_iter->offset * 2;
                        for (int i = 3; i >= 0; i--)
                        {
                            if (i < hex_value.length())
                                currentSection.machine_code[refptr] = hex_value[hex_value.length() - 1 - i];
                            else
                                currentSection.machine_code[refptr] = '0';
                            refptr++;
                        }
                    }
                    else
                    {
                        currentSection.relocTable.push_back(*reloc_iter);
                        currentSection.numRelocEntries++;
                    }
                }
                else // sigurno je lokalni simbol
                {
                    if (reloc_iter->type == RelocType::RELATIVE && !reloc_iter->symbol.compare(currentSection.name))
                    {
                        // da li moze da se desi??
                        std::cout << "483" << std::endl;
                        exit(-5);
                    }
                    else
                    {
                        // saberi upisanu vrednost i vrednost sekcije i upisi je nazad
                        int refptr = reloc_iter->offset * 2, num;
                        std::string hex_value = "";
                        for (int i = 0; i < 4; i++)
                        {
                            hex_value += currentSection.machine_code[refptr++];
                        }
                        std::stringstream stream;
                        stream << std::hex << hex_value;
                        stream >> num;
                        // sabira se sa pocetnom adresom sekcije trenutnog fajla
                        num += files[i]->section_table[reloc_iter->symbol].start_addr;
                        stream.clear();
                        stream.str(std::string()); // clear
                        stream << std::uppercase << std::hex << num;
                        hex_value = stream.str();
                        refptr = reloc_iter->offset * 2;
                        for (int i = 3; i >= 0; i--)
                        {
                            if (i < hex_value.length())
                                currentSection.machine_code[refptr] = hex_value[hex_value.length() - 1 - i];
                            else
                                currentSection.machine_code[refptr] = '0';
                            refptr++;
                        }
                        currentSection.relocTable.push_back(*reloc_iter);
                        currentSection.numRelocEntries++;
                    }
                    // else
                    // {
                    //     std::string symbolname = currentSection.name + std::to_string(i);
                    //     symbol_table[symbolname].initSymbol(symbolname, currentSection.name, SymType::LOCALSYM, fileSection.start_addr, 0);
                    //     reloc_iter->symbol = symbolname;
                    //     currentSection.relocTable.push_back(*reloc_iter);
                    //     currentSection.numRelocEntries++;
                    // }
                }
            }
        }
    }
}

void Linker::writeToOutputFile()
{
    writeSectionTable();
    std::unordered_map<std::string, struct Section>::iterator iter_section;
    for (iter_section = section_table.begin(); iter_section != section_table.end(); iter_section++)
    {
        // section machine code
        outputFile << "# " << iter_section->second.name << std::endl;
        for (int i = 0; i < iter_section->second.machine_code.length(); i++)
        {
            if (i > 0 && i % 2 == 0)
                outputFile << " ";
            outputFile << iter_section->second.machine_code[i];
        }
        outputFile << std::endl;

        if (iter_section->second.relocTable.size() == 0) continue;

        outputFile << "# rel." << iter_section->second.name << std::endl;
        std::list<struct RelocationEntry>::iterator relocIter;
        std::list<struct RelocationEntry> &lst = iter_section->second.relocTable;
        outputFile << "offset"
                   << "\t"
                   << "tip"
                   << "\t"
                   << "simbol" << std::endl;
        for (relocIter = lst.begin(); relocIter != lst.end(); relocIter++)
        {
            outputFile << (*relocIter) << std::endl;
        }
    }

    // output symbol table
    std::unordered_map<std::string, struct Symbol>::iterator iter;
    outputFile << "# tabela simbola" << std::endl;
    outputFile << "ime"
               << "\t"
               << "sekcija"
               << "\t"
               << "vr"
               << "\t"
               << "vidlj"
               << std::endl;
    for (iter = symbol_table.begin(); iter != symbol_table.end(); iter++)
    {
        outputFile << iter->second << std::endl;
    }
}

// void Linker::printSections() {
//     std::cout << "# sekcije" << std::endl;
//     std::cout << "ime" << "\t" << "vel." << std::endl;
//     std::unordered_map<std::string, struct Section>::iterator iter;
//     for (iter = section_table.begin(); iter != section_table.end(); iter++)
//     {
//         std::cout << iter->second.name << "\t" << iter->second.size << std::endl;
//     }
// }

void Linker::addSectionToSymtab()
{
    std::unordered_map<std::string, struct Section>::iterator iter;
    struct Symbol symbol;
    for (iter = section_table.begin(); iter != section_table.end(); iter++)
    {
        struct Section &section = iter->second;
        symbol.initSymbol(section.name, section.name, SymType::LOCALSYM, section.start_addr, section.size);
        if (symbol_table.find(section.name) == symbol_table.end())
        {
            symbol_table[section.name] = symbol;
        }
        else
        {
            std::cout << "SECTION IN SYMBOL TABLE" << std::endl;
            exit(-2);
        }
    }
}
