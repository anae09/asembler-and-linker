#include "assembly.hpp"

unsigned int Assembly::index_gen = 1;
std::string Assembly::undefined_section = "UND";

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
    /* Initialize symbol table */
    symtab[undefined_section].initSymbol(undefined_section, undefined_section, 0, SymType::NOSYMTYPE, 0, 0, true);
    
}

int Assembly::checkGlobals()
{
    std::unordered_map<std::string, struct Symbol>::iterator iter;
    for (iter = symtab.begin(); iter != symtab.end(); iter++)
    {
        if (iter->second.type == SymType::GLOBALSYM && iter->second.section.empty())
        {
            std::cout << "Error: "
                      << " symbol [" << iter->second.name << "]"
                      << " is undefined!" << std::endl;
            return -1;
        }
    }
    return 0;
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
                symtab[symbol].initSymbol(symbol, SymType::GLOBALSYM, index_gen++);
                symtab[symbol].section = undefined_section;
            }
            else
            {
                symtab[symbol].type = SymType::GLOBALSYM;
            }
            if (res->dir->type == DirectiveType::EXTERN)
            {
                symtab[symbol].section = undefined_section;
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
        if (symtab[currentSection].section.empty())
        {
            symtab[currentSection].initSymbol(currentSection, currentSection, index_gen++, SymType::LOCALSYM, 0, 0U, true);
        }
        else
        {
            std::cout << "Error: Symbol " << currentSection << " already defined!" << std::endl;
            exit(-3);
        }
    }
    else if (res->dir->type == DirectiveType::EQU)
    {
        std::string symbol_name = res->dir->args.front();
        res->dir->args.pop_front();
        std::string literal = res->dir->args.front();
        int literalValue = Parser::getInstance()->getLiteralValue(literal);
        if (symtab.find(symbol_name) == symtab.end())
        {
            symtab[symbol_name].initSymbol(symbol_name, "ABS", index_gen++, SymType::LOCALSYM, literalValue);
        }
        else if (!symtab[symbol_name].section.empty())
        {
            std::cout << "Error: Symbol " << symbol_name << " already defined!" << std::endl;
            exit(-3);
        }
        else
        { // section not defined, but is in symbol table; maybe global
            symtab[symbol_name].section = "ABS";
            symtab[symbol_name].offset = literalValue;
        }
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
        int skipBytes = Parser::getInstance()->getLiteralValue(res->dir->args.front());
        locationCounter += skipBytes;
    }
    else if (res->dir->type == DirectiveType::UNDEFINED_DIR)
    {
        std::cout << "Error: Undefined directive " << std::endl;
        exit(-1);
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
        if (symtab.find(res->symbol) == symtab.end())
        {
            symtab[res->symbol].initSymbol(res->symbol, currentSection, index_gen++, SymType::LOCALSYM, locationCounter);
        }
        else if (symtab[res->symbol].section.empty())
        {
            symtab[res->symbol].offset = locationCounter;
            symtab[res->symbol].section = currentSection;
        }
        else
        {
            std::cout << "Error in line: " << line << std::endl;
            std::cout << "Symbol " << res->symbol << " already defined!" << std::endl;
            exit(-3);
        }

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
        return parseDirectiveFirstPass(res);
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
        int n = Parser::getInstance()->getLiteralValue(res->dir->args.front());
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
            if (Parser::getInstance()->isSymbol(arg))
            {
                if (symtab.find(arg) == symtab.end())
                {
                    symtab[arg].initSymbol(arg, undefined_section, index_gen++, SymType::GLOBALSYM);
                }
                if (!symtab[arg].section.compare("ABS"))
                {
                    std::string hexValue = Parser::getInstance()->literalToHex(symtab[arg].offset);
                    for (int i = 3; i >= 0; i--)
                    {
                        if (i == 1)
                            output << " ";
                        if (i < hexValue.length())
                            output << hexValue[hexValue.length() - 1 - i];
                        else
                            output << "0";
                    }
                    continue;
                }
                RelocationEntry entry;
                // output << "XX XX ";
                if (symtab[arg].type == SymType::GLOBALSYM)
                { // -> ABSOLUTE
                    entry.initEntry(locationCounter, symtab[arg].index, RelocType::ABSOLUTE);
                    output << "00 00 ";
                }
                else // LOCAL SYM
                {
                    // output {symbolOFfset}
                    entry.initEntry(locationCounter, symtab[symtab[arg].section].index, RelocType::ABSOLUTE);
                    std::string hexValue = Parser::getInstance()->literalToHex(symtab[arg].offset);
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
                reloctab[currentSection].push_back(entry);
            }
            else
            {
                std::string hexValue = Parser::getInstance()->literalToHex(arg);
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
        symtab[res->symbol].initSymbol(res->symbol, undefined_section, index_gen++, SymType::GLOBALSYM);
    }
    if (res->stm->reloc_type == RelocType::ABSOLUTE)
    {
        if (!symtab[res->symbol].section.compare("ABS"))
        {
            Parser::getInstance()->writeLiteralToHex(symtab[res->symbol].offset, res);
            return;
        }
        if (symtab[res->symbol].type == SymType::LOCALSYM)
        {
            entry.initEntry(locationCounter - PAYLOAD_SIZE, symtab[symtab[res->symbol].section].index, RelocType::ABSOLUTE);
            Parser::getInstance()->writeLiteralToHex(symtab[res->symbol].offset, res);
        }
        else // GLOBALSYM
        {
            entry.initEntry(locationCounter - PAYLOAD_SIZE, symtab[res->symbol].index, RelocType::ABSOLUTE);
            res->stm->dataHigh[1] = '0';
            res->stm->dataHigh[0] = '0';
            res->stm->dataLow[1] = '0';
            res->stm->dataLow[0] = '0';
        }
    }
    else // PC_REL
    {
        if (symtab[res->symbol].type == SymType::GLOBALSYM || !symtab[res->symbol].section.compare("ABS")) // GLOBALSYM
        {
            entry.initEntry(locationCounter - PAYLOAD_SIZE, symtab[res->symbol].index, RelocType::RELATIVE);
            res->stm->dataHigh[1] = 'F';
            res->stm->dataHigh[0] = 'F';
            res->stm->dataLow[1] = 'F';
            res->stm->dataLow[0] = 'E';
        }
        else // LOCALSYM
        {
            if (!symtab[res->symbol].section.compare(currentSection))
            {
                /* AKO PRIPADA ISTOJ SEKCIJI POMERAJ UVEK ISTI TJ. NEMA RELOK. ZAPISA */
                // Parser::getInstance()->writeLiteralToHex(symtab[res->symbol].offset - locationCounter - PAYLOAD_SIZE, res);
                Parser::getInstance()->writeLiteralToHex(symtab[res->symbol].offset - locationCounter, res);
                return;
            }
            Parser::getInstance()->writeLiteralToHex(symtab[res->symbol].offset - PAYLOAD_SIZE, res);

            entry.initEntry(locationCounter - PAYLOAD_SIZE, symtab[symtab[res->symbol].section].index, RelocType::RELATIVE);
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
        if (!res) {
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

    printSymbolTable();

    // if (checkGlobals() < 0)
    //     exit(-4);

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

    printSymbolTable();
    printRelocTable();

    return 0;
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
              << "\t"
              << "rBr" << std::endl;
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
           << "\t"
           << "rBr" << std::endl;
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
               << "rbr" << std::endl;
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
                  << "rbr" << std::endl;
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