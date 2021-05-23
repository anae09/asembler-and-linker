#include <iostream>
#include "assembly.hpp"


int main(int argc, char *argv[])
{
    Assembly as("tests/test1.s", "tests/test1.o");
    if (!as.assemble()) 
        std::cout << "Success" << std::endl;
    
    return 0;
}