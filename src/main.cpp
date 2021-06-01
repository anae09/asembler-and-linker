#include <iostream>
#include <list>
#include "linker.hpp"

#include <sstream>
#include <string>

int main()
{
	std::list<std::string> filenames;
	filenames.push_back("test_linker1.o.bin");
	filenames.push_back("test_linker2.o.bin");
	filenames.push_back("test_linker3.o.bin");
	Linker linker(filenames, "output.o", true);
	//linker.addSection("text", 0);
	//linker.addSection("data", 50);
	linker.runLinkable();

	return 0;
}