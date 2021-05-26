#include <iostream>
#include <vector>
#include <sstream>
#include <regex>
#include <string.h>
#include <stdexcept>
#include "parser.hpp"

Parser *Parser::parser = nullptr;

void Parser::init_instructions_map()
{
    instructions["halt"].instrDescr = "00";
    instructions["halt"].type = InstrType::RETURN;

    instructions["int"].instrDescr = "10";
    instructions["int"].type = InstrType::SYSCALL;
    instructions["iret"].instrDescr = "20";
    instructions["iret"].type = InstrType::RETURN;

    instructions["call"].instrDescr = "30";
    instructions["call"].type = InstrType::CALL;

    instructions["ret"].instrDescr = "40";
    instructions["ret"].type = InstrType::RETURN;

    instructions["jmp"].instrDescr = "50";
    instructions["jmp"].type = InstrType::JUMP;
    instructions["jeq"].instrDescr = "51";
    instructions["jeq"].type = InstrType::JUMP;
    instructions["jne"].instrDescr = "52";
    instructions["jne"].type = InstrType::JUMP;
    instructions["jgt"].instrDescr = "53";
    instructions["jgt"].type = InstrType::JUMP;

    instructions["xchg"].instrDescr = "60";
    instructions["xchg"].type = InstrType::ARITHMETIC;

    instructions["add"].instrDescr = "70";
    instructions["add"].type = InstrType::ARITHMETIC;
    instructions["sub"].instrDescr = "71";
    instructions["sub"].type = InstrType::ARITHMETIC;
    instructions["mul"].instrDescr = "72";
    instructions["mul"].type = InstrType::ARITHMETIC;
    instructions["div"].instrDescr = "73";
    instructions["div"].type = InstrType::ARITHMETIC;
    instructions["cmp"].instrDescr = "74";
    instructions["cmp"].type = InstrType::ARITHMETIC;

    instructions["not"].instrDescr = "80";
    instructions["not"].type = InstrType::ARITHMETIC;
    instructions["and"].instrDescr = "81";
    instructions["and"].type = InstrType::ARITHMETIC;
    instructions["or"].instrDescr = "82";
    instructions["or"].type = InstrType::ARITHMETIC;
    instructions["xor"].instrDescr = "83";
    instructions["xor"].type = InstrType::ARITHMETIC;
    instructions["test"].instrDescr = "84";
    instructions["test"].type = InstrType::ARITHMETIC;

    instructions["shl"].instrDescr = "90";
    instructions["shl"].type = InstrType::ARITHMETIC;
    instructions["shr"].instrDescr = "91";
    instructions["shr"].type = InstrType::ARITHMETIC;

    instructions["ldr"].instrDescr = "A0";
    instructions["ldr"].type = InstrType::LDST;
    instructions["str"].instrDescr = "B0";
    instructions["str"].type = InstrType::LDST;
}

Parser::Parser()
{
    re_comment = comment_pattern;
    re_label = label_pattern;
    re_directive = directive_pattern;
    re_immediate = immediate_pattern;
    re_memdir = memdir_pattern;
    re_regdir = regdir_pattern;
    re_regind = regind_pattern;
    re_jmp_regdir = jmp_regdir_pattern;
    re_jmp_regind = jmp_regind_pattern;
    re_jmp_memdir_abs = jmp_memdir_abs_pattern;
    re_jmp_memdir_pc = jmp_memdir_pc_pattern;
    init_instructions_map();
}

std::string Parser::ltrim(const std::string &s)
{
    return std::regex_replace(s, std::regex("^\\s+"), std::string(""));
}
std::string Parser::rtrim(const std::string &s)
{
    return std::regex_replace(s, std::regex("\\s+$"), std::string(""));
}

std::string Parser::trim(const std::string &s)
{
    return ltrim(rtrim(s));
}

Parser *Parser::getInstance()
{
    if (parser == nullptr)
    {
        parser = new Parser();
    }
    return parser;
}

