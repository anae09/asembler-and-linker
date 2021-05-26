#include "assembly.hpp"

std::string Assembly::undefined_section = "UND";
std::string Assembly::absolute_section = "ABS";

Assembly::Assembly(std::string inputFilename, std::string outputFilename)
{
    input.open(inputFilename);
    if (!input)
    {
        std::cout << "Invalid filename" << std::endl;
        exit(-2);
    }
    output.open(outputFilename);
    if (!output)
    {
        std::cout << "File not created!" << std::endl;
    }
    symtab[undefined_section].initSymbol(undefined_section, undefined_section, SymType::NOSYMTYPE, 0, 0, true);
}

int Assembly::parseDirectiveFirstPass(ParserResult *res)
{
    if (res->dir->type == DirectiveType::END)
    {

        if (!currentSection.empty())
        {
            symtab[currentSection].size = locationCounter;
        }
        return 1;
    }
    if (res->dir->type == DirectiveType::GLOBAL || res->dir->type == DirectiveType::EXTERN)
    {
        for (std::string symbol : res->dir->args)
        {
            if (symtab.find(symbol) == symtab.end())
            {
                symtab[symbol].initSymbol(symbol, undefined_section, SymType::GLOBALSYM, 0, 0, false);
            }
            else
            {
                symtab[symbol].type = SymType::GLOBALSYM;
            }
        }
    }
    else if (res->dir->type == DirectiveType::SECTION)
    {
        if (!currentSection.empty())
        {
            symtab[currentSection].size = locationCounter;
        }
        locationCounter = 0;
        currentSection = res->dir->args.front();

        addToSymbolTable(currentSection, currentSection, SymType::LOCALSYM, 0, true);
    }
    else if (res->dir->type == DirectiveType::EQU)
    {
        std::string symbol_name = res->dir->args.front();
        res->dir->args.pop_front();
        std::string literal = res->dir->args.front();
        int literalValue = Parser::getLiteralValue(literal, res->line);

        addToSymbolTable(symbol_name, absolute_section, SymType::LOCALSYM, literalValue, false);
    }
    else if (res->dir->type == DirectiveType::WORD)
    {
        for (std::string arg : res->dir->args)
        {
            locationCounter += WORD_SIZE;
        }
    }
    else if (res->dir->type == DirectiveType::SKIP)
    {
        int skipBytes = Parser::getLiteralValue(res->dir->args.front(), res->line);
        locationCounter += skipBytes;
    }
    else if (res->dir->type == DirectiveType::UNDEFINED_DIR)
    {
        // std::cout << "Error: Undefined directive " << std::endl;
        // exit(-1);
        return -1;
    }
    return 0;
}

int Assembly::checkResult(ParserResult *res, std::string line)
{
    if (res->type == ParseType::COMMENT)
    {
        delete res;
        return 0;
    }
    resultList.push_back(res);
    if (res->type == ParseType::LABEL)
    {

        addToSymbolTable(res->symbol, currentSection, SymType::LOCALSYM, locationCounter, false);

        res = Parser::getInstance()->parse(line, true);
        if (res)
            checkResult(res, line);
    }
    else if (res->type == ParseType::INSTRUCTION)
    {
        locationCounter += res->size;
    }
    else if (res->type == ParseType::DIRECTIVE)
    {
        int status = parseDirectiveFirstPass(res);
        if(status == -1) {
            std::cout << "Error in line: " << line << "; undefined directive" << std::endl;
            exit(-1);
        }
        return status;
    }
    else
    {
        std::cout << "Error in line: " << line << std::endl;
        exit(-1);
    }
    return 0;
}

