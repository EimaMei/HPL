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

#define _CRT_SECURE_NO_WARNINGS

#include <interpreter.hpp>
#include <core.hpp>
#include <helper.hpp>

#include <iostream>
#include <string.h>

std::vector<std::string> alreadyReadFiles;

std::string HCL::colorText(std::string txt, RETURN_OUTPUT type, bool light/* = false*/) {
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	int clr;

	switch (type) {
		case OUTPUT_NOTHING: clr = 7; break;
		case OUTPUT_BLACK: clr = 16; break;
		case OUTPUT_RED: clr = FOREGROUND_RED; break;
		case OUTPUT_GREEN: clr = FOREGROUND_GREEN; break;
		case OUTPUT_YELLOW: clr = FOREGROUND_RED | FOREGROUND_GREEN; break;
		case OUTPUT_BLUE: clr = FOREGROUND_BLUE; break;
		case OUTPUT_PURPLE: clr = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE; break;
		case OUTPUT_CYAN: clr = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
		case OUTPUT_GRAY: clr = 8; break;
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
	bool alreadyRead = false;
	for (auto readFile : alreadyReadFiles) {
		if (readFile == file) {
			alreadyRead = true;
			break;
		}
	}

	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char* buf = new char[size];
		while (fgets(buf, size + 1, fp)) {
			buf[strcspn(buf, "\n")] = 0;
			line = buf;
			lineCount++;

			if (find(line, "#read once") && alreadyRead)
				break; // Since we already read the file, just don't do it.
			else if (find(line, "#read once"))
				continue; // Ignore this line, otherwise an error will appear.

			interpreteLine(line);
		}
		if (!alreadyRead) std::cout << colorText("Finished interpreting " +std::to_string(lineCount)+ " lines from ", OUTPUT_GREEN) << "'" << colorText(curFile, OUTPUT_YELLOW) << "'" << colorText(" successfully", OUTPUT_GREEN) << std::endl;
	}
	else
		std::cout << HCL::colorText("Error: ", HCL::OUTPUT_RED) << HCL::colorText(curFile, HCL::OUTPUT_RED) << ": No such file or directory" << std::endl;

	fclose(fp);
	resetRuntimeInfo();
	alreadyReadFiles.push_back(file);
}


