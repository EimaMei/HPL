#include <iostream>
#include <interpreter.hpp>
#include <functions.hpp>


std::string HCL::colorText(std::string txt, RETURN_OUTPUT type) {
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	int clr;

	switch (type) {
		case OUTPUT_GREEN: clr = FOREGROUND_GREEN; break;
		case OUTPUT_YELLOW: clr = FOREGROUND_RED | FOREGROUND_GREEN; break;
		case OUTPUT_RED: clr = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
		case OUTPUT_PURPLE: clr = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE; break;
		case OUTPUT_BLUE: clr = FOREGROUND_INTENSITY | FOREGROUND_BLUE; break;
		default: clr = type; break;
	}
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, clr);
	std::cout << txt;
	SetConsoleTextAttribute(hConsole, 7);

	#else
	std::string text;

	switch (type) {
		case OUTPUT_GREEN: text += "\x1B[32m"; break;
		case OUTPUT_YELLOW: text += "\x1B[33m"; break;
		case OUTPUT_RED:   text += "\x1B[31m"; break;
		case OUTPUT_PURPLE:   text += "\x1B[35m"; break;
		case OUTPUT_BLUE:    text += "\x1B[1;36m"; break;
	}
	text += txt + "\x1B[0m";

	return text;
	#endif

	return "";
}


void HCL::interpreteFile(std::string file) {
	curFile = file;
	FILE* fp = fopen(curFile.c_str(), "r");

	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char* buf = new char[size];
		while (fgets(buf, size + 1, fp)) {
			buf[strcspn(buf, "\n")] = 0;
			line = buf;
			lineCount++;
			
			if (find("//"))
				line = split(line, "//")[0];

			if (checkIncludes() == 1) continue;
			if (checkStruct() == 1) continue;
			if (checkVariables() == 1) continue;
		}
		std::cout << colorText("Finished interpreting " +std::to_string(lineCount)+ " lines from ", OUTPUT_GREEN) << "'" << colorText(curFile, OUTPUT_YELLOW) << "'" << colorText(" successfully", OUTPUT_GREEN) << std::endl;
	}
	else
		std::cout << HCL::colorText("Error: ", HCL::OUTPUT_RED) << HCL::colorText(curFile, HCL::OUTPUT_RED) << ": No such file or directory" << std::endl;

	fclose(fp);
	resetRuntimeInfo();
}


int HCL::checkIncludes() {
	if (useRegex(line, R"(#include\s+([<|\"].*[>|\"]))")) {
		std::string match = unstringify(matches.str(1));

		// Save the old data, since when we're gonna interprete a new file, it's gonna overwrite the old data but not reinstate it when it's finished.
		// Which means we have to reinstate the old data ourselves.
		std::string oldFile = curFile;
		int oldLineCount = lineCount;
		lineCount = 0; // We reset it so that the lineCount won't be innaccurate when the lines gets interpreted correctly.


		if (find("<") && find(">")) // Core library '#include <libpdx.hcl>'
			interpreteFile("core/" + unstringify(match, true));
		else if (find("\"")) // Some library '#include "../somelib.hcl"'
			interpreteFile(getPathFromFilename(oldFile) + "/" + match);

		curFile = oldFile;
		lineCount = oldLineCount;

	}
	return FOUND_NOTHING;
}


int HCL::checkVariables() {
	// Matches the type, name and value
	if (useRegex(line, R"(\s*([A-Za-z0-9]+)\s+([A-Za-z0-9]+)\s*=?\s*(\".*\"|true|false|\d+)*)")) {
		variable var;

		if (matches.str(1) == "string")
			var.type = STRING;
		else if (matches.str(1) == "int" || matches.str(1) == "bool")
			var.type = INT;
		else if (matches.str(1) == "scope")
			var.type = SCOPE;
		else { // Might be a structure type.
			for (auto s : structures) {
				if (matches.str(1) == s.first) { 
					var.type = CUSTOM;
					var.strType = matches.str(1);
					break;
				}
			}

			if (var.type != CUSTOM) { // No type was found.
				std::string msg = "Error at '" + curFile + ":" + std::to_string(lineCount) + "': type '" + matches.str(1) + "' doesn't exist.";
				throw std::runtime_error(msg);
			}
		}
		
		var.name = matches.str(2);

		if (matches.size() >= 3) // If a value is declared.
			var.value = unstringify(matches.str(3));
		
		if (mode == SAVE_STRUCT) // Save variables inside a struct.
			structures.begin()->second.push_back(var);
		else
			variables.push_back(var);
	}
	return 0;
}


