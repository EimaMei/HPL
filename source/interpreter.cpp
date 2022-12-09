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
			if (!arg.interprete)
				break;

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
	if (!arg.interprete)
		return FOUND_NOTHING;

	if (find(str, "//")) // Ignore comments
		str = split(str, "//", "\"\"")[0];

	line = str;

	if (arg.breakpoint) { // A breakpoint was set.
		if (curFile == arg.breakpointValues.first && lineCount == arg.breakpointValues.second) {
			std::cout << "Breakpoint reached at " << curFile << ":" << lineCount << std::endl;
			arg.interprete = false;
		}
	}

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
	bool rightBracket = useRegex(line, R"(^.*\s*\{\s*$)");

	if (bracket)
		equalBrackets--;
	if (useRegex(line, R"(^.*\s*\{\s*$)"))
		equalBrackets++;

	if (bracket && mode == MODE_SAVE_STRUCT) {
		mode = MODE_DEFAULT;

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			arg.curIndent.pop_back();
			std::cout << arg.curIndent << "LOG: [DONE][STRUCT]: " << curFile << ":" << lineCount << ": struct <name> {...}: struct " << structures.front().name << " {...}" << std::endl;
		}

		return FOUND_SOMETHING;
	}
	else if (bracket && mode == MODE_SAVE_FUNC && equalBrackets == 0) { // Last line, do not save anymore after this.
		mode = MODE_DEFAULT;

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			arg.curIndent.pop_back();
			std::cout << arg.curIndent << "LOG: [DONE][FUNCTION]: " << curFile << ":" << lineCount << ": <type> <name>(<params>): " << printFunction(functions.back()) << std::endl;
		}

		return FOUND_SOMETHING;
	}
	else if (mode == MODE_SAVE_FUNC) {
		functions.back().code.push_back(line);

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			//std::cout << arg.curIndent << "LOG: [ADDED][LINE]: " << curFile << ":" << lineCount << ": <line>: " << line << std::endl;
		}

		return FOUND_SOMETHING;
	}
	else if (mode == MODE_SCOPE_IGNORE_ALL) {
		if (bracket && equalBrackets + 1 == ifStatements.back().startingLine) {
			if (ifStatements.size() > 1)
				mode = MODE_SCOPE_IF_STATEMENT;
			else
				mode = MODE_DEFAULT;
		}
		return FOUND_SOMETHING;
	}
	else if ((bracket || rightBracket) && mode == MODE_SCOPE_IF_STATEMENT && !ifStatements.empty()) {
		std::vector<HCL::variable> oldVars = HCL::variables;
		mode = MODE_DEFAULT;

		if (ifStatements.back().type == "invalid") {
			if (equalBrackets != 0)
				mode = MODE_SCOPE_IF_STATEMENT;
			else
				mode = MODE_DEFAULT;
			ifStatements.pop_back();

			if (rightBracket)
				return FOUND_NOTHING;
			else
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
			mode = MODE_SCOPE_IF_STATEMENT;

		if (bracket || (rightBracket && ifStatements.empty()))
			return FOUND_SOMETHING;
	}
	else if (mode == MODE_SCOPE_IF_STATEMENT) {
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
		mode = MODE_SCOPE_IF_STATEMENT;
		ifStatements.push_back({.startingLine = equalBrackets});

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
				variable* existingVar = getVarFromName(p);

				// Copy over an existing variable to this new one.
				if (existingVar != nullptr) {
					if (!coreTyped(existingVar->type)) // Struct
						HCL::throwError(true, "Cannot compare a struct in an if-statement");
					else
						p = xToStr(existingVar->value);

					type = existingVar->type;
				}
				// Check if the value is a function.
				else if (useRegex(p, R"(\s*([^\s\(]+)\((.*)\)\s*)")) {
					variable var;
					assignFuncReturnToVar(&var, matches.str(1), matches.str(2));

					if (coreTyped(var.type))
						params[i] = p = xToStr(var.value);
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
				mode = MODE_SCOPE_IGNORE_ALL;
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
					mode = MODE_SCOPE_IGNORE_ALL;

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
				variable var = {matches.str(1), matches.str(2)};

				if (matches.str(3).empty())
					func.minParamCount++;
				else
					var.value = unstringify(matches.str(3));

				func.params.push_back(var);
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
		function f; HCL::variable res;

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [USE][FUNCTION]: " << curFile << ":" << lineCount << ": <name>(<params>): " << matches.str(1) << "(" << matches.str(2) << ")" << std::endl;
		}

		return executeFunction(matches.str(1), matches.str(2), f, res);
	}
	else if (useRegex(line, R"(\s*return\s+(f?\".*\"|\{.*\}|[^\s]+\(.*\)|[^\s]*)\s*)")) {
		setCorrectValue(functionOutput, matches.str(1));

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [FOUND][RETURN]: " << curFile << ":" << lineCount << ": return <value> (<type>): return " << xToStr(functionOutput.value) << " (" << functionOutput.type << ")" << std::endl;
		}

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


int HCL::checkVariables() {
	// <name> = <value>
	if (useRegex(line, R"(^\s*([^\s]+)\s*[^\-\+\/\*]=\s*(f?\".*\"|\{.*\}|\w*\(.*\)|[\d\s\+\-\*\/\.]+[^\w]*|[^\s]*)\s*.*$)")) { // Edit a pre-existing variable
		variable info = {"", matches.str(1), matches.str(2)};
		auto& value = getStr(info.value);

		variable* existingVar = getVarFromName(info.name);


		if (existingVar != nullptr) {
			setCorrectValue(*existingVar, value);
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
		variable* existingVar = getVarFromName(matches.str(1));
		double res;

		if (existingVar != nullptr && !(existingVar->type == "int" || existingVar->type == "float")) {
			HCL::throwError(true, "Cannot perform any math operations to a non-int variable (Variable '%s' isn't int/float typed, can't operate to a '%s' type).", existingVar->name.c_str(), existingVar->type.c_str());
		}
		else if (existingVar != nullptr) {
			double dec1 = xToType<float>(existingVar->value);
			double dec2 = std::stod(matches.str(3));

			if (matches.str(2) == "++")
				res = dec1 + 1;
			else if (matches.str(2) == "--")
				res = dec1 - 1;
			else if (matches.str(2) == "+=")
				res = dec1 + dec2;
			else if (matches.str(2) == "-=")
				res = dec1 - dec2;
			else if (matches.str(2) == "*=")
				res = dec1 * dec2;
			else if (matches.str(2) == "/=")
				res = dec1 / dec2;
			else if (matches.str(2) == "%=")
				res = fmod(dec1, dec2);

			if (existingVar->type == "int") {
				existingVar->value = (int)res;
			}
			else if (existingVar->type == "float") {
				existingVar->value = (float)res;
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
		variable var = {matches.str(1), matches.str(2), matches.str(3)};
		auto& value = getStr(var.value);
		structure s;

		if (value.empty())
			var.reset_value();

		if (!typeIsValid(var.type, &s)) // Type isn't cored or a structure.
			throwError(true, "Type '%s' doesn't exist (Cannot init a variable without valid type).", var.type.c_str());

		if (!s.name.empty()) { // The returned type from `typeIsValid` returned a struct
			if (useRegex(value, R"(\s*([^\s]+)\((.*)\))")) { // Value is a function.
				if (HCL::arg.debugAll || HCL::arg.debugLog) {
					std::cout << arg.curIndent << "LOG: [USE][FUNCTION]: " << curFile << ":" << lineCount << ": <name>(<params>): " << matches.str(1) << "(" << matches.str(2) << ")" << std::endl;
				}

				assignFuncReturnToVar(&var, HCL::matches.str(1), HCL::matches.str(2));
			}
			else if (!var.has_value()) { // Nothing is set, meaning it's just the struct's default arguments.
				std::vector<variable> res = s.value;
				var.value = res;
			}
			else { // Oh god oh fuck it's a custom list.
				// We don't split the string by the comma if the comma is inside double quotes
				// Meaning "This, yes this, exact test string" won't be split. We also remove the curly brackets before splitting.
				std::vector<std::string> valueList = split(unstringify(value, true), ",", "\"\"{}");
				std::vector<HCL::variable> result;

				if (valueList.size() > s.value.size()) {
					throwError(true, "Too many values are provided when declaring the variable '%s' (you provided '%i' arguments when struct type '%s' has only '%i' members).", var.name.c_str(), valueList.size(), var.type.c_str(), s.value.size());
				}

				// The first unstringify is used to remove any unneeded spaces.
				// Then the second removes the double quotes if it's a string.
				for (int i = 0; i < s.value.size(); i++) {
					if (i < s.value.size() && i < valueList.size()) {
						result.push_back({s.value[i].type, s.value[i].name, unstringify(unstringify(valueList[i], false, ' '))});
					}
					else { // Looks like the user didn't provide the entire argument list. That's fine, though we must check for any default options.
						if (s.value[i].has_value()) {
							result.push_back(s.value[i]);
						}
						else if (arg.strict)
							HCL::throwError(true, "Too few values are provided to fully initialize a struct (you provided '%i' arguments when struct type '%s' has '%i' members).", valueList.size(), var.type.c_str(), s.value.size());
						else
							HCL::throwError(true, "Shouldn't happen?");
					}
				}

				var.value = result;
			}
		}
		else if (!value.empty())
			setCorrectValue(var, value);

		if (mode == MODE_SAVE_STRUCT) // Save variables inside a struct.
			structures.begin()->value.push_back(var);
		else
			variables.push_back(var);


		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [CREATE][VARIABLE]: " << curFile << ":" << lineCount << ": <type> <variable> = [value]: " << var.type << " " << var.name;
			if (!value.empty()) {
				std::cout << " = ";
				print(var, "");
			}
			std::cout << std::endl;
		}

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
		auto& var = vars[index];

		RETURN_OUTPUT clr = OUTPUT_PURPLE;

		if (!coreTyped(var.type))
			clr = OUTPUT_NOTHING;

		std::cout << tabs << colorText(var.type, clr) << " " << var.name;
		if (var.has_value()) std::cout << " = ";
		print(var, "");

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
	functionOutput.reset_value();

	lineCount = 0;
	mode = MODE_DEFAULT;
	matches = {};
}


void HCL::throwError(bool sendRuntimeError, std::string text, ...) {
	va_list valist;
	va_start(valist, text);
	std::string msg = colorText("Error at ", OUTPUT_RED) + "'" + colorText(curFile + ":" + std::to_string(lineCount), OUTPUT_YELLOW) + "'" + colorText(": ", OUTPUT_RED);
	bool colorMode = false;

	for (int i = 0; i < text.size(); i++) {
		auto x = text[i];
		int num = 0;
		colorMode = false;

		if (x == '%' && (i + 1) < text.size()) {
			if ((i + 1) < text.size()) {
				if (text[i - 1] == '\'' && text[i + 2] == '\'')
					colorMode = true;
			}

			switch (text[i + 1]) {
				case 's':
					if (colorMode)
						msg += colorText(va_arg(valist, const char*), OUTPUT_YELLOW);
					else
						msg += va_arg(valist, const char*);

					break;
				case 'd':
				case 'i':
					num = va_arg(valist, int);

					if (colorMode)
						msg += colorText(std::to_string(num), OUTPUT_YELLOW);
					else
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

	HCL::variable functionOutput;
}