#pragma once

#include <iostream>
#include "parsetype.hpp"
#include "statement_parts.hpp"
#include "direct_parts.hpp"

struct ParserResult {
    struct StatementParts* stm = nullptr;
    struct DirectiveParts* dir = nullptr;
    std::string symbol;
    ParseType type;
    int size = 0;
    ~ParserResult() {
        if (stm) delete stm;
        if (dir) delete dir;
    }
};