/*
* Copyright (C) 2021-2022 Eima
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
#include <core.hpp>
#include <helper.hpp>
#include <iostream>


std::string HCL::colorText(std::string txt, RETURN_OUTPUT type, bool light/* = false*/) {
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
		case OUTPUT_NOTHING:  text = "\x1B[0m";  break;
		case OUTPUT_BLACK:    text = "\x1B[30m"; break;
		case OUTPUT_RED:      text = "\x1B[31m"; break;
		case OUTPUT_GREEN:    text = "\x1B[32m"; break;
		case OUTPUT_YELLOW:   text = "\x1B[33m"; break;
		case OUTPUT_BLUE:     text = "\x1B[34m"; break;
		case OUTPUT_PURPLE:   text = "\x1B[35m"; break;
		case OUTPUT_CYAN:     text = "\x1B[36m"; break;
		case OUTPUT_GRAY:     text = "\x1B[37m"; break;
	}
	if (light) {
		std::replace(text.begin(), text.begin() + 2, '3', '9');
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

			interpreteLine(line);
		}
		std::cout << colorText("Finished interpreting " +std::to_string(lineCount)+ " lines from ", OUTPUT_GREEN) << "'" << colorText(curFile, OUTPUT_YELLOW) << "'" << colorText(" successfully", OUTPUT_GREEN) << std::endl;
	}
	else
		std::cout << HCL::colorText("Error: ", HCL::OUTPUT_RED) << HCL::colorText(curFile, HCL::OUTPUT_RED) << ": No such file or directory" << std::endl;

	fclose(fp);
	resetRuntimeInfo();
}


int HCL::interpreteLine(std::string str) {
	if (find(str, "//")) // Ignore comments
		str = split(str, "//")[0];

	line = str;

	if (checkIncludes() == FOUND_SOMETHING) return FOUND_SOMETHING;
	if (checkFunctions() == FOUND_SOMETHING) return FOUND_SOMETHING;
	if (checkStruct() == FOUND_SOMETHING) return FOUND_SOMETHING;
	if (checkVariables() == FOUND_SOMETHING) return FOUND_SOMETHING;

	return FOUND_NOTHING;
}


int HCL::checkIncludes() {
	if (useRegex(line, R"(#include\s+([<|\"].*[>|\"]))")) {
		std::string match = unstringify(matches.str(1));

		// Save the old data, since when we're gonna interprete a new file, it's gonna overwrite the old data but not reinstate it when it's finished.
		// Which means we have to reinstate the old data ourselves.
		std::string oldFile = curFile;
		int oldLineCount = lineCount;
		lineCount = 0; // We reset it so that the lineCount won't be innaccurate when the lines gets interpreted correctly.


		if (find(line, "<") && find(line, ">")) // Core library '#include <libpdx.hcl>'
			interpreteFile("core/" + unstringify(match, true));
		else if (find(line, "\"")) // Some library '#include "../somelib.hcl"'
			interpreteFile(getPathFromFilename(oldFile) + "/" + match);

		curFile = oldFile;
		lineCount = oldLineCount;

	}
	return FOUND_NOTHING;
}


int HCL::checkFunctions() {
	// Match <return type> <name>(<params>) {
	if (useRegex(line, R"(^\s*([A-Za-z0-9\.]+)\s+([A-Za-z0-9]+)\((.*)\)\s*\{$)")) {
		std::string param = matches.str(3);
		function func = {matches.str(1), matches.str(2)};
		std::vector<std::string> params = split(param, ",", '\"');

		for (auto v : params) {
			if (useRegex(v, R"(\s*([A-Za-z0-9\.]+)\s+([A-Za-z0-9]+)?\s*=?\s*(\".*\"|\{.*\}|[^\s*]*)\s*)")) {
				variable var = {.type = matches.str(1), .name = matches.str(2), .value = {unstringify(matches.str(3))}};
				func.params.push_back(var);
				if (var.value[0].empty()) func.minParamCount++;
			}
		}

		functions.push_back(func);
		mode = SAVE_FUNC;

		return FOUND_SOMETHING;
	}
	else if (useRegex(line, R"(\s*(\})\s*)") && mode == SAVE_FUNC) {
		mode = SAVE_NOTHING;

		return FOUND_SOMETHING;
	}
	else if (mode == SAVE_FUNC) {
		functions[functions.size() - 1].code.push_back(line);

		return FOUND_SOMETHING;
	}
	else if (useRegex(line, R"(^\s*([A-Za-z0-9\.]+)\((.*)\)\s*$)")) {
		return coreFunctionCheck(matches.str(1), matches.str(2));
	}

	return FOUND_NOTHING;
}


