#include <iostream>
#include <list>
#include "linker.hpp"

#include <sstream>
#include <string>
#include <regex>

enum outputType {NO_TYPE, LINKABLE, HEX};

typedef struct placement {
	std::string name;
	int address;
} Placement;

Placement parsePlacement(std::string arg) {
	static std::regex place("-place=");
	static std::stringstream stream;
	Placement p;
	stream.clear();
    stream.str(std::string());
	arg = std::regex_replace(arg, place, "");
	// std::cout << arg << std::endl;
	int pos = arg.find('@');
	if (pos < 0) {
		std::cout << "Error: Invalid option " << arg << std::endl; 
	}
	p.name = arg.substr(0, pos);
	// std::cout << p.name << ", " << address << std::endl;
	stream << std::hex << arg.substr(pos + 1);
    stream >> p.address;
	return p;
}

int main(int argc, char* argv[])
{
	std::list<std::string> filenames;
	std::list<Placement> placements;
	std::string outputname = "";
	outputType output_type = outputType::NO_TYPE;
	int i = 1;
	std::string arg;
	while (i < argc) {
		arg = std::string(argv[i++]);
		// std:: cout << arg << std::endl;
		if (!arg.compare("-o")) {
			if (!outputname.empty()) {
				std::cout << "Output file already defined" << std::endl;
				return 0;
			}
			if (i < argc) {
				outputname = "tests/" + std::string(argv[i++]);
				continue;
			} else {
				std::cout << "Error: Missing filename" << std::endl;
				return 0;
			}
		}
		else if (!arg.compare("-linkable")) {
			if (output_type != outputType::NO_TYPE) {
				std::cout << "Error: output type already defined" << std::endl;
				return 0;
			} else {
				output_type = outputType::LINKABLE;
			}
		}
		else if (!arg.compare("-hex")) {
			if (output_type != outputType::NO_TYPE) {
				std::cout << "Error: output type already defined" << std::endl;
				return 0;
			} else {
				output_type = outputType::HEX;
			}
		}
		else if (arg.rfind("-place", 0) == 0) {
			Placement p = parsePlacement(arg);
			//std::cout << p.name << ", " << p.address << std::endl;
			placements.push_back(p);
		} else {
			if (arg.find('-',0) == 0) {
				std::cout << "Error: option " << arg << " not recognized" << std::endl;
				return 0; 
			}
			filenames.push_back("tests/" + arg);
		}
	}

	if (outputname.empty()) {
		std::cout << "Error: output file not defined" << std::endl;
		return 0;
	}

	if (output_type == outputType::NO_TYPE) {
		std::cout << "Error: output type not defined" << std::endl;
		return 0;
	}

	if (filenames.size() == 0) {
		std::cout << "Error: input files not defined" << std::endl;
		return 0;
	}

	
	if (output_type == outputType::HEX) {
		Linker linker(filenames, outputname, false);
		std::list<Placement>::iterator iter;
		for (iter = placements.begin(); iter != placements.end(); iter++) {
			linker.addSection(iter->name, iter->address);
		}
		linker.runHex();
	} else {
		Linker linker(filenames, outputname, true);
		linker.runLinkable();
	}


	// filenames.push_back("zad41.o.bin");
	// filenames.push_back("zad42.o.bin");
	// Linker linker(filenames, "zad4.o", true);
	// filenames.push_back("test_linker1.o.bin");
	// filenames.push_back("test_linker2.o.bin");
	// filenames.push_back("test_linker3.o.bin");
	// Linker linker(filenames, "output.hex");
	//linker.addSection("text", 0);
	//linker.addSection("data", 50);
	// linker.runHex();

	return 0;
}