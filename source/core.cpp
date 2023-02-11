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
#include <interpreter.hpp>
#include <core.hpp>
#include <helper.hpp>
#include <functions.hpp>

#include <iostream>


std::vector<std::string> coreTypes = {
	"string", // Works just like std::string.
	"int",    // Regular int.
	"float",  // Regular float.
	"bool",   // Regular bloat.
	"scope",  // A scope variable, meaning HOI4 code can be executed inside of it.
	"auto"    // Generic type.
};

enum coreFunctionsEnum {
	f_print, f_func_str, f_func_bool, f_func_int, f_func_float, f_createFolder, f_removeFolder, f_createFile, f_readFile, f_writeFile, f_writeToLine, f_writeToMultipleLines, f_removeFile, f_copyFile, f_writeLocalisation, f_convertToDds, f_pathExists, f_getFilenameFromPath, f_find, f_replaceAll, f_len, f_HPL_throwError
};
static std::map<std::string, coreFunctionsEnum> coreFunctionsList = {
	// Misc.
	{"print", f_print},
	{"str", f_func_str},
	{"bool", f_func_bool},
	{"int", f_func_int},
	{"float", f_func_float},
	// Folders.
	{"createFolder", f_createFolder},
	{"removeFolder", f_removeFolder},
	// Files.
	{"createFile", f_createFile},
	{"readFile", f_readFile},
	{"writeFile", f_writeFile},
	{"writeToLine", f_writeToLine},
	{"writeToMultipleLines", f_writeToMultipleLines},
	{"removeFile", f_removeFile},
	{"copyFile", f_copyFile},
	// Localisation.
	{"writeLocalisation", f_writeLocalisation},
	{"readLocalisation", f_writeLocalisation},
	// Image related.
	{"convertToDds", f_convertToDds},
	// Path related.
	{"pathExists", f_pathExists},
	{"getFilenameFromPath", f_getFilenameFromPath},
	// General string functions.
	{"find", f_find},
	{"replaceAll", f_replaceAll},
	{"len", f_len}
};

std::map<std::string, operatorsEnum> operatorList = {
	{"=", equal},
    {"+=", plus_equal}, {"-=", minus_equal}, {"*=", multiply_equal}, {"/=", divide_equal}, {"%=", module_equal},
    {"++", plus_plus}, {"--", minus_minus},

	{"==", equal_equal}, {"!=", not_equal},
	{">", greater_than}, {"<", lesser_than}, {">=", equal_greater}, {"<=", equal_lesser},
	{"&&", and_and}, {"||", or_or}
};

bool foundFunction = false; // If we found the function.
int startOrgAt = 0; // At which index we should start the organization. NOTE: A possible bug exists, where the first few params are in order, but then afterwards the defines out of order arguments have the same first few param names, ending in shenanigans. Needs fixing.
HPL::function globalFunction; // The found function.