int HCL::checkVariables() {
	// Matches the name and value
	if (useRegex(line, R"(^\s*([A-Za-z0-9^.]+)\s*=\s*(\".*\"|\{.*\}|[^\s*]*)\s*$)")) { // Edit a pre-existing variable
		variable info = {.name = matches.str(1), .value = {unstringify(matches.str(2))}}; variable structInfo;
		variable* existingVar = getVarFromName(info.name, &structInfo);


		if (existingVar != NULL) {
			if (structInfo.value.empty()) // Edit a regular variable
				existingVar->value = info.value;
			else {// Edit a struct member
				int index = std::stoi(structInfo.extra[0]);
				existingVar->value[index] = info.value[0];
			}
		}
	}
	// Matches the type, name and value
	else if (useRegex(line, R"(\s*([A-Za-z0-9\.]+)\s+([A-Za-z0-9]+)?\s*=?\s*(\".*\"|\{.*\}|[^\s*]*)\s*)")) { // Declaring a new variable.
		std::string ogValue = matches.str(3);
		variable var = {.type = matches.str(1), .name = matches.str(2), .value = {unstringify(ogValue)}};
		structure s;

		if (typeIsValid(var.type, &s)) { // Is type cored or a structure.
			variable structVar;
			variable* newVar = getVarFromName(var.value[0], &structVar);

			if (newVar != NULL) { // Copying over an existing variable.
				// If 'getVarFromName' finds a variable from a struct, `structVar.extra` becomes the index of where the member is in the struct.
				// However, due to `structVar.value` being a std::string, we have to convert it to an integer.
				int index = std::stoi(structVar.extra[0]);
				var.value = {structVar.value[index]}; 
			}
			else if (!s.name.empty()) {
				if (var.type == s.name) {
					var.value.clear(); // Clear everything in the vector so that no vector shenanigans would happen.
					
					if (var.value[0].empty()) { // Nothing is set, meaning it's just the struct's default arguments.
						for (int i = 0; i < s.value.size(); i++) {
							auto data = s.value[i];
							var.value.push_back(data.value[0]);
							var.extra.push_back(data.type);
						}
					}
					else { // Oh god oh fuck it's a custom list.
						// We don't split the string by the comma if the comma is inside double quotes
						// Meaning "This, yes this, exact test string" won't be split. We also remove the curly brackets before splitting.
						std::vector<std::string> valueList = split(unstringify(var.value[0], true), ",", '\"');

						if (valueList.size() > s.value.size()) {
							throwError("Too many values are provided when declaring the variable '%s' (you provided '%i' arguments when struct type '%s' has only '%i' members).", var.name.c_str(), valueList.size(), var.type.c_str(), s.value.size());
						}

						// The first unstringify is used to remove any unneeded spaces.
						// Then the second removes the double quotes if it's a string.
						for (int i = 0; i < s.value.size(); i++) {
							if (i < s.value.size() && i < valueList.size()) {
								var.value.push_back(unstringify(unstringify(valueList[i], false, ' ')));
								var.extra.push_back(s.value[i].type);
							}
							else { // Looks like the user didn't provide the entire argument list. That's fine, though we must check for any default options.
								if (!s.value[i].value[0].empty()) {
									var.value.push_back(s.value[i].value[0]);
									var.extra.push_back(s.value[i].type);
								}
								else if (strict)
									throwError("Too few values are provided to fully initialize a struct (you provided '%i' arguments when struct type '%s' has '%i' members).", valueList.size(), var.type.c_str(), s.value.size());
								else
									break;
							}
						}
					}
				}
			}
			else if (!var.value[0].empty() && newVar == NULL && !isInt(var.value[0]) && (var.value[0] != "true" && var.value[0] != "false") && !(find(ogValue, "\"") && var.type == "string")) {
				// This checks for if the user is trying to copy over a variable that doesn't exist. All of these checks check if it isn't just some core type so that it wouldn't output a false-negative.
				throwError("You cannot copy over a variable that doesn't exist (variable '%s' does not exist).", var.value[0].c_str());
			}
		}
		else {
			throwError("Type '%s' doesn't exist.", var.type.c_str());
		}

		if (mode == SAVE_STRUCT) // Save variables inside a struct.
			structures.begin()->value.push_back(var);
		else
			variables.push_back(var);
	}
	return FOUND_NOTHING;
}


