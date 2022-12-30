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
#include <cli.hpp>
#include <helper.hpp>
#include <interpreter.hpp>

void checkArg(std::string arg, std::string input, bool& config, bool& res) {
	if (input == ("-" + arg)) {
		config = true;
		res = true;
	}
	else if (input == ("-no" + arg)) {
		config = false;
		res = true;
	}
}


void checkArgs(std::vector<std::string> args, std::string input, bool& config, bool& res) {
	for (auto& a : args) {
		checkArg(a, input, config, res);
		if (res)
			break;
	}
}


void printHelp() {
	std::cout << HPL::colorText(VERSION, HPL::OUTPUT_GREEN) << std::endl
			  << AUTHORS << std::endl
			  << COMPILER << " on " << OS << std::endl
			  << HPL::colorText("ARGS:", HPL::OUTPUT_YELLOW) << "\n\t"
			  		<< HPL::colorText("<FILE>", HPL::OUTPUT_GREEN)                                                             << "								Selected file to be interpreted." << "\n"
			  << HPL::colorText("OPTIONS:", HPL::OUTPUT_YELLOW) << "\n\t"
			  		<< HPL::colorText("-help", HPL::OUTPUT_GREEN) << ", " << HPL::colorText("-h", HPL::OUTPUT_GREEN) << "                    					Prints the available CLI options as well as the the version, authors, compiler and OS of the HPL executable." << "\n\t"
			  		<< HPL::colorText("-debug", HPL::OUTPUT_GREEN) << ", " << HPL::colorText("-g", HPL::OUTPUT_GREEN) << "                   					Enables all debug procedures (logging and printing debug information)." << "\n\t"
					<< HPL::colorText("-log", HPL::OUTPUT_GREEN) << ", " << HPL::colorText("-l", HPL::OUTPUT_GREEN) << "                     					Logs and prints every noteworthy event that the interpreter has got." << "\n\t"
			  		<< HPL::colorText("-strict", HPL::OUTPUT_GREEN) << ", " << HPL::colorText("-s", HPL::OUTPUT_GREEN) << "                  					Enables a strict mode, where you have a limited amount of available features to make less confusing code/massive mistakes (Barely implemented)." << "\n\t"
					<< HPL::colorText("-breakpoint", HPL::OUTPUT_GREEN) << ", " << HPL::colorText("-b <FILE>:<LINE>", HPL::OUTPUT_GREEN) << "					Sets a breakpoint at a specific file and line where if the interpreter reaches it, it stops interpreting everything." << "\n\t"
					<< HPL::colorText("-dumpJson", HPL::OUTPUT_GREEN) << ", " << HPL::colorText("-d", HPL::OUTPUT_GREEN) << "	                 				Dumps the entire project's information (mod name, version, variables, functions etc.) into a JSON format. Used for creating other tools with HPL.";
}


void dumpJson() {
	auto mod = getVars(getVarFromName("HPL_currentMod")->value);
	int i = 0;

	std::string buffer =
	"{\n\t"
		"\"mod\" : {\n\t\t"
			"\"name\" : \"" + xToStr(mod[0].value) + "\"," + "\n\t\t"
			"\"version\" \"" + xToStr(mod[1].value) + "\"," + "\n\t\t"
			"\"path\" : \"" + xToStr(mod[3].value) + "\"," + "\n\t"
		"},\n\t"

		"\"variables\" : {\n\t\t";
			for (auto const& var : HPL::variables) {
			buffer +=
			"\"" + var.name + "\" : {\n\t\t\t"
				"\"type\" : \"" + var.type + "\",\n\t\t\t"
				"\"value\" : \"" + xToStr(var.value) + "\",\n\t\t" +
			"}";

			if (i + 1 < HPL::variables.size())
				buffer += ",";
			buffer += "\n\t\t";

			i++;
			} buffer +=
		"\n\t}\n"
	"}";

	std::printf("%s", buffer.c_str());
}