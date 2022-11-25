#include <iostream>
#include <interpreter.hpp>
#include <cli.hpp>


int main(int argc, char** argv) {
	std::string filename;

	for (int i = 1; i < argc; i++) {
		std::string arg = (std::string)argv[i];
		bool output = false;

		checkArgs({"help", "h"}, arg, HCL::arg.help, output);
		checkArgs({"debug", "g"}, arg, HCL::arg.debugAll, output); // Debug all
		checkArgs({"strict", "s"}, arg, HCL::arg.strict, output);
		checkArgs({"log", "l"}, arg, HCL::arg.debugLog, output);

		if (!output)
			filename = arg;
	}

	if (HCL::arg.help) {
		printHelp();
		return 0;
	}
	else if (filename.empty()) {
		std::cout << HCL::colorText("Error",  HCL::OUTPUT_RED) << ": No input files were provided" << std::endl;
		return -1;
	}


	HCL::interpreteFile(filename);

	if (HCL::arg.debugAll)
		HCL::debugMode();

	return 0;
}