int HCL::checkStruct() {
	// 'struct <name> {' Only matches the <name>
	if (useRegex(line, R"(struct\s+([a-zA-Z]+)\s*\{)")) {
		std::string name = removeSpaces(matches.str(1));

		structures.insert(structures.begin(), {name});
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
	
	std::cout << colorText("Variables:\n", OUTPUT_CYAN, true);
	debugPrintVar(variables);
	std::cout << colorText("\nStructures:\n", OUTPUT_CYAN, true);
	debugPrintStruct(structures);
	std::cout << colorText("Functions:\n", OUTPUT_CYAN, true);
	debugPrintFunc(functions);
}


void HCL::debugPrintVar(std::vector<variable> vars, std::string tabs/* = "	"*/, std::string end/* = "\n"*/) {
	for (int index = 0; index < vars.size(); index++) {  // Regular variables
		auto v = vars[index];

		RETURN_OUTPUT clr = OUTPUT_PURPLE;
	
		if (v.type == "scope")
			clr = OUTPUT_BLUE;
		else if (!coreTyped(v.type))
			clr = OUTPUT_NOTHING;
			
		std::cout << tabs << colorText(v.type, clr) << " " << v.name;

		if (!v.value[0].empty()) std::cout << " = ";
		if (v.value.size() > 1) std::cout << "{";

		std::string vtype = v.type;
		for (int i = 0; i < v.value.size(); i++) {
			auto value = v.value[i];
			if (i < v.extra.size()) vtype = v.extra[i];

			if (vtype == "string" && (!value.empty() || v.value.size() > 1)) 
				std::cout << colorText("\"" + value + "\"", HCL::OUTPUT_BLUE, true);
			else if (!value.empty() || v.value.size() > 1)
				std::cout << colorText(value, HCL::OUTPUT_BLUE, true);

			if (v.value.size() > 1 && ((i + 1) < v.value.size()))
				std::cout << ", ";
		}

		if (v.value.size() > 1) std::cout << "}";

		if (index + 1 < vars.size()) std::cout << end;
	}
}


void HCL::debugPrintStruct(std::vector<structure> structList, std::string indent/* = "\t"*/) {
	for (auto s : structures) {
		std::cout << indent << colorText("struct", HCL::OUTPUT_PURPLE) << " " << colorText(s.name, HCL::OUTPUT_YELLOW) << colorText(" {", HCL::OUTPUT_GREEN) << std::endl;
		debugPrintVar(s.value, indent + indent);
		std::cout << colorText("\n" + indent + "}\n", HCL::OUTPUT_GREEN);
	}
}


void HCL::debugPrintFunc(std::vector<function> func, std::string indent/* = "\t"*/) {
	for (auto f : functions) {
		RETURN_OUTPUT clr = OUTPUT_PURPLE;
	
		if (f.type == "scope")
			clr = OUTPUT_BLUE;
		else if (!coreTyped(f.type))
			clr = OUTPUT_RED;

		std::cout << indent << colorText(f.type, clr) << " " << f.name << "(";
		debugPrintVar(f.params, "", ", ");
		std::cout << ") {\n";
		for (auto c : f.code)
			std::cout << c << std::endl;
		std::cout << "}";
	}
}


void HCL::resetRuntimeInfo() {
	curFile.clear();
	line.clear();
	lineCount = 0;
	mode = SAVE_NOTHING;
	matches = {};
}


void HCL::throwError(std::string text, ...) {
	va_list valist;
	va_start(valist, text);
	std::string msg = "Error at '" + curFile + ":" + std::to_string(lineCount) + "': ";

	for (int i = 0; i < text.size(); i++) {
		auto x = text[i];
		int num = 0;

		if (x == '%' && (i + 1) < text.size()) {
			switch (text[i + 1]) {
				case 's':
					msg += va_arg(valist, const char*);
					break;
				case 'i':
					num = va_arg(valist, int);
					msg += std::to_string(num);
					break;

				default:
					break;
			}
			i++;
		}
		else msg += x;
	}
	va_end(valist);
	
	if (debug) debugMode();
	throw std::runtime_error("\x1B[0m" + msg);
}


namespace HCL {
	// Inteperter configs.
	bool debug;
	bool strict;

	// Interpreter rules runtime.
	std::string curFile;
	std::string line;
	int lineCount = 0;
	int mode = 0;
	std::smatch matches;

	// Defnitions that are saved in memory.
	std::vector<variable> variables;
	std::vector<structure> structures;
	std::vector<function> functions;
}