DirectiveType Parser::parseDirective(std::string dir)
{
    if (!dir.compare(".global"))
        return DirectiveType::GLOBAL;
    if (!dir.compare(".extern"))
        return DirectiveType::EXTERN;
    if (!dir.compare(".section"))
        return DirectiveType::SECTION;
    if (!dir.compare(".word"))
        return DirectiveType::WORD;
    if (!dir.compare(".skip"))
        return DirectiveType::SKIP;
    if (!dir.compare(".equ"))
        return DirectiveType::EQU;
    if (!dir.compare(".end"))
        return DirectiveType::END;
    return DirectiveType::UNDEFINED_DIR;
}

char Parser::parseRegister(std::string &reg, std::string& line)
{
    // if (!reg.compare("sp"))
    //     return '6';
    // if (!reg.compare("pc"))
    //     return '7';
    // if (!reg.compare("psw"))
    //     return '8';
    if (reg[0] == 'r'){
        if (reg.size() > 2 || reg[1] > '8') {
            std::cout << "Error in line: " << line << "; undefined register: " << reg << std::endl;
            exit(-1);
        }
        return reg[1];
    }
    
    std::cout << "Error in line: " << line << "; undefined register: " << reg << std::endl;
    exit(-1);

    return -1;
}

void Parser::parseDirectiveArguments(std::list<std::string> &args, std::string token)
{
    token = std::regex_replace(token, std::regex("(\\s+)|(#.*)"), "");
    std::stringstream args_stream(token);
    std::string tmp;
    while (getline(args_stream, tmp, ','))
    {
        args.push_back(tmp);
    }
}

void Parser::parseInstructionArguments(std::vector<std::string> &args, std::string token)
{
    //token = std::regex_replace(token, std::regex("(\\s+)|(#.*)"), "");
    std::stringstream args_stream(token);
    std::string tmp;
    while (getline(args_stream, tmp, ','))
    {
        args.push_back(tmp);
    }
}

void Parser::parseLiteral(ParserResult *res, std::string arg)
{
    std::regex re_hex("^0x.*");
    if (!std::regex_match(arg, re_hex))
    {
        int num;
        try
        {
            num = stoi(arg);
        }
        catch (std::invalid_argument)
        {
            std::cout << "Error in line: " << res->line <<"; " << arg << " is invalid" << std::endl;
            exit(-1);
        }
        catch (std::out_of_range)
        {
            std::cout << "Error in line: " << res->line << "; " << arg << " is out of range" << std::endl;
            exit(-1);
        }

        std::stringstream stream;
        stream << std::uppercase << std::hex << num;
        arg = stream.str();
    }
    else
    {
        arg = std::regex_replace(arg, std::regex("0x"), "");
    }
    int i = arg.size() - 1;
    res->stm->dataLow[0] = arg[i--];
    if (i >= 0)
        res->stm->dataLow[1] = arg[i--];
    else
        res->stm->dataLow[1] = '0';
    if (i >= 0)
        res->stm->dataHigh[0] = arg[i--];
    else
        res->stm->dataHigh[0] = '0';
    if (i >= 0)
        res->stm->dataHigh[1] = arg[i--];
    else
        res->stm->dataHigh[1] = '0';
}