int Assembly::parseDirectiveSecondPass(ParserResult *res)
{
    if (res->dir->type == DirectiveType::END)
        return 1;
    else if (res->dir->type == DirectiveType::SECTION)
    {
        currentSection = res->dir->args.front();
        locationCounter = 0;
        output << "# " << currentSection << std::endl;
    }
    else if (res->dir->type == DirectiveType::SKIP)
    {
        int n = Parser::getLiteralValue(res->dir->args.front(), res->line);
        for (int i = 0; i < n; i++)
        {
            output << "00 ";
        }
        output << std::endl;
        locationCounter += n;
    }
    else if (res->dir->type == DirectiveType::WORD)
    {
        int n;
        for (std::string arg : res->dir->args)
        {
            if (Parser::isSymbol(arg))
            {
                if (symtab.find(arg) == symtab.end()) {
                    std::cout << "Error in line: " << res->line << "; Local symbol " << arg << " is undefined" << std::endl;
                    exit(-2);
                }
                    // symtab[arg].initSymbol(arg, undefined_section, SymType::GLOBALSYM);

                if (!symtab[arg].section.compare("ABS"))
                {
                    std::string hexValue = Parser::literalToHex(symtab[arg].offset);
                    outputHex(hexValue);
                    continue;
                }
                RelocationEntry entry;
                // output << "XX XX ";
                if (symtab[arg].type == SymType::GLOBALSYM)
                { // -> ABSOLUTE
                    entry.initEntry(locationCounter, arg, RelocType::ABSOLUTE);
                    output << "00 00 ";
                }
                else // LOCAL SYM
                {
                    // output {symbolOFfset}
                    entry.initEntry(locationCounter, symtab[arg].section, RelocType::ABSOLUTE);
                    std::string hexValue = Parser::literalToHex(symtab[arg].offset);
                    outputHex(hexValue);
                }
                reloctab[currentSection].push_back(entry);
            }
            else
            {
                std::string hexValue = Parser::literalToHex(arg, res->line);
                outputHex(hexValue);
            }
            locationCounter += WORD_SIZE;
        }
        output << std::endl;
    }
    return 0;
}

void Assembly::generateRelocEntry(ParserResult *res)
{
    RelocationEntry entry;
    if (symtab.find(res->symbol) == symtab.end())
    {
        std::cout << "Error in line: " << res->line << "; Local symbol " << res->symbol << " is undefined" << std::endl;
        exit(-2);
        // symtab[res->symbol].initSymbol(res->symbol, undefined_section, SymType::GLOBALSYM);
    }
    if (res->stm->reloc_type == RelocType::ABSOLUTE)
    {
        if (!symtab[res->symbol].section.compare("ABS"))
        {
            Parser::writeLiteralToHex(symtab[res->symbol].offset, res);
            return;
        }
        if (symtab[res->symbol].type == SymType::LOCALSYM)
        {
            entry.initEntry(locationCounter - PAYLOAD_SIZE, symtab[res->symbol].section, RelocType::ABSOLUTE);
            Parser::writeLiteralToHex(symtab[res->symbol].offset, res);
        }
        else // GLOBALSYM
        {
            entry.initEntry(locationCounter - PAYLOAD_SIZE, res->symbol, RelocType::ABSOLUTE);
            res->stm->setDataZero();
        }
    }
    else // PC_REL
    {
        if (symtab[res->symbol].type == SymType::GLOBALSYM || !symtab[res->symbol].section.compare("ABS")) // GLOBALSYM
        {
            entry.initEntry(locationCounter - PAYLOAD_SIZE, res->symbol, RelocType::RELATIVE);
            res->stm->setDataPCRel();
        }
        else // LOCALSYM
        {
            if (!symtab[res->symbol].section.compare(currentSection))
            {
                /* AKO PRIPADA ISTOJ SEKCIJI POMERAJ UVEK ISTI TJ. NEMA RELOK. ZAPISA */
                // Parser::getInstance()->writeLiteralToHex(symtab[res->symbol].offset - locationCounter - PAYLOAD_SIZE, res);
                Parser::writeLiteralToHex(symtab[res->symbol].offset - locationCounter, res);
                return;
            }
            Parser::writeLiteralToHex(symtab[res->symbol].offset - PAYLOAD_SIZE, res);

            entry.initEntry(locationCounter - PAYLOAD_SIZE, symtab[res->symbol].section, RelocType::RELATIVE);
        }
    }

    reloctab[currentSection].push_back(entry);
}

