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
std::vector<std::string> coreFunctionList = {
	// Misc.
	"print",
	"str",
	"bool",
	"int",
	"float",
	// Folders.
	"createFolder",
	"removeFolder",
	// Files.
	"createFile",
	"readFile",
	"writeFile",
	"writeToLine",
	"writeToMultipleLines",
	"removeFile"
	"copyFile",
	// Localisation.
	"writeLocalisation",
	// Image related.
	"convertToDds",
	// Path related.
	"pathExists",
	"getFilenameFromPath",
	// General string functions.
	"find",
	"replaceAll"
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
		HPL::variable var = {"NO_TYPE", "NO_NAME"};

		// Removing the spaces and quotes from match.
		std::string oldMatch = unstringify(p, false, ' '); // In case the match is actually a string.
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
				auto list = split(funcValues[i].str(2), ",", "()\"\"");
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
			bool res = setCorrectValue(var, oldMatch);

			if (!res)
				HPL::throwError(true, "Variable '%s' doesn't exist (Cannot use a variable that doesn't exist).", p.c_str());
		}

		if (organizeParams) // Set the 'var.name' to the param's name, since 'var.name' isn't needed for functions.
			var.name = outOfOrderParam;
		else
			startOrgAt++;

		params.push_back(var);

		if (HPL::arg.debugAll || HPL::arg.debugLog) {
			std::cout << HPL::arg.curIndent << "LOG: [FIND][PARAM]: " << HPL::curFile << ":" << HPL::lineCount << ": <type> <name> = <value>: " << var.type << " " << var.name << " = ";
			print(var);
		}
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

			HPL::resetRuntimeInfo();
			HPL::curFile = func.file;
			HPL::lineCount = func.startingLine;
			//HPL::equalBrackets

			for (int i = 0; i < func.params.size(); i++) {
				auto var = func.params[i];

				if (i < params.size())
					var.value = params[i].value;

				HPL::variables.push_back(var);
			}

			for (auto& line : func.code) {
				HPL::lineCount++;
				HPL::interpreteLine(line);

				if (HPL::functionOutput.has_value()) // If the function returned something, exit.
					break;
			}

			// Set the output value and type.
			output = HPL::functionOutput;
			// Reset the saved output value and type.
			HPL::functionOutput.reset_all();


			// If a global variable was edited in the function, save the changes.
			for (auto& oldV : oldVars) {
				for (auto newV : HPL::variables) {
					if (oldV.name == newV.name)
						oldV.value = newV.value;
				}
			}

			// Reset everything back to normal.
			HPL::variables = oldVars;
			HPL::curFile = oldCurFile;
			HPL::lineCount = oldLineCount;
			HPL::mode = oldMode;

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
	bool organize = false;

	if (!sentUserParams.empty() && sentUserParams.back().type == "IS_OOO") {
		sentUserParams.pop_back();
		organize = true;
	}

	if (func.name == globalFunction.name && func.params.size() < sentUserParams.size())
		HPL::throwError(true, "Too many parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", sentUserParams.size(), func.name.c_str(), func.params.size());
	if (func.name == globalFunction.name && func.minParamCount > sentUserParams.size())
		HPL::throwError(true, "Too few parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", sentUserParams.size(), func.name.c_str(), func.minParamCount);


	if (func.name == globalFunction.name) {
		if (organize) { // If there are any out of order arguments.
			HPL::function* outOfOrderFunc = &func; // The function.

			std::vector<HPL::variable> organizedParams = sentUserParams;
			std::string buf;
			int num = 0;

			for (const auto& outOfOrderParam : sentUserParams) {
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

			if (func.params[i].has_value() && (i + 1) > sentUserParams.size())
				sentUserParams.push_back(func.params[i]);
		}

		foundFunction = true;
		globalFunction.type = func.type;
		globalFunction.minParamCount = func.minParamCount;
	}

	return foundFunction;
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

		if (func.type != output.type)
			HPL::throwError(true, "Cannot return a '%s' type (the return type for '%s' is '%s', not '%s')", output.type.c_str(), funcName.c_str(), func.type.c_str(), output.type.c_str());

		existingVar->type = func.type;

		if (func.type == "string")
			existingVar->value = getStr(output.value);
		else if (func.type == "int")
			existingVar->value = getInt(output.value);
		else if (func.type == "bool")
			existingVar->value = getBool(output.value);
		else if (func.type == "float")
			existingVar->value = getFloat(output.value);
	}
	else {
		existingVar->reset_value();

		return FOUND_NOTHING;
	}

	return FOUND_SOMETHING;
}