void Parser::parseOperand(ParserResult *res, std::string arg)
{
    if (regex_match(arg, re_immediate)) // IMMEDIATE
    {
        res->size = 5;
        res->stm->addrMode[0] = AddrType::IMMEDIATE;
        res->stm->addrMode[1] = '0';
        arg = regex_replace(arg, std::regex("\\$"), "");
        if (regex_match(arg, std::regex("^\\d+.*"))) // number
        {
            parseLiteral(res, arg);
        }
        else
        { // symbol
            res->stm->reloc_type = RelocType::ABSOLUTE;
            res->symbol = arg;
            res->stm->setDataX();
        }
    }
    else if (regex_match(arg, re_memdir)) // MEM-PC
    {
        res->size = 5;
        res->stm->addrMode[0] = AddrType::MEM;
        res->stm->addrMode[1] = '0';
        res->stm->reloc_type = RelocType::RELATIVE;
        res->symbol = std::regex_replace(arg, std::regex("%"), "");
        res->stm->setDataX();
    }
    else if (regex_match(arg, re_regdir)) // REGDIR
    {
        res->size = 3;
        res->stm->addrMode[0] = AddrType::REGDIR;
        res->stm->addrMode[1] = '0';
        res->stm->regsDescr[0] = parseRegister(arg, res->line);
        if (!res->stm->regsDescr[1])
            res->stm->regsDescr[1] = empty_slot;
    }
    else if (regex_match(arg, re_regind)) // REGIND
    {
        arg = std::regex_replace(arg, std::regex("(\\[)|(\\s+)|(\\])"), "");
        std::stringstream arg_stream(arg);
        std::string tmp;
        getline(arg_stream, tmp, '+');
        res->stm->regsDescr[0] = parseRegister(tmp, res->line);
        if (!res->stm->regsDescr[1])
            res->stm->regsDescr[1] = empty_slot;
        tmp = "";
        getline(arg_stream, tmp);
        if (tmp.empty()) // REGIND
        {
            res->size = 3;
            res->stm->addrMode[0] = AddrType::REGIND;
            res->stm->addrMode[1] = '0';
        }
        else // REGINDPOM
        {
            res->size = 5;
            res->stm->addrMode[0] = AddrType::REGINDPOM;
            res->stm->addrMode[1] = '0';                 // no update
            if (regex_match(tmp, std::regex("^\\d+.*"))) // number
            {
                parseLiteral(res, tmp);
            }
            else
            { // symbol
                res->stm->reloc_type = RelocType::ABSOLUTE;
                res->symbol = tmp;
                res->stm->setDataX();
            }
        }
    }
    else // MEM-ABS
    {
        res->size = 5;
        res->stm->addrMode[0] = AddrType::MEM;
        res->stm->addrMode[1] = '0';
        if (regex_match(arg, std::regex("^\\d+.*"))) // number
        {
            parseLiteral(res, arg);
        }
        else
        { // symbol
            res->stm->reloc_type = RelocType::ABSOLUTE;
            res->symbol = arg;
            res->stm->setDataX();
        }
    }
}

void Parser::parseJumpOperand(ParserResult *res, std::string arg)
{
    if (regex_match(arg, re_jmp_regdir))
    {
        arg = std::regex_replace(arg, std::regex("\\*"), "");
        res->size = 3;
        res->stm->addrMode[0] = AddrType::REGDIR;
        res->stm->addrMode[1] = '0';
        res->stm->regsDescr[0] = parseRegister(arg, res->line);
        if (!res->stm->regsDescr[1])
            res->stm->regsDescr[1] = empty_slot;
    }
    else if (regex_match(arg, re_jmp_regind))
    {
        arg = std::regex_replace(arg, std::regex("(\\*\\[)|(\\s+)|(\\])"), "");
        std::stringstream arg_stream(arg);
        std::string tmp;
        getline(arg_stream, tmp, '+');
        res->stm->regsDescr[0] = parseRegister(tmp, res->line);
        if (!res->stm->regsDescr[1])
            res->stm->regsDescr[1] = empty_slot;
        tmp = "";
        getline(arg_stream, tmp);
        if (tmp.empty()) // REGIND
        {
            res->size = 3;
            res->stm->addrMode[0] = AddrType::REGIND;
            res->stm->addrMode[1] = '0';
        }
        else // REGINDPOM
        {
            res->size = 5;
            res->stm->addrMode[0] = AddrType::REGINDPOM;
            res->stm->addrMode[1] = '0';                 // no update
            if (regex_match(tmp, std::regex("^\\d+.*"))) // number
            {
                parseLiteral(res, tmp);
            }
            else
            { // symbol
                res->stm->reloc_type = RelocType::ABSOLUTE;
                res->symbol = tmp;
                res->stm->setDataX();
            }
        }
    }
    else if (regex_match(arg, re_jmp_memdir_abs))
    {
        arg = std::regex_replace(arg, std::regex("\\*"), "");
        res->size = 5;
        res->stm->addrMode[0] = AddrType::MEM;
        res->stm->addrMode[1] = '0';
        if (regex_match(arg, std::regex("^\\d+.*"))) // number
        {
            parseLiteral(res, arg);
        }
        else
        { // symbol
            res->stm->reloc_type = RelocType::ABSOLUTE;
            res->symbol = arg;
            res->stm->setDataX();
        }
    }
    else if (regex_match(arg, re_jmp_memdir_pc))
    {
        res->size = 5;
        res->stm->addrMode[0] = AddrType::IMMEDIATE; /**/
        res->stm->addrMode[1] = '0';
        res->stm->reloc_type = RelocType::RELATIVE;
        res->symbol = std::regex_replace(arg, std::regex("%"), "");
        res->stm->setDataX();
    }
    else
    { // immediate
        res->size = 5;
        res->stm->addrMode[0] = AddrType::IMMEDIATE;
        res->stm->addrMode[1] = '0';
        if (regex_match(arg, std::regex("^\\d+.*"))) // number
        {
            parseLiteral(res, arg);
        }
        else
        { // symbol
            res->stm->reloc_type = RelocType::ABSOLUTE;
            res->symbol = arg;
            res->stm->setDataX();
        }
    }
}

