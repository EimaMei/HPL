#include <interpreter.hpp>
#include <helper.hpp>
#include <cli.hpp>

#include <iostream>


int main(int argc, char** argv) {
	std::string filename;

	for (int i = 1; i < argc; i++) {
		std::string arg = (std::string)argv[i];
		bool output = false;

		checkArgs({"help", "h"}, arg, HCL::arg.help, output);
		checkArgs({"debug", "g"}, arg, HCL::arg.debugAll, output); // Debug all
		checkArgs({"strict", "s"}, arg, HCL::arg.strict, output);
		checkArgs({"log", "l"}, arg, HCL::arg.debugLog, output);
		checkArgs({"breakpoint", "b"}, arg, HCL::arg.breakpoint, output);

		if (HCL::arg.breakpoint && find(arg, ":") && HCL::arg.breakpointValues.first.empty()) {
			std::vector<std::string> input = split(arg, ":"); // [0] - file, [1] - line.

			HCL::arg.breakpointValues = std::make_pair(input[0], std::stoi(input[1]));
		}
		else if (!output)
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