allowedTypes coreFunctions(std::vector<HPL::variable> params) {
	if (useFunction({.type = "void", .name = "print", .params = {{"auto", "msg"}, {"string", "ending", "\n"}}, .minParamCount = 1}, params))
		print(params[0], getStr(params[1].value));

	else if (useFunction({.type = "string", .name = "str", .params = {{"auto", "value"}}, .minParamCount = 1}, params))
		return func_str(params[0]);

	else if (useFunction({.type = "int", .name = "int", .params = {{"auto", "value"}}, .minParamCount = 1}, params))
		return func_int(params[0]);

	else if (useFunction({.type = "bool", .name = "bool", .params = {{"auto", "value"}}, .minParamCount = 1}, params))
		return func_bool(params[0]);

	else if (useFunction({.type = "float", .name = "float", .params = {{"auto", "value"}}, .minParamCount = 1}, params))
		return func_float(params[0]);

	else if (useFunction({.type = "int", .name = "createFolder", .params = {{"string", "path"}}, .minParamCount = 1}, params))
		return createFolder(getStr(params[0].value));

	else if (useFunction({.type = "int", .name = "removeFolder", .params = {{"string", "path"}}, .minParamCount = 1}, params))
		return removeFolder(getStr(params[0].value));

	else if (useFunction({.type = "int", .name = "createFile", .params = {{"string", "path"}, {"string", "content", ""}, {"bool", "useUtf8BOM", false}}, .minParamCount = 1}, params))
		return createFile(getStr(params[0].value), getStr(params[1].value), getBool(params[2].value));

	else if (useFunction({.type = "int", .name = "readFile", .params = {{"string", "path"}}, .minParamCount = 1}, params))
		return readFile(getStr(params[0].value));

	else if (useFunction({.type = "int", .name = "writeFile",  .params = {{"string", "path"}, {"string", "content"}, {"string", "mode", "w"}}, .minParamCount = 2}, params))
		return writeFile(getStr(params[0].value), getStr(params[1].value), getStr(params[2].value));

	else if (useFunction({.type = "int", .name = "removeFile", .params = {{"string", "path"}}, .minParamCount = 1}, params))
		return removeFile(getStr(params[0].value));

	else if (useFunction({.type = "int", .name = "copyFile", .params = {{"string", "source"}, {"string", "output"}}, .minParamCount = 2}, params))
		return copyFile(getStr(params[0].value), getStr(params[1].value));

	else if (useFunction({.type = "int", .name = "writeToLine", .params = {{"string", "path"}, {"int", "line"}, {"string", "content"}, {"string", "mode", "w"}}, .minParamCount = 3}, params))
		return writeToLine(getStr(params[0].value), getInt(params[1].value), getStr(params[2].value), getStr(params[3].value));

	else if (useFunction({.type = "int", .name = "writeToMultipleLines", .params = {{"string", "path"}, {"int", "lineBegin"}, {"int", "lineEnd"}, {"string", "content"}, {"string", "mode", "w"}}, .minParamCount = 4}, params))
		return writeToMultipleLines(getStr(params[0].value), getInt(params[1].value), getInt(params[2].value), getStr(params[3].value), getStr(params[4].value));

	else if (useFunction({.type = "int", .name = "writeLocalisation", .params = {{"string", "file"}, {"string", "name"}, {"string", "description"}}, .minParamCount = 3}, params))
		return writeLocalisation(getStr(params[0].value), getStr(params[1].value), getStr(params[2].value));

	else if (useFunction({.type = "int", .name = "convertToDds", .params = {{"string", "input"}, {"string", "output"}}, .minParamCount = 2}, params))
		return convertToDds(getStr(params[0].value), getStr(params[1].value));

	else if (useFunction({.type = "string", .name = "getFilenameFromPath", .params = {{"string", "path"}}, .minParamCount = 1}, params))
		return getFilenameFromPath(getStr(params[0].value));

	else if (useFunction({.type = "bool", .name = "pathExists", .params = {{"string", "path"}}, .minParamCount = 1}, params))
		return pathExists(getStr(params[0].value));

	else if (useFunction({.type = "bool", .name = "find", .params = {{"string", "line"}, {"string", "input"}}, .minParamCount = 2}, params))
		return find(getStr(params[0].value), getStr(params[1].value));

	else if (useFunction({.type = "string", .name = "replaceAll", .params = {{"string", "str"}, {"string", "oldString"}, {"string", "newString"}}, .minParamCount = 3}, params))
		return replaceAll(getStr(params[0].value), getStr(params[1].value), getStr(params[2].value));


	return {};
}