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
#include <functions.hpp>

#include <iostream>
#include <string.h>
#include <math.h>

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
	if (checkModes() == FOUND_SOMETHING) return FOUND_SOMETHING;
	if (checkConditions() == FOUND_SOMETHING) return FOUND_SOMETHING;
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
			match = "core/" + unstringify(match, true);
		else if (find(line, "\"")) // Some library '#include "../somelib.hcl"'
			match = getPathFromFilename(oldFile) + "/" + match;

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [INCLUDE][FILE]: " << curFile << ":" << lineCount << ": #include <file>: #include \"" << match << "\"" << std::endl;
		}
		interpreteFile(match);

		curFile = oldFile;
		lineCount = oldLineCount;

	}
	return FOUND_NOTHING;
}


int HCL::checkModes() {
	bool bracket = useRegex(line, R"(^\s*(\})\s*$)");

	if (bracket)
		equalBrackets--;
	if (useRegex(line, R"(^.*\s*\{\s*$)"))
		equalBrackets++;

	if (bracket && mode == MODE_SAVE_STRUCT) {
		mode = MODE_SAVE_NOTHING;

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			arg.curIndent.pop_back();
			std::cout << arg.curIndent << "LOG: [DONE][STRUCT]: " << curFile << ":" << lineCount << ": struct <name> {...}: struct " << structures.back().name << "{...}" << std::endl;
		}

		return FOUND_SOMETHING;
	}
	else if (bracket && mode == MODE_SAVE_FUNC && equalBrackets == 0) { // Last line, do not save anymore after this.
		mode = MODE_SAVE_NOTHING;

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			arg.curIndent.pop_back();
			std::cout << arg.curIndent << "LOG: [DONE][FUNCTION]: " << curFile << ":" << lineCount << ": <type> <name>(<params>): " << printFunction(functions.back()) << std::endl;
		}

		return FOUND_SOMETHING;
	}
	else if (mode == MODE_SAVE_FUNC) {
		functions.back().code.push_back(line);

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [ADDED][LINE]: " << curFile << ":" << lineCount << ": <line>: " << line << std::endl;
		}

		return FOUND_SOMETHING;
	}
	else if (bracket && mode == MODE_SCOPE_IF && !ifStatements.empty()) {
		std::vector<HCL::variable> oldVars = HCL::variables;
		mode = MODE_SAVE_NOTHING;

		if (ifStatements.back().type == "invalid") {
			if (equalBrackets != 0)
				mode = MODE_SCOPE_IF;
			ifStatements.pop_back();

			return FOUND_SOMETHING;
		}

		HCL::lineCount -= ifStatements.front().code.size();

		for (auto line : ifStatements.front().code) {
			HCL::lineCount++;
			HCL::interpreteLine(line);
		}

		// If a global variable was edited in the function, save the changes.
		for (auto& oldV : oldVars) {
			for (auto newV : HCL::variables) {
				if (oldV.name == newV.name)
					oldV.value = newV.value;
			}
		}
		HCL::variables = oldVars;
		ifStatements.erase(ifStatements.begin());
		arg.curIndent.pop_back();

		if (!ifStatements.empty())
			mode = MODE_SCOPE_IF;

		return FOUND_SOMETHING;
	}
	else if (mode == MODE_SCOPE_IF) {
		if (ifStatements.empty())
			return FOUND_SOMETHING;

		if (ifStatements.back().type == "invalid") {
			return FOUND_SOMETHING;
		}
		if (useRegex(line, R"(^\s*if\s*(.*)\s*\{$)")) {
			return FOUND_NOTHING;
		}
		ifStatements.back().code.push_back(line);

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


int HCL::checkConditions() {
	if (useRegex(line, R"(^\s*if\s*(.*)\s*\{$)")) {
		arg.curIndent += "\t";
		std::vector params = split(matches.str(1), " ", "\"\"{}");

		std::string oldValue = matches.str(1);
		mode = MODE_SCOPE_IF;
		ifStatements.push_back({});

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [FOUND][IF-STATEMENT]: " << curFile << ":" << lineCount << ": if (<condition>): if (" << oldValue << ")" << std::endl;
		}

		std::string _operator;
		bool failed = true;
		arg.curIndent += "\t";

		for (int i = 0; i < params.size(); i++) {
			auto& p = params[i];
			std::string type = getTypeFromValue(p);
			p = unstringify(p);

			if (type.empty()) {
				variable structVar;
				variable* existingVar = getVarFromName(p, &structVar);

				// Copy over an existing variable to this new one.
				if (existingVar != nullptr) {
					if (!coreTyped(existingVar->type)) // Struct
						HCL::throwError(true, "qweqwe can't check full struct");
					else if (!structVar.value.empty()) {// Edit a struct member
						int index = std::stoi(structVar.extra[0]);
						p = existingVar->value[index];
					}
					else
						p = existingVar->value[0];

					type = existingVar->type;
				}
				// Check if the value is a function.
				else if (useRegex(p, R"(\s*([^\s\(]+)\((.*)\)\s*)")) {
					variable var;
					assignFuncReturnToVar(&var, matches.str(1), matches.str(2));

					if (coreTyped(var.type))
						p = var.value[0];
					else
						HCL::throwError(true, "Cannot compare struct variables in an if-statement (variable '%s' is struct-typed)", p.c_str());
				}
				else if (existingVar == nullptr && !isInt(p) && (p != "true" && p != "false") && !(find(p, "\""))) {
					throwError(true, "You cannot use a variable that doesn't exist in an if-statement (variable '%s' does not exist).", p.c_str());
				}
				getValueFromFstring(p, p);
			}

			if (HCL::arg.debugAll || HCL::arg.debugLog) {
				std::cout << arg.curIndent << "LOG: [CHECKING][IF-STATEMENT-VALUE]: " << curFile << ":" << lineCount << ": <value> ([type]): " << p << " (" << type << ")" << std::endl;
			}

			if (p == "false" || p == "0") {
				for (int x = i; x < params.size(); x++) {
					if (params[x] == "||") { failed = false; break; }
				}
				if (!failed) { failed = true; continue; }

				ifStatements.back().type = "invalid";
				arg.curIndent.pop_back();
				arg.curIndent.pop_back();

				if (HCL::arg.debugAll || HCL::arg.debugLog) {
					std::cout << arg.curIndent << "LOG: [FAILED][IF-STATEMENT]: " << curFile << ":" << lineCount << ": Condition failed, output returned false." << std::endl;
				}
				return FOUND_SOMETHING;
			}
			if (type == "relational-operator") {
				_operator = p;
				continue;
			}

			if (!_operator.empty()) {
				bool res = false;

				if (_operator == "==")
					res = (params[i - 2] == p);
				else if (_operator == "!=")
					res = (params[i - 2] != p);
				else if (_operator == ">=")
					res = (params[i - 2] >= p);
				else if (_operator == "<=")
					res = (params[i - 2] <= p);
				else if (_operator == ">")
					res = (params[i - 2] >  p);
				else if (_operator == "<")
					res = (params[i - 2] <  p);

				if (HCL::arg.debugAll || HCL::arg.debugLog) {
					std::cout << arg.curIndent << "LOG: [OPERATOR][RELATION]: " << curFile << ":" << lineCount << ": <value 1> <operator> <value 2>: " << params[i - 2] << " " << _operator << " " <<  p << " (" << (res == true ? "true" : "false") << ")" << std::endl;
				}

				_operator = "";

				if (!res) {
					for (int x = i; x < params.size(); x++) {
						if (params[x] == "||") { failed = false; break; }
					}
					if (!failed) { failed = true; continue; }

					ifStatements.back().type = "invalid";

					if (HCL::arg.debugAll || HCL::arg.debugLog) {
						arg.curIndent.pop_back();
						std::cout << arg.curIndent << "LOG: [FAILED][IF-STATEMENT]: " << curFile << ":" << lineCount << ": Condition failed, output returned false." << std::endl;
					}

					return FOUND_SOMETHING;
				}
			}
		}
		arg.curIndent.pop_back();

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


int HCL::checkStruct() {
	// 'struct <name> {' Only matches the <name>
	if (useRegex(line, R"(struct\s+([a-zA-Z]+)\s*\{)")) {
		std::string name = removeSpaces(matches.str(1));

		structures.insert(structures.begin(), {name});
		mode = MODE_SAVE_STRUCT;

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [CREATE][STRUCT]: " << curFile << ":" << lineCount << ": struct <name>: struct " << name << std::endl;
			arg.curIndent += "\t";
		}

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}



int HCL::checkFunctions() {
	// Match <return type> <name>(<params>) {
	if (useRegex(line, R"(\s*([^\s]+)\s+([^\s]+)\s*\((.*)\)\s*\{)")) {
		function func = {matches.str(1), matches.str(2)};
		std::vector<std::string> params = split(matches.str(3), ",", "\"\"");

		for (auto v : params) {
			if (useRegex(v, R"(\s*([^\s]+)\s+([^\s]+)?\s*=?\s*(f?\".*\"|\{.*\}|[^\s*]*)\s*)")) {
				variable var = {matches.str(1), matches.str(2), {unstringify(matches.str(3))}};
				func.params.push_back(var);

				if (matches.str(3).empty()) func.minParamCount++;
			}
		}
		func.file = HCL::curFile;
		func.startingLine = HCL::lineCount;

		functions.push_back(func);
		mode = MODE_SAVE_FUNC;

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [CREATE][FUNCTION]: " << curFile << ":" << lineCount << ": <type> <name>(<params>): " << printFunction(func) << std::endl;
			arg.curIndent += "\t";
		}

		return FOUND_SOMETHING;
	}
	else if (useRegex(line, R"(^\s*([^\s\(]+)\((.*)\)\s*$)")) {
		function f; std::string n;

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [USE][FUNCTION]: " << curFile << ":" << lineCount << ": <name>(<params>): " << matches.str(1) << "(" << matches.str(2) << ")" << std::endl;
		}

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
		else {
			HCL::throwError(true, "Internal HCL error: %s", matches.str(1).c_str());
		}

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [FOUND][RETURN]: " << curFile << ":" << lineCount << ": return <value> | <type>: return " << functionOutput << " " << functionReturnType << std::endl;
		}

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


int HCL::checkVariables() {
	// <name> = <value>
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
					existingVar->value = possibleVar->value;
				else {// Edit a struct member
					int index = std::stoi(possibleStructInfo.extra[0]);
					existingVar->value[0] = possibleVar->value[index];
				}
			}
			else if (!(res = extractMathFromValue(info.value[0], existingVar)).empty()) { // A math expression.
				info.value[0] = res;
			}

			else if (coreTyped(existingVar->type)) {// Edit a regular variable
				existingVar->value = info.value;
			}
			else if (typeIsValid(existingVar->type)) { // A struct variable
				if (find(existingVar->name, ".")) { // Edit a struct member
					int index = std::stoi(structInfo.extra[0]);
					existingVar->value[index] = info.value[0];
				}
				else { // Entire struct edit
					std::vector<std::string> params = split(unstringify(info.value[0], true), ",", "\"\"{}");
					HCL::structure* s = getStructFromName(existingVar->type);
					existingVar->value.clear();
					existingVar->extra.clear();

					for (int i = 0; i < params.size(); i++) {
						auto& p = params[i];
						auto& sMem = s->value[i];
						std::string res;

						p = unstringify(p, false, ' ');
						getValueFromFstring(p, res);
						p = unstringify(p);
						possibleVar = getVarFromName(p);

						if (possibleVar != nullptr)
							p = possibleVar->value[0];
						else if (!res.empty())
							p = res;
						else if (p.front() == '{' && p.back() == '}') {
						}
						existingVar->value.push_back(p);
						existingVar->extra.push_back(sMem.type);
					}
				}
			}

			getValueFromFstring(ogValue, existingVar->value[0]);
		}
		else
			HCL::throwError(true, "Variable '%s' doesn't exist (Can't edit a variable that doesn't exist)", info.name.c_str());

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [EDIT][VARIABLE]: " << curFile << ":" << lineCount << ": <variable> = <value>: " << existingVar->name << " = ";
			print(*existingVar);
		}
	}
	//<int> <operator> [value]
	else if (useRegex(line, R"(^\s*([^\s\-\+]+)\s*([\+\-\*\/\=]+)\s*(f?\".*\"|\{.*\}|[^\s]+\(.*\)|[^\s]*)\s*.*$)")) { // A math operator.`
		variable* existingVar = getVarFromName(matches.str(1), NULL);
		double res;

		if (existingVar != nullptr && !(existingVar->type == "int" || existingVar->type == "float")) {
			HCL::throwError(true, "Cannot perform any math operations to a non-int variable (Variable '%s' isn't int/float typed, can't operate to a '%s' type).", existingVar->name.c_str(), existingVar->type.c_str());
		}
		else if (existingVar != nullptr) {
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
			else if (matches.str(2) == "%=")
				res = fmod(std::stod(existingVar->value[0]), std::stod(matches.str(3)));

			if (existingVar->type == "int") {
				existingVar->value[0] = std::to_string((int)res);
			}
			else if (existingVar->type == "float") {
				existingVar->value[0] = std::to_string((float)res);
				existingVar->value[0].erase(existingVar->value[0].find_last_not_of('0') + 1, std::string::npos);
			}
		}
		else {
			throwError(true, "Cannot perform any math operations to this variable (Variable '%s' does not exist).", matches.str(1).c_str());
		}

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [MATH][VARIABLE]: " << curFile << ":" << lineCount << ": <variable> <operator> [value]: " << existingVar->name << " " << matches.str(2);
			if (matches.str(2) != "--" || matches.str(2) != "++") std::cout << " " << res;
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
				if (structVar.value.empty()) // Full struct.
					var.value = existingVar->value;
				else {// Edit a struct member
					int index = std::stoi(structVar.extra[0]);
					var.value = {existingVar->value[index]};
				}
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
								else if (arg.strict)
									throwError(true, "Too few values are provided to fully initialize a struct (you provided '%i' arguments when struct type '%s' has '%i' members).", valueList.size(), var.type.c_str(), s.value.size());
								else
									break;
							}
						}
					}
				}
			}
			else if (!var.value[0].empty() && existingVar == nullptr && !isInt(var.value[0]) && (var.value[0] != "true" && var.value[0] != "false") && !(find(ogValue, "\"") && var.type == "string")) {
				// This checks for if the user is trying to copy over a variable that doesn't exist. All of these checks check if it isn't just some core type so that it wouldn't output a false-negative.
				throwError(true, "You cannot copy over a variable that doesn't exist (variable '%s' does not exist).", var.value[0].c_str());
			}
		}
		else {
			throwError(true, "Type '%s' doesn't exist (Cannot init a variable without valid type).", var.type.c_str());
		}
		getValueFromFstring(ogValue, var.value[0]);
		var.value[0] = convertBackslashes(var.value[0]);

		if (mode == MODE_SAVE_STRUCT) // Save variables inside a struct.
			structures.begin()->value.push_back(var);
		else
			variables.push_back(var);


		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [CREATE][VARIABLE]: " << curFile << ":" << lineCount << ": <type> <variable> = [value]: " << var.type << " " << var.name;
			if (!var.value[0].empty()) {
				std::cout << " = ";
				print(var, "");
			}
			std::cout << std::endl;
		}
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
	mode = MODE_SAVE_NOTHING;
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

	if ((HCL::arg.debugAll || HCL::arg.debugPrint) && sendRuntimeError)
		debugMode();

	if (sendRuntimeError)
		throw std::runtime_error("\x1B[0m" + msg);
	else
		std::printf("\x1B[0m%s\n", msg.c_str());
}


namespace HCL {
	// Inteperter configs.
	HCL::configArgs arg;

	// Interpreter rules runtime.
	std::string curFile;
	std::string line;
	int lineCount = 0;
	int mode = 0;
	HCL::vector matches;
	int equalBrackets = 0;

	// Defnitions that are saved in memory.
	std::vector<variable> variables;
	std::vector<structure> structures;
	std::vector<function> functions;
	std::vector<function> ifStatements;

	void* functionOutput = nullptr;
	std::string functionReturnType = "";
}