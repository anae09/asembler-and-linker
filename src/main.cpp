#include <iostream>
#include "assembly.hpp"


int main(int argc, char *argv[])
{
    Assembly as("tests/test_word.s", "tests/test_word.o");
    if (!as.assemble()) 
        std::cout << "Success" << std::endl;
    
    return 0;
}