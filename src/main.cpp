#include <iostream>
#include "assembly.hpp"


int main(int argc, char *argv[])
{
    if (argc != 4) {
        std::cout << "Error: Not enough arguments" << std::endl;
        return -4;
    }
    if (strcmp(argv[1], "-o")) {
        std::cout << "Option: " << argv[1] << " not defined" << std::endl;
        return -4;
    }

    std::string inputFilename(argv[3]);
    std::string outputFilename(argv[2]);

    Assembly as("tests/" + inputFilename, "tests/" + outputFilename);

    if (!as.assemble()) 
        std::cout << "Success" << std::endl;
    
    return 0;
}