int executeFunction(std::string name, std::string info, HPL::function& function, HPL::variable& output, bool dontCheck/* = false*/) {
	// General params.
	std::vector<std::string> values = split(info, ",", "(){}\"\"");
	std::vector<HPL::variable> params;
	bool organizeParams = false; // If we have to organize params.

	if (HPL::arg.debugLog || HPL::arg.debugAll)
		HPL::arg.curIndent += "\t";

	for (auto& p : values) {
		HPL::variable var;

		// Removing the spaces and quotes from match.
		std::string oldMatch = removeFrontAndBackSpaces(p); // In case the match is actually a string.
		p = unstringify(oldMatch);

		// Out of order initialization settings.
		bool outOfOrder = false; // Is out of order.
		std::string outOfOrderParam; // The name of the param.


		// Found an out of order argument.
		if (find(oldMatch, "=") && !isStr(oldMatch)) {
			// Find the param and true value.
			useRegex(oldMatch, R"(\s*(\w*)\s*\=\s*(f?\".*\"|\{.*\}|\w*\(.*\)|[\d\s\+\-\*\/\.]+[^\w]*|[^\s]*)\s*.*)");

			if (!HPL::matches.empty()) { // If we found the param and value.
				outOfOrder = true;
				organizeParams = true;
				outOfOrderParam = HPL::matches.str(1);

				oldMatch = HPL::matches.str(2);
				p = unstringify(oldMatch);
			}
			else
				HPL::throwError(true, "Invalid syntax");
		}

		if (!outOfOrder && organizeParams) { // If the input didn't set the param name, even though we're in out of order init mode.
			HPL::throwError(true, "All out of order argument initializations must be accompanied with the name of the param (format is '%s', not just '%s')", "<param> = <value>", p.c_str());
		}

		// Checks if the parameter is just a function.
		if (useRegex(p, R"(^\s*([^\s\(]+)\((.*)\)\s*$)")) {
			// If so, get the name and params of said parameter.
			std::vector<HPL::vector> funcValues, oldFuncValues;
			std::string str = p;
			while (true) {
				// Check if the param isn't just a function.
				useRegex(str, R"(\s*([^\s\(]+)\((.*)\)\s*)");
				if (!HPL::matches.str(2).empty() || !str.empty()) {
					funcValues.insert(funcValues.begin(), HPL::matches);
					str = HPL::matches.str(2);
				}
				else
					break;
			}

			oldFuncValues = funcValues;

			for (int i = 0; i < funcValues.size(); i++) {
				// Since the param DOES have functions inside, we have to get that functions' output.
				auto list = split(funcValues[i].str(2), ",", "(){}\"\"");
				assignFuncReturnToVar(&var, funcValues[i].str(1), funcValues[i].str(2), true);

				if ((i + 1) < funcValues.size()) { // If there are more functions inside the param.
					auto& noodles = funcValues[i + 1].value[1]; // Get the next function.

					auto msg = xToStr(var.value); // Get the return from the current function.
					if (!isInt(p) && p != "true" && p != "false")
						msg = '\"' + msg + '\"'; // Since it's a string, we have to add quotes

					// Since the next function's param is gonna be "{currentFunctionName}({currentFunctionParams})",
					// we have to replace that with the current function's output (aka msg).
					noodles = replaceOnce(noodles, oldFuncValues[i].str(1) + "(" + oldFuncValues[i].str(2) + ")", msg);
				}
			}
		}
		else {
			bool res = setCorrectValue(var, oldMatch, false);

			if (!res)
				HPL::throwError(true, "Variable '%s' doesn't exist (Cannot use a variable that doesn't exist).", p.c_str());
		}

		if (organizeParams) // Set the 'var.name' to the param's name, since 'var.name' isn't needed for functions.
			var.name = outOfOrderParam;
		else
			startOrgAt++;

		params.push_back(var);

		if (HPL::arg.debugAll || HPL::arg.debugLog)
			std::cout << HPL::arg.curIndent << "LOG: [FIND][PARAM]: " << HPL::curFile << ":" << HPL::lineCount << ": <type> <name> = <value>: " << printVar(var) << std::endl;
	}
	foundFunction = false;
	globalFunction.name = name;

	if (organizeParams)
		params.push_back(HPL::variable{.type = "IS_OOO"});

	output.value = coreFunctions(params);

	if (foundFunction) {
		function = globalFunction;
		output.type = function.type;
		globalFunction = {};

		if (HPL::arg.debugAll || HPL::arg.debugLog) {
			std::cout << HPL::arg.curIndent << "LOG: [FIND][FUNCTION](0): " << HPL::curFile << ":" << HPL::lineCount << ": <type> <name> | <output> (<output's type>): " << function.type << " " << function.name << " | " << xToStr(output.value) << " (" << output.type << ")" << std::endl;
			HPL::arg.curIndent.pop_back();
		}

		return FOUND_SOMETHING;
	}

	// Non-core functions
	for (const auto& func : HPL::functions) {
		if (useFunction(func, params)) {
			// Save the info and reset it all so that the interpreter doesn't spout random info.
			std::string oldCurFile = HPL::curFile;
			auto oldLineCount = HPL::lineCount;
			auto oldVars = HPL::variables;
			auto oldMode = HPL::mode;
			auto oldEqualBrackets = HPL::equalBrackets;

			HPL::resetRuntimeInfo();
			HPL::curFile = func.file;
			HPL::lineCount = func.startingLine;

			for (int i = 0; i < func.params.size(); i++) {
				auto var = func.params[i];

				if (i < params.size())
					var.value = params[i].value;

				HPL::variables.push_back(var);

				if (HPL::arg.dumpJson)
					HPL::cachedVariables.push_back(var);
			}

			for (const auto& line : func.code) {
				HPL::lineCount++;
				HPL::interpreteLine(line);

				if (HPL::functionOutput.has_value()) // If the function returned something, exit.
					break;
				if (!HPL::arg.interprete)
					return FOUND_NOTHING;
			}

			// Set the output value and type.
			output = HPL::functionOutput;
			// Reset the saved output value and type.
			HPL::functionOutput.reset_all();


			// If a global variable was edited in the function, save the changes.
			for (auto& oldV : oldVars) {
				for (auto& newV : HPL::variables) {
					if (oldV.name == newV.name) {
						oldV.value = newV.value;
						break;
					}
				}
			}

			// Reset everything back to normal.
			HPL::variables = oldVars;
			HPL::curFile = oldCurFile;
			HPL::lineCount = oldLineCount;
			HPL::mode = oldMode;
			HPL::equalBrackets = oldEqualBrackets;

			globalFunction = func;
			foundFunction = true;

			if (HPL::arg.debugAll || HPL::arg.debugLog) {
				std::cout << HPL::arg.curIndent << "LOG: [FIND][FUNCTION](1): " << HPL::curFile << ":" << HPL::lineCount << ": <type> <name> | <output> (<output's type>): " << func.type << " " << func.name << " | " << xToStr(output.value) << " (" << output.type << ")" << std::endl;
				HPL::arg.curIndent.pop_back();
			}

			function = globalFunction;
			globalFunction = {};

			return FOUND_SOMETHING;
		}
	}

	if (!dontCheck)
		HPL::throwError(true, "Function '%s' doesn't exist (Either the function is defined nowhere or it's a typo)", name.c_str());

	return FOUND_NOTHING;
}


