/*
* Copyright (C) 2021-2022 EimaMei/Sacode
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
*
*/
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
		checkArgs({"dumpJson", "d"}, arg, HPL::arg.dumpJson, output);
		checkArgs({"compile", "c"}, arg, HPL::arg.objectify, output);

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

	HPL::timer t = startTimer();
	HPL::interpreteFile(filename);
	updateTimer(t);
	std::cout << t << std::endl;

    //FILE * file = fopen("output.hplo", "wb");
//
	//for (auto const& var : HPL::variables) {
	//	HPL::object obj = {1, 1, var};
	//	fwrite(&obj, sizeof(HPL::object), 1, file);
	//}
    //fclose(file);

	//file = fopen("output.bin", "rb");
    //if (file != NULL) {
    //    fread(&obj, sizeof(HPL::object), 1, file);
    //    fclose(file);
    //}

	if (HPL::arg.dumpJson) {
		dumpJson();
		return 0;
	}

	if (HPL::arg.debugAll)
		HPL::debugMode();

	return 0;
}