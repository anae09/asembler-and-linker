#include <iostream>
#include <list>
#include "linker.hpp"

int main()
{
	std::list<std::string> filenames;
	filenames.push_back("test_linker1.o.bin");
	filenames.push_back("test_linker2.o.bin");
	filenames.push_back("test_linker3.o.bin");
	Linker linker(filenames);
	linker.addSection("text", 0);
	linker.addSection("data", 50);
	linker.run();
	return 0;
}