int Parser::parseInstruction(struct InstrEntry entry, struct ParserResult *res, std::string &token)
{
    StatementParts *p = new StatementParts();
    p->intrDescr[1] = entry.instrDescr[0];
    p->intrDescr[0] = entry.instrDescr[1];
    token = std::regex_replace(token, std::regex("(\\s+)|(#.*)"), "");
    res->stm = p;
    std::vector<std::string> args;
    switch (entry.type)
    {
    case InstrType::ARITHMETIC:
        res->size = 2;
        parseInstructionArguments(args, token);
        p->regsDescr[1] = parseRegister(args[0], res->line);
        p->regsDescr[0] = parseRegister(args[1], res->line);
        break;
    case InstrType::SYSCALL:
        res->size = 2;
        parseInstructionArguments(args, token);
        p->regsDescr[1] = parseRegister(args[0], res->line);
        p->regsDescr[0] = empty_slot;
        break;
    case InstrType::RETURN:
        res->size = 1;
        break;
    case InstrType::LDST:
        parseInstructionArguments(args, token);
        p->regsDescr[1] = parseRegister(args[0], res->line);
        p->regsDescr[0] = empty_slot;
        parseOperand(res, args[1]);
        break;
    case InstrType::CALL:
        res->stm->regsDescr[1] = empty_slot;
        res->stm->regsDescr[0] = empty_slot;
        parseOperand(res, token);
        break;
    case InstrType::JUMP:
        res->stm->regsDescr[1] = empty_slot;
        res->stm->regsDescr[0] = empty_slot;
        parseJumpOperand(res, token);
        break;
    default:
        return 1;
    }
    return 0;
}

ParserResult *Parser::parse(std::string line, bool checkAfterLabel)
{
    if (checkAfterLabel)
    {
        line = std::regex_replace(line, std::regex(labelDetected), "");
        if (regex_match(line, std::regex(".*:.*"))) {
            std::cout << "Error in line: " << line << "; multiple labels" << std::endl;
            exit(-1);
        }
        return parse(std::regex_replace(line, std::regex(".*\\:"), ""));
    }

    ParserResult *res = new ParserResult();
    res->line = line;
    std::stringstream line_stream(std::regex_replace(trim(line), std::regex("\\s{2,}"), " "));

    std::vector<std::string> tokens;
    std::string token;

    // while (getline(line_stream, token, ' ')){
    //     tokens.push_back(token);
    // }

    getline(line_stream, token, ' ');
    tokens.push_back(token);

    getline(line_stream, token);
    tokens.push_back(token);

    //std::cout << tokens[0] << std::endl;

    if (regex_match(tokens[0], re_comment))
    { // CHECK IF COMMENT
        // std::cout << "Comment" << std::endl;
        res->type = ParseType::COMMENT;
    }
    else if (regex_match(tokens[0], re_label))
    { // CHECK IF LABEL
        // std::cout << "Label" << std::endl;
        res->type = ParseType::LABEL;
        labelDetected = tokens[0];
        res->symbol = std::regex_replace(tokens[0], std::regex(":"), "");
    }
    else if (regex_match(tokens[0], re_directive))
    { // CHECK IF DIRECTIVE
        // std::cout << "Directive" << std::endl;
        res->type = ParseType::DIRECTIVE;
        DirectiveParts *dir = new DirectiveParts();
        dir->type = parseDirective(tokens[0]);
        if (tokens.size() == 1)
        {
            std::cout << "Error in line: " << line << "; not enough arguments" << std::endl;
            delete dir;
            delete res;
            return nullptr;
        }
        parseDirectiveArguments(dir->args, tokens[1]);
        res->dir = dir;
    }
    else if (instructions.find(tokens[0]) != instructions.end())
    { // CHECK IF INSTRUCTION
        // std::cout << "Instruction" << std::endl;
        res->type = ParseType::INSTRUCTION;
        if (tokens.size() == 1)
        {
            std::cout << "Error in line: " << line << "; not enough arguments" << std::endl;
            delete res;
            return nullptr;
        }
        if(parseInstruction(instructions[tokens[0]], res, tokens[1])) {
            std::cout << "Error in line: " << line << "; undefined instruction" << std::endl;
        }
    }
    else if (!tokens[0].compare("push"))
    {
        if (tokens.size() == 1)
        {
            std::cout << "Error in line: " << line << "; not enough arguments" << std::endl;
            delete res;
            return nullptr;
        }
        std::string pushArgs = std::regex_replace(tokens[1], std::regex("(\\s+)|(#.*)"), "") + ",[r6]";
        res->type = ParseType::INSTRUCTION;
        parseInstruction(instructions["str"], res, pushArgs);
        res->stm->addrMode[1] = '1';
    }
    else if (!tokens[0].compare("pop"))
    {
        if (tokens.size() == 1)
        {
            std::cout << "Error in line: " << line << "; not enough arguments" << std::endl;
            delete res;
            return nullptr;
        }
        std::string pushArgs = std::regex_replace(tokens[1], std::regex("(\\s+)|(#.*)"), "") + ",[r6]" + ",[r6]";
        res->type = ParseType::INSTRUCTION;
        parseInstruction(instructions["ldr"], res, pushArgs);
        res->stm->addrMode[1] = '4';
    }
    else // UNDEFINED
    {
        delete res;
        return nullptr;
    }

    return res;
}

