/*
* Copyright (C) 2022-2023 EimaMei/Sacode
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
#include <parser.hpp>

#include <scope/hoi4scripting.hpp>

#include <iostream>
#include <string.h>
#include <math.h>
#include <map>


void HPL_log(std::string typeOfLog, std::string logFormat, std::string logInfo, int tabsToRemove = 0, HPL::RETURN_OUTPUT outputColor = HPL::OUTPUT_NOTHING, bool light = false, const char* filename = __FILE__, int line = __LINE__) {
	if (tabsToRemove != 0)
		HPL::arg.curIndent.erase(0, tabsToRemove);

	if (HPL::arg.debugAll || HPL::arg.debugLog)
		std::cout << HPL::arg.curIndent << "LOG: " << colorText(typeOfLog, outputColor, light) << ": " << HPL::curFile << ":" << HPL::lineCount << "(" << filename << ":" << line << "): " << logFormat <<": " << logInfo << std::endl;
}

std::vector<std::string> alreadyReadFiles;
std::map<std::string, HPL::variable*> vars;

std::string HPL::colorText(std::string txt, RETURN_OUTPUT type, bool light/* = false*/) {
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

	return std::string{};
}


void HPL::interpreteFile(std::string file) {
	curFile = file;
	FILE* fp = fopen(curFile.c_str(), "r");
	bool alreadyRead = false;

	for (const auto& readFile : alreadyReadFiles) {
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

			HPL::interpreteLine(line);
		}
		if (!alreadyRead)
			std::cout << colorText("Finished interpreting " +std::to_string(lineCount)+ " lines from ", OUTPUT_GREEN) << "'" << colorText(curFile, OUTPUT_YELLOW) << "'" << colorText(" successfully", OUTPUT_GREEN) << std::endl;
	}
	else
		std::cout << HPL::colorText("Error: ", HPL::OUTPUT_RED) << HPL::colorText(curFile, HPL::OUTPUT_RED) << ": No such file or directory" << std::endl;

	if (HPL::arg.objectify) {
		FILE* f = fopen((curFile.substr(0, curFile.find_last_of(".")) + ".hplo").c_str(), "w");

		if (f != NULL) {
			for (auto const& object : HPL::objects)
				fwrite(&object, sizeof(HPL::object), 1, f);
			fclose(f);
		}
	}

	fclose(fp);
	resetRuntimeInfo();
	alreadyReadFiles.push_back(file);
}


int HPL::interpreteLine(const std::string& str) {
	if (!arg.interprete)
		return FOUND_NOTHING;
	bool found = false;

	line = removeFrontAndBackSpaces(str);

	if (find(line, "//")) // Ignore comments
		line = split(line, "//", "\"\"")[0];


	if (arg.breakpoint && (mode != MODE_SAVE_FUNC && mode != MODE_SAVE_STRUCT)) { // A breakpoint was set.
		if (curFile == arg.breakpointValues.first && lineCount == arg.breakpointValues.second) {
			std::cout << "Breakpoint reached at " << curFile << ":" << lineCount << std::endl;
			arg.interprete = false;
		}
	}

	HPL::vector matches;
	parseVariable(line, matches);
	//for (const auto& v : matches.value) { std::cout << "|" << v << "| "; }
	//std::cout << std::endl;



	if (checkIncludes(matches) == FOUND_SOMETHING) found = true;
	else if (checkModes(matches) == FOUND_SOMETHING) found = true;
	else if (checkConditions(matches) == FOUND_SOMETHING) found = true;
	//else if (checkFunctions() == FOUND_SOMETHING) found = true;
	else if (checkStruct(matches) == FOUND_SOMETHING) found = true;
	else if (checkVariables(matches) == FOUND_SOMETHING) found = true;

	return found;
}


