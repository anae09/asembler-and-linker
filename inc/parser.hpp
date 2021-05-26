#pragma once 

#include <iostream>
#include <regex>
#include <unordered_map>
#include "parser_result.hpp"
#include "instr_entry.hpp"
#include "addrtype.hpp"


class Parser {
private:
    Parser();
    std::unordered_map<std::string,struct InstrEntry> instructions;
    static Parser* parser;
    
    DirectiveType parseDirective(std::string dir);
    void parseDirectiveArguments(std::list <std::string>& args, std::string token);
    void parseInstructionArguments(std::vector <std::string>& args, std::string token);

    char parseRegister(std::string& reg, std::string& line);

    std::string trim(const std::string &s);
    std::string ltrim(const std::string &s);
    std::string rtrim(const std::string &s);
    
    std::string comment_pattern = "^#.*";
    std::string label_pattern = ".*\\:$";
    std::string directive_pattern = "^\\..*";

    std::regex re_comment;
    std::regex re_label;
    std::regex re_directive;

    std::string immediate_pattern = "^\\$.*";
    std::string memdir_pattern = "^%.*";
    std::string regdir_pattern = "^r.*";
    std::string regind_pattern = "^\\[.*";

    std::regex re_immediate;
    std::regex re_memdir;
    std::regex re_regdir;
    std::regex re_regind;

    std::string jmp_regdir_pattern = "^\\*r.*";
    std::string jmp_regind_pattern = "^\\*\\[.*";
    std::string jmp_memdir_abs_pattern = "^\\*.*";
    std::string jmp_memdir_pc_pattern = "^%.*";

    std::regex re_jmp_regdir;
    std::regex re_jmp_regind;
    std::regex re_jmp_memdir_abs;
    std::regex re_jmp_memdir_pc;


    const char empty_slot = 'F';

    void init_instructions_map();
    int parseInstruction(struct InstrEntry entry, struct ParserResult* res, std::string& args);
    void parseOperand(ParserResult* res, std::string arg);
    void parseJumpOperand(ParserResult* res, std::string arg);
    void parseLiteral(ParserResult* res, std::string arg);

    std::string labelDetected;
public:
    Parser(Parser &other) = delete;
    void operator=(const Parser &) = delete;
    static Parser* getInstance();
    ParserResult* parse(std::string line, bool checkAfterLabel=false);
    static int getLiteralValue(std::string literal, std::string& line);
    static bool isSymbol(std::string arg);
    static std::string literalToHex(std::string arg, std::string& line);
    static std::string literalToHex(int arg);
    static void writeLiteralToHex(int arg, ParserResult* res);
};