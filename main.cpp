#include <interpreter.hpp>
#include <helper.hpp>
#include <cli.hpp>

#include <iostream>


int main(int argc, char** argv) {
	std::string filename;

	for (int i = 1; i < argc; i++) {
		std::string arg = (std::string)argv[i];
		bool output = false;

		checkArgs({"help", "h"}, arg, HPL::arg.help, output);
		checkArgs({"debug", "g"}, arg, HPL::arg.debugAll, output); // Debug all
		checkArgs({"strict", "s"}, arg, HPL::arg.strict, output);
		checkArgs({"log", "l"}, arg, HPL::arg.debugLog, output);
		checkArgs({"breakpoint", "b"}, arg, HPL::arg.breakpoint, output);

		if (HPL::arg.breakpoint && find(arg, ":") && HPL::arg.breakpointValues.first.empty()) {
			std::vector<std::string> input = split(arg, ":"); // [0] - file, [1] - line.

			HPL::arg.breakpointValues = std::make_pair(input[0], std::stoi(input[1]));
		}
		else if (!output)
			filename = arg;
	}

	if (HPL::arg.help) {
		printHelp();
		return 0;
	}
	else if (filename.empty()) {
		std::cout << HPL::colorText("Error",  HPL::OUTPUT_RED) << ": No input files were provided" << std::endl;
		return -1;
	}


	HPL::interpreteFile(filename);

	if (HPL::arg.debugAll)
		HPL::debugMode();

	return 0;
}