#include <iostream>
#include <algorithm>
#include <interpreter.hpp>
#include <functions.hpp>

int main(int argc, char** argv) {
	std::string filename;

	if (argc <= 1) { std::cout << HCL::colorText("Error",  HCL::OUTPUT_RED) << ": No input files provided" << std::endl; return -1;  }
	else {
		for (int i = 1; i < argc; i++) {
			std::string arg = (std::string)argv[i];
			if (arg == "-debug" || arg == "-g") { 
				HCL::debug = true; 
				continue; 
			}
			else 
				filename = arg; 
		}
	}
	HCL::interpreteFile(filename);

	if (HCL::debug)
		HCL::debugMode();

	return 0;
}