int Assembly::assemble()
{
    std::string line;
    ParserResult *res;
    bool end_defined = false;
    /* FIRST PASS */
    while (getline(input, line))
    {
        if (line.empty())
            continue;
        // std::cout << line << std::endl;
        res = Parser::getInstance()->parse(line);
        if (!res)
        {
            std::cout << "Error in line: " << line << "; couldn't parse" << std::endl;
            return -1;
        }

        if (checkResult(res, line))
        {
            end_defined = true;
            break;
        }
    }

    if (!end_defined)
    {
        std::cout << "Error: Missing .end directive" << std::endl;
        exit(-1);
    }

    //printSymbolTable();

    /* SECOND PASS */

    // std::cout << resultList.size() << std::endl;
    locationCounter = 0;
    for (ParserResult *res : resultList)
    {
        if (res->type == ParseType::DIRECTIVE)
        {
            if (parseDirectiveSecondPass(res))
                break;
        }
        else if (res->type == ParseType::INSTRUCTION)
        {
            locationCounter += res->size;
            if (res->stm->reloc_type != RelocType::NONE)
            {
                generateRelocEntry(res);
            }

            output << res->stm << std::endl;
        }
    }
    outputSymbolTable();
    outputRelocTable();

    //printSymbolTable();
    //printRelocTable();

    return 0;
}

void Assembly::addToSymbolTable(std::string name, std::string section, SymType type, int offset, bool is_section)
{
    if (symtab.find(name) == symtab.end())
    {
        symtab[name].initSymbol(name, section, type, offset, 0, is_section);
    }
    else if (!symtab[name].section.compare(undefined_section))
    {
        symtab[name].section = section;
        symtab[name].offset = offset;
    }
    else
    {
        std::cout << "Error: Symbol " << name << " already defined" << std::endl;
        exit(-3);
    }
}

void Assembly::outputHex(std::string hexValue)
{
    for (int i = 3; i >= 0; i--)
    {
        if (i == 1)
            output << " ";
        if (i < hexValue.length())
            output << hexValue[hexValue.length() - 1 - i];
        else
            output << "0";
    }
    output << " ";
}

void Assembly::printSymbolTable()
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
    for (iter = symtab.begin(); iter != symtab.end(); iter++)
    {
        std::cout << iter->second << std::endl;
    }
}

void Assembly::outputSymbolTable()
{
    std::unordered_map<std::string, struct Symbol>::iterator iter;
    output << "# tabela simbola" << std::endl;
    output << "ime"
           << "\t"
           << "sekcija"
           << "\t"
           << "vr"
           << "\t"
           << "vidlj"
           << std::endl;
    for (iter = symtab.begin(); iter != symtab.end(); iter++)
    {
        output << iter->second << std::endl;
    }
}

void Assembly::outputRelocTable()
{
    std::unordered_map<std::string, std::list<struct RelocationEntry>>::iterator iter;
    std::list<struct RelocationEntry>::iterator relocIter;
    for (iter = reloctab.begin(); iter != reloctab.end(); iter++)
    {
        output << "# rel." << iter->first << std::endl;
        std::list<struct RelocationEntry> &lst = iter->second;
        output << "offset"
               << "\t"
               << "tip"
               << "\t"
               << "simbol" << std::endl;
        for (relocIter = lst.begin(); relocIter != lst.end(); relocIter++)
        {
            output << (*relocIter) << std::endl;
        }
    }
}

void Assembly::printRelocTable()
{
    std::unordered_map<std::string, std::list<struct RelocationEntry>>::iterator iter;
    std::list<struct RelocationEntry>::iterator relocIter;
    for (iter = reloctab.begin(); iter != reloctab.end(); iter++)
    {
        std::cout << "# rel." << iter->first << std::endl;
        std::list<struct RelocationEntry> &lst = iter->second;
        std::cout << "offset"
                  << "\t"
                  << "tip"
                  << "\t"
                  << "simbol" << std::endl;
        for (relocIter = lst.begin(); relocIter != lst.end(); relocIter++)
        {
            std::cout << (*relocIter) << std::endl;
        }
    }
}

Assembly::~Assembly()
{
    if (input)
        input.close();
    if (output)
        output.close();
    for (ParserResult *res : resultList)
    {
        delete res;
    }
    resultList.clear();
}

