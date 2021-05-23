#include <iostream>
#include "assembly.hpp"


int main(int argc, char *argv[])
{
    Assembly as("tests/test_alu.s", "tests/test_alu.o");
    if (!as.assemble()) 
        std::cout << "Success" << std::endl;
    
    return 0;
}