int HPL::checkIncludes(HPL::vector& m) {
	if (m.str(1) == "#include") {
		std::string includeName = m.str(2);

		if (includeName.front() == '<' && includeName.back() == '>')
			includeName = "core/" + removeFrontAndBackLetters(includeName);
		else if (includeName.front() == '\"' && includeName.back() == '\"')
			includeName = getPathFromFilename(curFile) + "/" + removeFrontAndBackLetters(includeName);
		else {
			std::string includeFilename = removeFrontAndBackLetters(includeName);
			HPL::throwError(true, "Invalid front and back characters (it should be '#include <%s>' or '#include \"%s\", not '#include %s')", includeFilename.c_str(), includeFilename.c_str(), includeName.c_str());
		}

		// Save the old data, since when we're gonna interprete a new file, it's gonna overwrite the old data but not reinstate it when it's finished.
		// Which means we have to reinstate the old data ourselves.
		auto oldFile = curFile;
		auto oldLineCount = lineCount;
		auto oldSettings = arg;
		lineCount = 0; // We reset it so that the lineCount won't be innaccurate when the lines gets interpreted correctly.

		HPL_log("[INCLUDE][FILE]", "#include <file>", "#include \"" + includeName + "\"");
		interpreteFile(includeName);

		curFile = oldFile;
		lineCount = oldLineCount;
		arg = oldSettings;

	}
	return FOUND_NOTHING;
}


