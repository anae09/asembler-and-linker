#include <iostream>
#include "assembly.hpp"


int main(int argc, char *argv[])
{
    Assembly as("tests/test_jump.s", "tests/test_jump.o");
    if (!as.assemble()) 
        std::cout << "Success" << std::endl;
    
    return 0;
}