bool useFunction(HPL::function func, std::vector<HPL::variable>& sentUserParams) {
	if (func.name != globalFunction.name)
		return false;

	bool organize = false;
	int size = sentUserParams.size();

	if (!sentUserParams.empty() && sentUserParams.back().type == "IS_OOO") {
		size--;
		organize = true;
	}

	if (func.name == globalFunction.name && func.params.size() < size)
		HPL::throwError(true, "Too many parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", size, func.name.c_str(), func.params.size());
	if (func.name == globalFunction.name && func.minParamCount > size)
		HPL::throwError(true, "Too few parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", size, func.name.c_str(), func.minParamCount);


	if (organize) { // If there are any out of order arguments.
		HPL::function* outOfOrderFunc = &func; // The function.
		sentUserParams.pop_back();

		std::vector<HPL::variable> organizedParams = outOfOrderFunc->params;
		std::string buf;
		int num = 0;

		for (const auto& outOfOrderParam : sentUserParams) { // Iterate through user inputted params.
			bool paramExist = false;
			int paramIndex = 0;
			num++;

			if (!find(buf, outOfOrderParam.name)) // Checks for any duplicates.
				buf += outOfOrderParam.name;
			else
				HPL::throwError(true, "Cannot initialize the same param multiple times (param '%s' was initialized multiple times).", outOfOrderParam.name.c_str());

			if (num - 1 < startOrgAt && startOrgAt != 0) { // Ignore the in order initializations.
				organizedParams.push_back(sentUserParams[num - 1]);
				continue;
			}

			for (auto& functionParam : outOfOrderFunc->params) { // Iterate through the *actual* params.
				if (functionParam.name == outOfOrderParam.name) { // Found it.
					organizedParams[paramIndex] = outOfOrderParam; // Fix the param.
					paramExist = true;
					break;
				}
				paramIndex++;
			}

			if (!paramExist) // Didn't find an existing param.
				HPL::throwError(true, "Param '%s' doesn't exist (Must use a proper param name that exists in function '%s')", outOfOrderParam.name.c_str(), outOfOrderFunc->name.c_str());
		}

		sentUserParams = organizedParams;

		if (HPL::arg.debugAll || HPL::arg.debugLog)
			std::cout << HPL::arg.curIndent << HPL::colorText("LOG: [FIX][FUNCTION-OOO]: ", HPL::OUTPUT_BLUE) << HPL::curFile << ":" << HPL::lineCount << ": Fixed the out of order initializations." << std::endl;
	}

	startOrgAt = 0; // Reset out of order initialization organization.

	for (int i = 0; i < func.params.size(); i++) {
		if ((i + 1) <= sentUserParams.size()) {
			if (!coreTyped(func.params[i].type)) {
				auto& userParams = getVars(sentUserParams[i].value);

				HPL::structure* _struct = getStructFromName(func.params[i].type);

				if (_struct->value.size() < userParams.size())
					HPL::throwError(true, "Too many members were provided (you provided '%i' arguments when struct '%s' takes at most '%i' members)", userParams.size(), _struct->name.c_str(), userParams.size());

				for (int x = 0; x < userParams.size(); x++) {
					auto& member = _struct->value[x];

					if (member.type != userParams[x].type && member.type != "auto")
						HPL::throwError(true, "Members' types do not match ('%s' is %s-typed, while '%s' is %s-typed)", member.name.c_str(), member.type.c_str(), userParams[x].name.c_str(), userParams[x].type.c_str());
				}
				sentUserParams[i].type = func.params[i].type; // why?
			}

			if (func.params[i].type != sentUserParams[i].type && func.params[i].type != "auto")
				HPL::throwError(true, "Cannot input a '%s' type to a %s-only parameter (param '%s' is %s-only)", sentUserParams[i].type.c_str(), func.params[i].type.c_str(), func.params[i].name.c_str(), func.params[i].type.c_str());
		}

		if (func.params[i].has_value() && (i + 1) > sentUserParams.size()) {// If the function has default parameters that weren't covered
			auto var = func.params[i];

			std::string value = xToStr(var.value);
			if (value == "{}")
				setCorrectValue(var, xToStr(var.value), true);

			sentUserParams.push_back(var);
		}
	}

	foundFunction = true;
	globalFunction.type = func.type;
	globalFunction.minParamCount = func.minParamCount;

	return true;
}