int HPL::checkModes(HPL::vector& m) {
	//if ((HPL::arg.debugAll || HPL::arg.debugLog) && mode != oldMode) {
	//	std::cout << arg.curIndent << colorText("LOG: [CURRENT][MODE]: ", HPL::OUTPUT_RED) << curFile << ":" << lineCount << ": <name of the mode>: " << getModeName() << std::endl;
	//	oldMode = mode;
	//}

	if (mode == MODE_SAVE_SCOPE) {
		variables[scopeIndex].value = xToStr(variables[scopeIndex].value) + HSM::interpreteLine(line);

		if (mode == MODE_DEFAULT) {
			HPL::scopeIndex = -1;
			HSM::equalBrackets = 1;

			HPL_log("[HSM][OFF]", "", "HSM mode turned off, back to HPL", 0, HPL::OUTPUT_CYAN, true);
		}

		return FOUND_SOMETHING;
	}
	bool leftBracket = (m.str(1) == "}" && m.size() == 1);
	bool rightBracket = (m.str(m.size()) == "{");

	if (leftBracket)
		equalBrackets--;
	else if (rightBracket) {
		equalBrackets++;

		switch (mode) {
			case MODE_CHECK_STRUCT: mode = MODE_SAVE_STRUCT; break;
			case MODE_CHECK_FUNC: mode = MODE_SAVE_FUNC; return FOUND_SOMETHING;
			case MODE_CHECK_IF_STATEMENT: mode = MODE_SCOPE_IF_STATEMENT; return FOUND_SOMETHING; // NEEDS FIXING TOO.
			case MODE_CHECK_IGNORE_ALL: mode = MODE_SCOPE_IGNORE_ALL; return FOUND_SOMETHING;
			default: break;
		}
	}

	if (mode == MODE_DEFAULT)
		return FOUND_NOTHING;


	if (mode == MODE_SAVE_STRUCT) {
		if (rightBracket) {
			mode = MODE_DEFAULT;
			HPL_log("[DONE][STRUCT]", "struct <name> { ... }", "struct " + structures.front().name, 1);

			return FOUND_SOMETHING;
		}
		return FOUND_NOTHING;
	}
	else if (mode == MODE_SAVE_FUNC) { // Last line, do not save anymore after this.
		if (rightBracket && equalBrackets == 0) {
			mode = MODE_DEFAULT;
			HPL_log("[DONE][FUNCTION]", "<type> <name>(<params>)", printFunction(functions.back()), 1);

			return FOUND_SOMETHING;
		}
		else {
			functions.back().code.push_back(line);

			return FOUND_SOMETHING;
		}
	}
	else if (mode == MODE_SCOPE_IGNORE_ALL) {
		if (leftBracket && equalBrackets + 1 == ifStatements.back().startingLine) {
			if (ifStatements.size() > 1)
				mode = MODE_SCOPE_IF_STATEMENT;
			else
				mode = MODE_DEFAULT;

			ifStatements.pop_back();
		}
		return FOUND_SOMETHING;
	}

	if ((leftBracket || rightBracket) && mode == MODE_SCOPE_IF_STATEMENT && !ifStatements.empty()) {
		std::vector<HPL::variable> oldVars = HPL::variables;
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

		HPL::lineCount -= ifStatements.front().code.size();

		auto& code = ifStatements.front().code;
		std::vector<std::string> whileCode;
		auto oldCount = HPL::lineCount;
		bool alreadyWent = false;
		std::vector<HPL::object> ifStatementInstruction;

		if (HPL::matches.str(1) == "while") {
			whileCode = HPL::matches.value;
			whileCode.erase(whileCode.begin());


			for (const auto& obj : whileCode) {
				HPL::variable var;
				HPL::object res;
				HPL::variable* originalVar = (HPL::variable*)std::malloc(sizeof(HPL::variable*));

				setCorrectValue(var, obj, false, &originalVar);

				if (originalVar == nullptr)
					res = HPL::object{.type = VARIABLE, .value = var.value, .objVarType = var.type};
				else
					res = HPL::object{.type = VARIABLE, .value = originalVar, .objVarType = var.type, .isVar = true};

				ifStatementInstruction.push_back(res);
			}
		}
		instructions.clear();
		int n = 0;
		int* point = nullptr;


		the_statement:
			//std::cout << instructions.size() << std::endl;

			if (!alreadyWent) {
				for (const auto& line : code) {
					HPL::lineCount++;
					HPL::interpreteLine(line);
				}
			}
			else {

				for (const auto& execute : instructions) {
					HPL::lineCount++; // this takes 13 ms from 30 ms
					// no opt - 30 ms
					// int* point - 27 ms


					switch ((int)execute.action) {
						case MATH_ADD: {
							HPL::variable* var = getPVar(execute.res);

							if (point == nullptr)
								point = &std::get<int>(var->value);

							if (var->type == "int") {
								if (execute.isVar) {
									auto& newValue = getPVar(execute.value);
									if (newValue->type == "int")
										point += std::get<int>(newValue->value);
									else
										point += std::get<int>(newValue->value);
								}
								else
									point += (int)std::get<float>(execute.value);
							}
							else {
								///
							}

							break;
						}
						default: break;
						//default: std::cout << "Baded: " << execute.action << std::endl;
					}
					//std::cout << *point << std::endl;
				}
			}

		if (!whileCode.empty()) {
			bool worked  = fixIfStatements(ifStatementInstruction);
			n++;

			if (worked && n != 1000000) {
				HPL::lineCount = oldCount;
				alreadyWent = true;
				goto the_statement;
			}
		}

		// If a global variable was edited in the function, save the changes.
		for (const auto& newV : HPL::variables) {
			bool found = false;

			for (auto& oldV : oldVars) {
				if (oldV.name == newV.name) {
					oldV.value = newV.value;
					found = true;
					break;
				}
			}

			if (!found && HPL::arg.dumpJson)
				HPL::cachedVariables.push_back(newV);
		}
		HPL::variables = oldVars;
		ifStatements.erase(ifStatements.begin());
		arg.curIndent.pop_back();

		if (!ifStatements.empty())
			mode = MODE_SCOPE_IF_STATEMENT;

		if (leftBracket || (rightBracket && ifStatements.empty()))
			return FOUND_SOMETHING;
	}
	else if (mode == MODE_SCOPE_IF_STATEMENT) {
		if (ifStatements.empty())
			return FOUND_SOMETHING;

		if (ifStatements.back().type == "invalid")
			return FOUND_SOMETHING;

		if (m.str(1) == "if" || m.str(1) == "while")
			return FOUND_NOTHING;

		ifStatements.back().code.push_back(line);

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


int HPL::checkConditions(HPL::vector& m) {
	if (m.str(1) == "if" || m.str(1) == "while") {
		bool rightBracket = false;

		if (m.str(m.size()).back() == '{') {
			rightBracket = true;
			m[m.size() - 1].pop_back();
			mode = MODE_SCOPE_IF_STATEMENT;
		}
		else
			mode = MODE_CHECK_IF_STATEMENT;

		auto params = m.value;
		params.erase(params.begin());

		ifStatements.push_back({.startingLine = equalBrackets});
		if (m.str(1) == "while")
			HPL::matches = m.value;

		//if (HPL::arg.debugAll || HPL::arg.debugLog) {
		//	std::cout << arg.curIndent << "LOG: [FOUND][IF-STATEMENT]: " << curFile << ":" << lineCount << ": if|while (<condition>): " << m.str(1) << " (" << "oldValue" << ")" << std::endl;
		//	arg.curIndent += "\t";
		//}

		bool worked = fixIfStatements(params);

		if (!worked) {
			ifStatements.back().type = "invalid";
			mode = rightBracket ? MODE_SCOPE_IGNORE_ALL : MODE_CHECK_IGNORE_ALL;

			//if (HPL::arg.debugAll || HPL::arg.debugLog) {
			//	arg.curIndent.pop_back();
			//	std::cout << arg.curIndent << "LOG: [FAILED][IF-STATEMENT]: " << curFile << ":" << lineCount << ": Condition failed, output returned false." << std::endl;
			//}
		}

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


int HPL::checkStruct(HPL::vector& m) {
	if (m.str(1) == "struct") {
		std::string name = m.str(2);

		structures.insert(structures.begin(), {name});
		mode = MODE_CHECK_STRUCT;

		if (HPL::arg.debugAll || HPL::arg.debugLog) {
			std::cout << arg.curIndent << "LOG: [CREATE][STRUCT]: " << curFile << ":" << lineCount << ": struct <name>: struct " << name << std::endl;
			arg.curIndent += "\t";
		}

		if (m.str(3) == "{") {
			mode = MODE_SAVE_STRUCT;
			return FOUND_SOMETHING;
		}
	}

	return FOUND_NOTHING;
}



int HPL::checkFunctions(HPL::vector& m) {
	// Match <return type> <name>(<params>) {
	//HPL::vector funcMatches;
	//if (parseFunction(line, funcMatches)) {
	//	if (funcMatches.size() == 3) {
	//		function func = {funcMatches.str(1), funcMatches.str(2)};
	//		std::vector<std::string> params = split(funcMatches.str(3), ",", "(){}\"\"");
//
	//		for (const auto& v : params) {
	//			HPL::vector varMatches;
	//			if (parseVariable(line, varMatches)) {
	//				variable var = {varMatches.str(1), varMatches.str(2)};
//
	//				if (varMatches.str(3).empty())
	//					func.minParamCount++;
	//				else
	//					var.value = unstringify(varMatches.str(3));
//
	//				func.params.push_back(var);
	//			}
	//		}
	//		func.file = HPL::curFile;
	//		func.startingLine = HPL::lineCount;
//
	//		functions.push_back(func);
	//		mode = MODE_CHECK_FUNC;
//
	//		if (HPL::arg.debugAll || HPL::arg.debugLog) {
	//			std::cout << arg.curIndent << "LOG: [CREATE][FUNCTION]: " << curFile << ":" << lineCount << ": <type> <name>(<params>): " << printFunction(func) << std::endl;
	//			arg.curIndent += "\t";
	//		}
//
	//		if (find(line, "{"))
	//			mode = MODE_SAVE_FUNC;
	//	}
	//	else if (funcMatches.size() == 2) {
	//		function f; HPL::variable res;
//
	//		if (HPL::arg.debugAll || HPL::arg.debugLog)
	//			std::cout << arg.curIndent << "LOG: [USE][FUNCTION]: " << curFile << ":" << lineCount << ": <name>(<params>): " << funcMatches.str(1) << "(" << funcMatches.str(2) << ")" << std::endl;
//
	//		return executeFunction(funcMatches.str(1), funcMatches.str(2), f, res);
	//	}
//
	//	return FOUND_SOMETHING;
	//}
	//else if (parseReturn(line, HPL::matches)) {
	//	setCorrectValue(functionOutput, matches.str(1), false);
//
	//	if (HPL::arg.debugAll || HPL::arg.debugLog)
	//		std::cout << arg.curIndent << "LOG: [FOUND][RETURN]: " << curFile << ":" << lineCount << ": return <value> (<type>): return " << xToStr(functionOutput.value) << " (" << functionOutput.type << ")" << std::endl;
//
	//	return FOUND_SOMETHING;
	//}
//
	return FOUND_NOTHING;
}


int HPL::checkVariables(HPL::vector& m) {
	// 1. int test = 3
	//		int - <match 1>
	//		test - <match 2>
	//		= - <match 3>
	//		3 - <match 4>
	// 2. test = 3
	// 		test - <match 1>
	//		= - <match 2>
	//		3 - <match 3>
	// 3. test += 33
	//		test - <match 1>
	//		+= - <match 2>
	//		33 - <match 3>
	// 4. test++
	//		test - <match 1>
	//		++ - <match 2>
	// 5. int test
	//		int - <match 1>
	//		test - <match 2>
		HPL::variable var;
		OBJ_ACTION type = NOTHING;

		switch (m.size()) {
			case 4: { // New variable with value.
				var = HPL::variable{.type = m.str(1), .name = m.str(2), .value = m.str(4)};
				type = SAVE;
				break;
			}
			case 3: { // Edit pre-existing variable.
				var = HPL::variable{.type = m.str(2), .name = m.str(1), .value = m.str(3)};
				type = EDIT;

				if (m.str(2).size() == 2)
					type = MATH;

				break;
			}
			case 2: { // Math/new variable with no value.
				if (operatorList.find(m.str(2)) != operatorList.end()) {// ++/--
					var = HPL::variable{.type = m.str(2), .name = m.str(1)};
					type = MATH;
				}
				else {
					var = HPL::variable{.type = m.str(1), .name = m.str(2)};
					type = SAVE;
				}
				break;
			}
		}
		if (type == NOTHING)
			return FOUND_NOTHING;

		std::vector<std::string> listOfVarNames = {var.name};
		std::vector<std::string> listOfVarValues = {xToStr(var.value)};

		instructions.push_back({});
		auto& latestInstruction = instructions.back();
		HPL::variable* originalVar = (HPL::variable*)std::malloc(sizeof(HPL::variable*));


		for (int listOfVarIndex = 0; listOfVarIndex < listOfVarNames.size(); listOfVarIndex++) {
			switch ((int)type) {
				case SAVE: {
					if (listOfVarIndex < listOfVarValues.size())
						var.value = removeFrontAndBackSpaces(listOfVarValues[listOfVarIndex]);

					auto& value = getStr(var);

					if (value.empty())
						var.reset_value();

					if (!typeIsValid(var.type)) // Type isn't cored or a structure.
						throwError(true, "Type '%s' doesn't exist (Cannot init a variable without valid type).", var.type.c_str());

					if (!setCorrectValue(var, value, true, &originalVar) && !value.empty())
						throwError(true, "Variable '%s' doesn't exist (Cannot copy value from something that doesn't exist).", value.c_str());

					auto pointerToOutput = &variables;


					if (mode == MODE_SAVE_STRUCT) // Save variables inside a struct.
						pointerToOutput = &structures.begin()->value;

					pointerToOutput->push_back(var);

					if (var.type == "scope")
						scopeIndex = pointerToOutput->size() - 1;

					if (originalVar != nullptr)
						latestInstruction = HPL::object{.action = type, .type = VARIABLE, .res = &pointerToOutput->back(), .value = originalVar, .objVarType = var.type, .isVar = true};
					else
						latestInstruction = HPL::object{.action = type, .type = VARIABLE, .res = &pointerToOutput->back(), .value = var.value, .objVarType = var.type};

					HPL_log("[CREATE][VARIABLE]", "<type> <variable> = [value]", printVar(var));

					break;
				}
				case EDIT: {
					HPL::variable* existingVar = getVarFromName(var.name);

					if (existingVar == nullptr)
						HPL::throwError(true, "Variable '%s' doesn't exist (Can't edit a variable that doesn't exist)", var.name.c_str());

					setCorrectValue(*existingVar, getStr(var), true, &originalVar);

					if (originalVar != nullptr)
						latestInstruction = HPL::object{.action = type, .type = VARIABLE, .res = existingVar, .value = originalVar, .isVar = true};
					else
						latestInstruction = HPL::object{.action = type, .type = VARIABLE, .res = existingVar, .value = var.value};

					HPL_log("[EDIT][VARIABLE]", "<type> <variable> = <value>", printVar(*existingVar));

					break;
				}
				case MATH: {
					variable* existingVar = getVarFromName(var.name); //vars[var.name];
					float res;

					if (existingVar == nullptr)
						throwError(true, "Cannot perform any math operations to this variable (Variable '%s' does not exist).", m.str(1).c_str());

					if (!(existingVar->type == "int" || existingVar->type == "float" || existingVar->type == "string"))
						HPL::throwError(true, "Cannot perform any math operations to a non-int variable (Variable '%s' isn't int/float/string-typed, can't operate to a '%s' type).", existingVar->name.c_str(), existingVar->type.c_str());

					setCorrectValue(var, getStr(var), false, &originalVar);

					if (existingVar->type == "string" && var.type == "+=") {
						if (var.type == "struct" || var.type == "scope")
							HPL::throwError(true, "Cannot append a %s type to a string (Value '%s' is a %s-type).", var.type.c_str(), xToStr(var.value).c_str(), var.type.c_str());


						existingVar->value = xToStr(existingVar->value) + xToStr(var.value);

						if (originalVar != nullptr)
							latestInstruction = HPL::object{.action = type, .type = VARIABLE, .res = existingVar, .value = originalVar};
						else
							latestInstruction = HPL::object{.action = type, .type = VARIABLE, .res = existingVar, .value = var.value};

						return FOUND_SOMETHING;
					}

					else {
						float dec1 = 0.0, dec2 = 0.0;


						if (existingVar->type == "int")
							dec1 = xToType<int>(existingVar->value);
						else
							dec1 = xToType<float>(existingVar->value);

						if (!xToStr(var.value).empty())
							dec2 = xToType<float>(var.value);

						switch (operatorList[var.type]) {
							case plus_plus: res = dec1 + 1; type = MATH_ADD; break;
							case minus_minus: res = dec1 - 1; type = MATH_REMOVE; break;
							case plus_equal: res = dec1 + dec2; type = MATH_ADD; break;
							case minus_equal: res = dec1 - dec2; type = MATH_REMOVE; break;
							case multiply_equal: res = dec1 * dec2; type = MATH_MULTIPLY; break;
							case divide_equal: res = dec1 / dec2; type = MATH_DIVIDE; break;
							case module_equal: res = fmod(dec1, dec2); type = MATH_MODULE; break;
							default: HPL::throwError(true, "Error"); break;
						}

						if (existingVar->type == "int")
							existingVar->value = (int)res;
						else if (existingVar->type == "float")
							existingVar->value = (float)res;

						var.value = (float)dec2;
					}

					if (originalVar != nullptr)
						latestInstruction = HPL::object{.action = type, .type = VARIABLE, .res = existingVar, .value = originalVar, .objVarType = existingVar->type, .isVar = true};
					else
						latestInstruction = HPL::object{.action = type, .type = VARIABLE, .res = existingVar, .value = var.value, .objVarType = existingVar->type};

					//if (HPL::arg.debugAll || HPL::arg.debugLog) {
					//	std::cout << arg.curIndent << "LOG: [MATH][VARIABLE]: " << curFile << ":" << lineCount << ": <variable> <operator> [value]: " << existingVar->name << " " << m.str(2);
					//	if (m.str(2) != "--" || m.str(2) != "++")
					//		std::cout << " " << m.str(3);
					//	std::cout << std::endl;
					//}
					break;
				}
			}


			return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


void HPL::debugMode() {
	std::cout << colorText("============ DEBUG INFORMATION ============\n", HPL::OUTPUT_PURPLE);

	std::cout << colorText("Variables:\n", OUTPUT_CYAN, true);
	debugPrintVar(variables);
	std::cout << colorText("\nStructures:\n", OUTPUT_CYAN, true);
	debugPrintStruct(structures);
	std::cout << colorText("Functions:\n", OUTPUT_CYAN, true);
	debugPrintFunc(functions);
}


void HPL::debugPrintVar(std::vector<variable> vars, std::string tabs/* = "	"*/, std::string end/* = "\n"*/) {
	for (int index = 0; index < vars.size(); index++) {  // Regular variables
		auto& var = vars[index];

		RETURN_OUTPUT clr = OUTPUT_PURPLE;

		if (!coreTyped(var.type))
			clr = OUTPUT_NOTHING;

		std::cout << tabs << colorText(var.type, clr) << " " << var.name;
		if (var.has_value())
			std::cout << " = ";
		print(var, "");

		if (index + 1 < vars.size())
			std::cout << end;
	}
}


void HPL::debugPrintStruct(std::vector<structure> structList, std::string indent/* = "\t"*/) {
	for (const auto& s : structures) {
		std::cout << indent << colorText("struct", HPL::OUTPUT_PURPLE) << " " << colorText(s.name, HPL::OUTPUT_YELLOW) << colorText(" {", HPL::OUTPUT_GREEN) << std::endl;
		debugPrintVar(s.value, indent + indent);
		std::cout << colorText("\n" + indent + "}\n", HPL::OUTPUT_GREEN);
	}
}


void HPL::debugPrintFunc(std::vector<function> func, std::string indent/* = "\t"*/) {
	for (const auto& f : functions) {
		RETURN_OUTPUT clr = OUTPUT_PURPLE;

		if (f.type == "scope")
			clr = OUTPUT_BLUE;
		else if (!coreTyped(f.type))
			clr = OUTPUT_RED;

		std::cout << indent << colorText(f.type, clr) << " " << f.name << "("; debugPrintVar(f.params, "", ", "); std::cout << ") {\n";

		for (auto c : f.code)
			std::cout << indent << c << std::endl;

		std::cout << indent << "}" << std::endl;
	}
}


void HPL::resetRuntimeInfo() {
	curFile.clear();
	line.clear();
	functionOutput.reset_value();

	lineCount = 0;
	mode = MODE_DEFAULT;
	matches.clear();
}


void HPL::throwError(bool sendRuntimeError, std::string text, ...) {
	va_list valist;
	va_start(valist, text);
	bool colorMode = false;

	#if defined(WINDOWS)
	std::string msg;
	std::cout << colorText("Error at ", OUTPUT_RED) << "'" << colorText(curFile + ":" + std::to_string(lineCount), OUTPUT_YELLOW) << "'" << colorText(": ", OUTPUT_RED);
	#else
	std::string msg = colorText("Error at ", OUTPUT_RED) + "'" + colorText(curFile + ":" + std::to_string(lineCount), OUTPUT_YELLOW) + "'" + colorText(": ", OUTPUT_RED);
	#endif

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
					if (colorMode) {
						#if defined(WINDOWS)
						std::cout << msg << colorText(va_arg(valist, const char*), OUTPUT_YELLOW);
						msg.clear();
						#else
						msg += colorText(va_arg(valist, const char*), OUTPUT_YELLOW);
						#endif
					}
					else {
						#if defined(WINDOWS)
						std::cout << msg << va_arg(valist, const char*);
						msg.clear();
						#else
						msg += va_arg(valist, const char*);
						#endif
					}

					break;
				case 'c':
					if (colorMode) {
						#if defined(WINDOWS)
						std::cout << msg << colorText(va_arg(valist, char*), OUTPUT_YELLOW);
						msg.clear();
						#else
						msg += colorText(va_arg(valist, char), OUTPUT_YELLOW);
						#endif
					}
					else {
						#if defined(WINDOWS)
						std::cout << msg << va_arg(valist, char*);
						msg.clear();
						#else
						msg += va_arg(valist, const char*);
						#endif
					}

					break;
				case 'd':
				case 'i':
					num = va_arg(valist, int);

					if (colorMode) {
						#if defined(WINDOWS)
						std::cout << msg << colorText(std::to_string(num), OUTPUT_YELLOW);
						msg.clear();
						#else
						msg += colorText(std::to_string(num), OUTPUT_YELLOW);
						#endif
					}
					else {
						#if defined(WINDOWS)
						std::cout << msg << num;
						msg.clear();
						#else
						msg += std::to_string(num);
						#endif
					}

					break;

				default:
					break;
			}
			i++;
		}
		else msg += x;
	}
	va_end(valist);

	if (sendRuntimeError) {
		#if defined(WINDOWS) // Windows is fucking stupid with colored terminals.
		std::cout << msg << std::endl;

		if ((HPL::arg.debugAll || HPL::arg.debugPrint) && sendRuntimeError)
			debugMode();
		#else
		std::cout << "\x1B[0m" << msg << std::endl;
		#endif

		throw std::runtime_error("");
	}
	else {
		#if !defined(WINDOWS) // Windows is fucking stupid with colored terminals.
		if ((HPL::arg.debugAll || HPL::arg.debugPrint) && sendRuntimeError)
			debugMode();
		#endif

		std::cout << "\x1B[0m" << msg << std::endl;

		#if defined(WINDOWS) // Windows is fucking stupid with colored terminals.
		if ((HPL::arg.debugAll || HPL::arg.debugPrint) && sendRuntimeError)
			debugMode();
		#endif
	}
}


std::string HPL::getModeName() {
	switch (mode) {
		case MODE_DEFAULT: return "MODE_DEFAULT";

		case MODE_SAVE_STRUCT: return "MODE_SAVE_STRUCT";
		case MODE_SAVE_FUNC: return "MODE_SAVE_FUNC";
		case MODE_SAVE_SCOPE: return "MODE_SAVE_SCOPE";
		case MODE_SCOPE_IF_STATEMENT: return "MODE_SCOPE_IF_STATEMENT";
		case MODE_SCOPE_IGNORE_ALL: return "MODE_SCOPE_IGNORE_ALL";


		case MODE_CHECK_STRUCT: return "MODE_CHECK_STRUCT";
		case MODE_CHECK_FUNC: return "MODE_CHECK_FUNC";
		case MODE_CHECK_SCOPE: return "MODE_CHECK_SCOPE";
		case MODE_CHECK_IF_STATEMENT: return "MODE_CHECK_IF_STATEMENT";
		case MODE_CHECK_IGNORE_ALL: return "MODE_CHECK_IGNORE_ALL";
		default: return "N/A";
	}
}


namespace HPL {
	// Inteperter configs.
	HPL::configArgs arg;

	// Interpreter rules runtime.
	std::string curFile;
	std::string line;
	int lineCount = 0;
	int mode = 0;
	HPL::vector matches;
	int equalBrackets = 0;
	int scopeIndex = -1;
	int oldMode = -1;
	std::vector<HPL::object> objects;

	// Defnitions that are saved in memory.
	std::vector<variable> variables = {{"bool", "HPL_SCOPE_MODE", false}};
	std::vector<structure> structures;
	std::vector<function> functions;
	std::vector<function> ifStatements;

	std::vector<variable> cachedVariables;

	HPL::variable functionOutput;
	std::vector<HPL::object> instructions;
}