int HCL::interpreteLine(std::string str) {
	if (find(str, "//")) // Ignore comments
		str = split(str, "//", "\"\"")[0];

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
	if (useRegex(line, R"(\s*([^\s]+)\s+([^\s]+)\s*\((.*)\)\s*\{)")) {
		std::string param = matches.str(3);
		function func = {matches.str(1), matches.str(2)};
		std::vector<std::string> params = split(param, ",", "\"\"");

		for (auto v : params) {
			if (useRegex(v, R"(\s*([^\s]+)\s+([^\s]+)?\s*=?\s*(f?\".*\"|\{.*\}|[^\s*]*)\s*)")) {
				variable var = {matches.str(1), matches.str(2), {unstringify(matches.str(3))}};
				func.params.push_back(var);
				if (var.value[0].empty()) func.minParamCount++;
			}
		}
		func.file = HCL::curFile;
		func.startingLine = HCL::lineCount;

		functions.push_back(func);
		mode = SAVE_FUNC;

		return FOUND_SOMETHING;
	}
	else if (useRegex(line, R"(^\s*(\})\s*$)") && mode == SAVE_FUNC) {
		mode = SAVE_NOTHING;

		return FOUND_SOMETHING;
	}
	else if (mode == SAVE_FUNC) {
		functions.back().code.push_back(line);

		return FOUND_SOMETHING;
	}
	else if (useRegex(line, R"(^\s*([^\s\(]+)\((.*)\)\s*$)")) {
		function f; std::string n;
		return executeFunction(matches.str(1), matches.str(2), f, functionOutput, n);
	}
	else if (useRegex(line, R"(\s*return\s+(f?\".*\"|\{.*\}|[^\s]+\(.*\)|[^\s]*)\s*)")) {
		// Core types.
		if (matches.str(1).front() == '\"' && matches.str(1).back() == '\"') {
			std::string output = unstringify(matches.str(1));
			functionOutput = stringToVoid(output);
			functionReturnType = "string";
		}
		else if (isInt(matches.str(1))) {
			if (find(matches.str(1), ".")) {
				functionOutput = floatToVoid(std::stof(matches.str(1)));
				functionReturnType = "float";
			}
			else {
				functionOutput = intToVoid(std::stoi(matches.str(1)));
				functionReturnType = "int";
			}
		}
		else if (matches.str(1) == "true" || matches.str(1) == "false") {
			functionOutput = intToVoid(stringToBool(matches.str(1)));
			functionReturnType = "bool";
		}
		// A struct
		else if (matches.str(1).front() == '{' && matches.str(1).back() == '}') {
			useIterativeRegex(unstringify(matches.str(1), true), R"(([^\,\s]+))");
			for (auto& v : HCL::matches.value) {
				HCL::variable* var = getVarFromName(v);
				if (var != nullptr) {
					v = var->value[0]; // Need to add struct support to this too later.
				}
				else if (!getTypeFromValue(v).empty())
					continue;
				else
					HCL::throwError(true, "Variable '%s' doesn't exist (Cannot set a member to something that doesn't exist)", v.c_str());
			}
			functionOutput = (void*)&HCL::matches.value;
			functionReturnType = "struct"; // We'll deal with this later in the code.
		}

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


int HCL::checkVariables() {
	// Matches the name and value of an existing variable.
	if (useRegex(line, R"(^\s*([^\s]+)\s*[^\-\+\/\*]=\s*(f?\".*\"|\{.*\}|\w*\(.*\)|[\d\s\+\-\*\/\.]+[^\w]*|[^\s]*)\s*.*$)")) { // Edit a pre-existing variable
		std::string ogValue = matches.str(2);
		variable info = {"", matches.str(1), {unstringify(matches.str(2))}}; variable structInfo, possibleStructInfo;
		variable* existingVar = getVarFromName(info.name, &structInfo);
		variable* possibleVar = getVarFromName(info.value[0], &possibleStructInfo);
		std::string res;


		if (existingVar != NULL) {
			// Check if the value is a function. If so, execute the
			// function and get its return, so that it gets assigned
			// to the variable.
			if (useRegex(line, R"(\s*([^\s]+)\((.*)\))")) {
				assignFuncReturnToVar(existingVar, matches.str(1), matches.str(2));
				return FOUND_SOMETHING;
			}
			else if (possibleVar != nullptr) { // Value might be just a variable, if so just copy it over.
				if (possibleStructInfo.value.empty()) // Edit a regular variable
					info.value = possibleVar->value;
				else {// Edit a struct member
					int index = std::stoi(possibleStructInfo.extra[0]);
					info.value[0] = existingVar->value[index];
				}
			}
			else if (!(res = extractMathFromValue(info.value[0], existingVar)).empty()) { // A math expression.
				info.value[0] = res;
			}

			if (structInfo.value.empty()) // Edit a regular variable
				existingVar->value = info.value;
			else {// Edit a struct member
				int index = std::stoi(structInfo.extra[0]);
				existingVar->value[index] = info.value[0];
			}

			getValueFromFstring(ogValue, existingVar->value[0]);
		}
		else
			HCL::throwError(true, "Variable '%s' doesn't exist (Can't edit a variable that doesn't exist)", info.name.c_str());
	}
	// matches <int> <operator> [value]
	else if (useRegex(line, R"(^\s*([^\s\-\+]+)\s*([\+\-\*\/\=]+)\s*(f?\".*\"|\{.*\}|[^\s]+\(.*\)|[^\s]*)\s*.*$)")) { // A math operator.`
		variable* existingVar = getVarFromName(matches.str(1), NULL);

		if (existingVar != nullptr && !(existingVar->type == "int" || existingVar->type == "float")) {
			HCL::throwError(true, "Cannot perform any math operations to a non-int variable (Variable '%s' isn't int/float typed, can't operate to a '%s' type).", existingVar->name.c_str(), existingVar->type.c_str());
		}
		else if (existingVar != nullptr) {
			double res;
			if (matches.str(2) == "++")
				res = std::stod(existingVar->value[0]) + 1;
			else if (matches.str(2) == "--")
				res = std::stod(existingVar->value[0]) - 1;
			else if (matches.str(2) == "+=")
				res = std::stod(existingVar->value[0]) + std::stod(matches.str(3));
			else if (matches.str(2) == "-=")
				res = std::stod(existingVar->value[0]) - std::stod(matches.str(3));
			else if (matches.str(2) == "*=")
				res = std::stod(existingVar->value[0]) * std::stod(matches.str(3));
			else if (matches.str(2) == "/=")
				res = std::stod(existingVar->value[0]) / std::stod(matches.str(3));

			if (existingVar->type == "int") {
				existingVar->value[0] = std::to_string((int)res);
			}
			else if (existingVar->type == "float") {
				existingVar->value[0] = std::to_string((float)res);
			}
		}
		else {
			throwError(true, "Cannot perform any math operations to this variable (Variable '%s' does not exist).", matches.str(1).c_str());
		}
	}
	// Matches the type, name and value
	else if (useRegex(line, R"(^\s*([^\s]+)\s+([^\s]+)?\s*=?\s*(f?\".*\"|\{.*\}|[^\s]+\(.*\)|[^\s]*)\s*$)")) { // Declaring a new variable.
		std::string ogValue = matches.str(3);
		variable var = {matches.str(1), matches.str(2), {unstringify(ogValue)}};
		structure s;

		if (typeIsValid(var.type, &s)) { // Is type cored or a structure.
			variable structVar;
			variable* existingVar = getVarFromName(var.value[0], &structVar);

			// Copy over an existing variable to this new one.
			if (existingVar != nullptr) {
				// If 'getVarFromName' finds a variable from a struct, `structVar.extra` becomes the index of where the member is in the struct.
				// However, due to `structVar.extra` being a std::string, we have to convert it to an integer.
				int index = std::stoi(structVar.extra[0]);
				var.value = {structVar.value[index]};
			}
			// Check if the value is a function. If so, execute the
			// function and get its return, so that it gets assigned
			// to the variable.
			else if (useRegex(var.value[0], R"(\s*([^\s\(]+)\((.*)\)\s*)")) {
				assignFuncReturnToVar(&var, matches.str(1), matches.str(2));
			}
			// The type is a struct.
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
						std::vector<std::string> valueList = split(unstringify(var.value[0], true), ",", "\"\"");

						if (valueList.size() > s.value.size()) {
							throwError(true, "Too many values are provided when declaring the variable '%s' (you provided '%i' arguments when struct type '%s' has only '%i' members).", var.name.c_str(), valueList.size(), var.type.c_str(), s.value.size());
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
									throwError(true, "Too few values are provided to fully initialize a struct (you provided '%i' arguments when struct type '%s' has '%i' members).", valueList.size(), var.type.c_str(), s.value.size());
								else
									break;
							}
						}
					}
				}
			}
			else if (!var.value[0].empty() && existingVar == NULL && !isInt(var.value[0]) && (var.value[0] != "true" && var.value[0] != "false") && !(find(ogValue, "\"") && var.type == "string")) {
				// This checks for if the user is trying to copy over a variable that doesn't exist. All of these checks check if it isn't just some core type so that it wouldn't output a false-negative.
				throwError(true, "You cannot copy over a variable that doesn't exist (variable '%s' does not exist).", var.value[0].c_str());
			}
		}
		else {
			throwError(true, "Type '%s' doesn't exist (Cannot init a variable without valid type).", var.type.c_str());
		}
		getValueFromFstring(ogValue, var.value[0]);
		var.value[0] = convertBackslashes(var.value[0]);

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
			std::cout << indent << c << std::endl;
		std::cout << indent << "}" << std::endl;
	}
}