int HCL::checkStruct() {
	// 'struct <name> {' Only matches the <name>
	if (useRegex(line, R"(struct\s+([a-zA-Z]+)\s*\{)")) {
		std::string name = removeSpaces(matches.str(1));
		std::vector<variable> v;
		
		structures.insert(structures.begin(), std::make_pair(name, v));
		mode = SAVE_STRUCT;

		return FOUND_SOMETHING;
	}
	// Find a '}'
	else if (useRegex(line, R"(\s*(\})\s*)") && mode == SAVE_STRUCT) {
		mode = SAVE_NOTHING; // Don't save variables into a struct anymore, save them as general variables.

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


void HCL::debugMode() {
	std::cout << colorText("============ DEBUG INFORMATION ============\n", HCL::OUTPUT_PURPLE);

	std::cout << "Variables:\n";
	for (auto v : variables) { 
		if (v.type == STRING) {
			std::cout << "            " << colorText("string ", HCL::OUTPUT_PURPLE) << v.name;
			if (!v.value.empty()) std::cout << " = " << colorText("\"" + v.value + "\"", HCL::OUTPUT_BLUE);
		}
		else if (v.type == INT) {
			std::cout << "            " << colorText("int ", HCL::OUTPUT_PURPLE) << v.name;
			if (!v.value.empty()) std::cout << " = " << colorText(v.value, HCL::OUTPUT_BLUE);
		}
		else if (v.type == SCOPE) {
			std::cout << "            " << colorText("scope ", HCL::OUTPUT_BLUE) << v.name;
			if (!v.value.empty()) std::cout << " = " << colorText(v.value, HCL::OUTPUT_BLUE);
		}
		else std::cout << "            " << v.strType << " " << v.name;
		std::cout << std::endl;
	}
	std::cout << "Structures:\n";
	for (auto s : structures) {
		std::cout << "      " << colorText("struct", HCL::OUTPUT_PURPLE) << " " << colorText(s.first, HCL::OUTPUT_YELLOW) << colorText(" {\n", HCL::OUTPUT_GREEN);
		for (auto v : s.second) {
			if (v.type == STRING) {
				std::cout << "            " << colorText("string ", HCL::OUTPUT_PURPLE) << v.name;
				if (!v.value.empty()) std::cout << " = " << colorText("\"" + v.value + "\"", HCL::OUTPUT_BLUE);
			}
			else if (v.type == INT) {
				std::cout << "            " << colorText("int ", HCL::OUTPUT_PURPLE) << v.name;
				if (!v.value.empty()) std::cout << " = " << colorText(v.value, HCL::OUTPUT_BLUE);
			}
			else if (v.type == SCOPE) {
				std::cout << "            " << colorText("scope ", HCL::OUTPUT_BLUE) << v.name;
				if (!v.value.empty()) std::cout << " = " << colorText(v.value, HCL::OUTPUT_BLUE);
			}
			else std::cout << "            " << v.strType << " " << v.name;
			std::cout << std::endl;
		}
		std::cout << colorText("      }\n", HCL::OUTPUT_GREEN);
	}
}


bool HCL::find(std::string str) {
	return (line.find(str) != std::string::npos);
}


void HCL::resetRuntimeInfo() {
	curFile.clear();
	line.clear();
	lineCount = 0;
	mode = SAVE_NOTHING;
	matches = {};
}


namespace HCL {
	// Inteperter configs.
	bool debug;

	// Interpreter rules runtime.
	std::string curFile;
	std::string line;
	int lineCount = 0;
	int mode = 0;
	std::smatch matches;

	// Defnitions that are saved in memory.
	std::vector<variable> variables;
	std::vector<std::pair<variable, std::vector<variable>>> functions;
	std::vector<std::pair<std::string, std::vector<variable>>> structures;
}