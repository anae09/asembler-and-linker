#include <iostream>
#include "assembly.hpp"


int main(int argc, char *argv[])
{
    Assembly as("tests/zad9.s", "tests/zad9.o");
    if (!as.assemble()) 
        std::cout << "Success" << std::endl;
    
    return 0;
}