void HCL::resetRuntimeInfo() {
	curFile.clear();
	line.clear();
	lineCount = 0;
	mode = SAVE_NOTHING;
	matches = {};
}


void HCL::throwError(bool sendRuntimeError, std::string text, ...) {
	va_list valist;
	va_start(valist, text);
	std::string msg = colorText("Error at ", OUTPUT_RED) + "'" + colorText(curFile + ":" + std::to_string(lineCount), OUTPUT_YELLOW) + "'" + colorText(": ", OUTPUT_RED);

	for (int i = 0; i < text.size(); i++) {
		auto x = text[i];
		int num = 0;

		if (x == '%' && (i + 1) < text.size()) {
			switch (text[i + 1]) {
				case 's':
					msg += colorText(va_arg(valist, const char*), OUTPUT_YELLOW);
					break;
				case 'i':
					num = va_arg(valist, int);
					msg += colorText(std::to_string(num), OUTPUT_YELLOW);
					break;

				default:
					break;
			}
			i++;
		}
		else msg += x;
	}
	va_end(valist);

	if (debug && sendRuntimeError)
		debugMode();

	if (sendRuntimeError)
		throw std::runtime_error("\x1B[0m" + msg);
	else
		std::printf("\x1B[0m%s\n", msg.c_str());
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
	HCL::vector matches;

	// Defnitions that are saved in memory.
	std::vector<variable> variables;
	std::vector<structure> structures;
	std::vector<function> functions;
	//std::vector<std::string> scope;
	void* functionOutput = nullptr;
	std::string functionReturnType = "";
}