int assignFuncReturnToVar(HPL::variable* existingVar, std::string funcName, std::string funcParam, bool dontCheck/* = false*/) {
	HPL::function func;
	HPL::variable output;
	executeFunction(funcName, funcParam, func, output, dontCheck);

	if (output.has_value()) {
		if (output.type == "struct") {
			HPL::structure* s = getStructFromName(func.type);
			if (s != nullptr) {
				if (existingVar->type != func.type) {
					HPL::throwError(true, "later");
				}
				else {
					auto result = getVars(output.value);
					existingVar->value = result;
				}

				return FOUND_SOMETHING;
			}
		}

		std::string value = xToStr(output.value);

		if (existingVar->type == "string" || existingVar->type == "scope")
			existingVar->value = value;
		else if (existingVar->type == "int")
			existingVar->value = xToType<int>(value);
		else if (existingVar->type == "bool")
			existingVar->value = stringToBool(value);
		else if (existingVar->type == "float")
			existingVar->value = xToType<float>(value);
		else {
			if (func.type == "string" || func.type == "scope")
				existingVar->value = getStr(output);
			else if (func.type == "int")
				existingVar->value = getInt(output);
			else if (func.type == "bool")
				existingVar->value = getBool(output);
			else if (func.type == "float")
				existingVar->value = getFloat(output);

			existingVar->type = func.type;
		}
	}
	else {
		existingVar->reset_value();

		return FOUND_NOTHING;
	}

	return FOUND_SOMETHING;
}


