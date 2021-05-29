#include <iostream>
#include <list>
#include "linker.hpp"

int main()
{
	std::list<std::string> filenames;
	filenames.push_back("test1.o.bin");
	//filenames.push_back("test2.o.bin");
	Linker linker(filenames);
	return 0;
}