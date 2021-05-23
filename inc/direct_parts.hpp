#pragma once

#include <iostream>
#include <list>
#include "directype.hpp"

struct DirectiveParts {
    DirectiveType type;
    std::list <std::string> args;
};