allowedTypes coreFunctions(std::vector<HPL::variable>& params) {
	if (coreFunctionsList.find(globalFunction.name) == coreFunctionsList.end())
		return {};

	switch(coreFunctionsList[globalFunction.name]) {
		case f_print: useFunction({.type = "void", .name = "print", .params = {{"auto", "msg"}, {"string", "ending", "\n"}}, .minParamCount = 1}, params); print(params[0], getStr(params[1])); break;
		case f_func_str: useFunction({.type = "string", .name = "str", .params = {{"auto", "value"}}, .minParamCount = 1}, params); return func_str(params[0]);
		case f_func_int: useFunction({.type = "int", .name = "int", .params = {{"auto", "value"}}, .minParamCount = 1}, params); return func_int(params[0]);
		case f_func_bool: useFunction({.type = "bool", .name = "bool", .params = {{"auto", "value"}}, .minParamCount = 1}, params); return func_bool(params[0]);
		case f_func_float: useFunction({.type = "float", .name = "float", .params = {{"auto", "value"}}, .minParamCount = 1}, params); return func_float(params[0]);

		case f_createFolder: useFunction({.type = "int", .name = "createFolder", .params = {{"string", "path"}}, .minParamCount = 1}, params); return createFolder(getStr(params[0]));
		case f_removeFolder: useFunction({.type = "int", .name = "removeFolder", .params = {{"string", "path"}}, .minParamCount = 1}, params); return removeFolder(getStr(params[0]));

		case f_createFile: useFunction({.type = "int", .name = "createFile", .params = {{"string", "path"}, {"string", "content", ""}, {"bool", "useUtf8BOM", false}}, .minParamCount = 1}, params); return createFile(getStr(params[0]), getStr(params[1]), getBool(params[2]));
		case f_readFile: useFunction({.type = "int", .name = "readFile", .params = {{"string", "path"}}, .minParamCount = 1}, params); return readFile(getStr(params[0]));
		case f_writeFile: useFunction({.type = "int", .name = "writeFile",  .params = {{"string", "path"}, {"string", "content"}, {"string", "mode", "w"}}, .minParamCount = 2}, params); return writeFile(getStr(params[0]), getStr(params[1]), getStr(params[2]));
		case f_removeFile: useFunction({.type = "int", .name = "removeFile", .params = {{"string", "path"}}, .minParamCount = 1}, params); return removeFile(getStr(params[0]));
		case f_copyFile: useFunction({.type = "int", .name = "copyFile", .params = {{"string", "source"}, {"string", "output"}}, .minParamCount = 2}, params); return copyFile(getStr(params[0]), getStr(params[1]));

		case f_writeToLine: useFunction({.type = "int", .name = "writeToLine", .params = {{"string", "path"}, {"int", "line"}, {"string", "content"}, {"string", "mode", "w"}}, .minParamCount = 3}, params); return writeToLine(getStr(params[0]), getInt(params[1]), getStr(params[2]), getStr(params[3]));
		case f_writeToMultipleLines: useFunction({.type = "int", .name = "writeToMultipleLines", .params = {{"string", "path"}, {"int", "lineBegin"}, {"int", "lineEnd"}, {"string", "content"}, {"string", "mode", "w"}}, .minParamCount = 4}, params); return writeToMultipleLines(getStr(params[0]), getInt(params[1]), getInt(params[2]), getStr(params[3]), getStr(params[4]));


		case f_writeLocalisation: useFunction({.type = "int", .name = "writeLocalisation", .params = {{"string", "file"}, {"string", "name"}, {"string", "description"}}, .minParamCount = 3}, params); return writeLocalisation(getStr(params[0]), getStr(params[1]), getStr(params[2]));
		//case f_readLocalisation:

		case f_convertToDds: useFunction({.type = "int", .name = "convertToDds", .params = {{"string", "input"}, {"string", "output"}}, .minParamCount = 2}, params); return convertToDds(getStr(params[0]), getStr(params[1]));

		case f_getFilenameFromPath: useFunction({.type = "string", .name = "getFilenameFromPath", .params = {{"string", "path"}}, .minParamCount = 1}, params); return getFilenameFromPath(getStr(params[0]));
		case f_pathExists: useFunction({.type = "bool", .name = "pathExists", .params = {{"string", "path"}}, .minParamCount = 1}, params); return pathExists(getStr(params[0]));

		case f_find: useFunction({.type = "bool", .name = "find", .params = {{"string", "line"}, {"string", "input"}}, .minParamCount = 2}, params); return find(getStr(params[0]), getStr(params[1]));
		case f_replaceAll: useFunction({.type = "string", .name = "replaceAll", .params = {{"string", "str"}, {"string", "oldString"}, {"string", "newString"}}, .minParamCount = 3}, params); return replaceAll(getStr(params[0]), getStr(params[1]), getStr(params[2]));
		case f_len: useFunction({.type = "int", .name = "len", .params = {{"auto", "value"}}, .minParamCount = 1}, params); return len(params[0]);

		case f_HPL_throwError: useFunction({.type = "void", .name = "HPL_throwError", .params = {{"string", "messsage"}}, .minParamCount = 1}, params); HPL::throwError(true, getStr(params[0])); break;

		default: break;
    }
	return {};
}