#include <iostream>
#include <list>
#include "linker.hpp"

#include <sstream>
#include <string>

int main()
{
	std::list<std::string> filenames;
	// filenames.push_back("zad41.o.bin");
	// filenames.push_back("zad42.o.bin");
	// Linker linker(filenames, "zad4.o", true);
	filenames.push_back("test_linker1.o.bin");
	filenames.push_back("test_linker2.o.bin");
	filenames.push_back("test_linker3.o.bin");
	Linker linker(filenames, "output.hex");
	//linker.addSection("text", 0);
	//linker.addSection("data", 50);
	linker.runHex();

	return 0;
}