int Parser::getLiteralValue(std::string literal, std::string& line)
{
    int x;
    if (std::regex_match(literal, std::regex("^0x.*")))
    {
        std::stringstream stream;
        stream << std::hex << literal;
        stream >> x;
    }
    else
    {
        try
        {
            x = stoi(literal);
        }
        catch (std::invalid_argument)
        {
            std::cout << "Error in line: " << line << "; " << literal << " is invalid" << std::endl;
            exit(-1);
        }
        catch (std::out_of_range)
        {
            std::cout << "Error in line: " << line << "; " << literal << " is out of range" << std::endl;
            exit(-1);
        }
    }
    return x;
}

bool Parser::isSymbol(std::string arg)
{
    return !std::regex_match(arg, std::regex("\\d+.*"));
}

std::string Parser::literalToHex(std::string arg, std::string& line)
{
    std::string result;
    std::regex re_hex("^0x.*");
    if (!std::regex_match(arg, re_hex))
    {
        int num;
        try
        {
            num = stoi(arg);
        }
        catch (std::invalid_argument)
        {
            std::cout << "Error in line: " << line << "; " << arg << " is invalid" << std::endl;
            exit(-1);
        }
        catch (std::out_of_range)
        {
            std::cout << "Error in line: " << line << "; " << arg << " is out of range" << std::endl;
            exit(-1);
        }
        std::stringstream stream;
        stream << std::uppercase << std::hex << num;
        result = stream.str();
    }
    else
    {
        result = std::regex_replace(arg, std::regex("0x"), "");
    }

    return result;
}

std::string Parser::literalToHex(int arg)
{
    std::stringstream stream;
    stream << std::uppercase << std::hex << arg;
    return stream.str();
}

void Parser::writeLiteralToHex(int num, ParserResult *res)
{
    std::stringstream stream;
    stream << std::uppercase << std::hex << num;
    std::string arg = stream.str();

    int i = arg.size() - 1;
    res->stm->dataLow[0] = arg[i--];
    if (i >= 0)
        res->stm->dataLow[1] = arg[i--];
    else
        res->stm->dataLow[1] = '0';
    if (i >= 0)
        res->stm->dataHigh[0] = arg[i--];
    else
        res->stm->dataHigh[0] = '0';
    if (i >= 0)
        res->stm->dataHigh[1] = arg[i--];
    else
        res->stm->dataHigh[1] = '0';
}