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


std::vector<std::string> coreTypes = {
	"string", // Works just like std::string/const char*.
	"int",    // Regular int.
	"bool",   // Just an int except it can only be 0 or 1.
	"scope",  // A scope variable, meaning HOI4 code can be executed inside of it.
	"var"     // Generic type.
};

bool foundFunction = false;
std::string globalFunctionName;
int userSentParamCount;
HCL::function functionPointer;
std::string functionType;

int checkForFunctions(std::string name, std::string info, HCL::function& function, void*& output) {
	std::vector<std::string> values = split(info, ",", "()\"\"");

	std::vector<HCL::variable> params;

	for (auto& p : values) {
		HCL::variable var = {"?", "?", {"?"}}; HCL::variable structInfo;
		useRegex(p, R"(\s*([A-Za-z0-9\.]+\(.*\)|f?\".*\"|\w*)\s*)"); // We only get the actual value and remove any unneeded whitespaces/quotes.
		p = unstringify(HCL::matches.str(1));
		std::string oldMatch = HCL::matches.str(1);

		HCL::variable* existingVar = getVarFromName(oldMatch, &structInfo);

		// Checks if the parameter is just a function.
		if (useRegex(p, R"(^\s*([A-Za-z0-9\.]+)\((.*)\)\s*$)")) {
			// If so, get the name and params of said parameter.
			std::vector<HCL::vector> funcValues, oldFuncValues;
			std::string str = p;
			while (true) {
				// Check if the param isn't just a function.
				useRegex(str, R"(\s*([A-Za-z0-9\.]+)\((.*)\))");
				if (!HCL::matches.str(2).empty()) {
					funcValues.insert(funcValues.begin(), HCL::matches);
					str = HCL::matches.str(2);
				}
				else
					break;
			}
			oldFuncValues = funcValues;
			for (int i = 0; i < funcValues.size(); i++) {
				// Since the param DOES have functions inside, we have to get that functions' output.
				std::vector<std::string> list = split(funcValues[i].str(2), ",", "()\"\"");
				assignFuncReturnToVar(&var, funcValues[i].str(1), funcValues[i].str(2));

				if ((i + 1) < funcValues.size()) { // If there are more functions inside the param.
					auto& noodles = funcValues[i + 1].value[1]; // Get the next function.

					std::string msg = var.value[0]; // Get the return from the current function.
					if (isInt(p) || p == "true" || p == "false") {
						// do nothing
				 	}
					else
						msg = '\"' + var.value[0] + '\"'; // Since it's a string, we have to add quotes

					// Since the next function's param is gonna be "{currentFunctionName}({currentFunctionParams})",
					// we have to replace that with the current function's output (aka msg).
					noodles = replaceOnce(noodles, oldFuncValues[i].str(1) + "(" + oldFuncValues[i].str(2) + ")", msg);
				}
			}
		}
		else if (existingVar == NULL) { // If parameter isn't a variable.
			if (find(oldMatch, "\""))
				var.type = "string";
			else if (isInt(p))
				var.type = "int";
			else if (p == "true" || p == "false") {
				var.type = "bool";
				p = std::to_string(stringToBool(p));
			}
			else
				HCL::throwError(true, "Variable '%s' doesn't exist", p.c_str());

			if (getValueFromFstring(oldMatch, var.value[0]) == 0) {// Is F-string string.
				var.value[0] = convertBackslashes(var.value[0]);
			}
			else
				var.value[0] = convertBackslashes(p);
		}
		else { // If parameter IS a variable.
			var.name = p;
			var.extra = existingVar->extra;

			if (structInfo.name.empty()) // if the type isn't a struct
				var.type = existingVar->type;
			else
				var.type = structInfo.type;

			if (structInfo.value.empty()) // Edit a regular variable
				var.value = existingVar->value;
			else {// Edit a struct member
				int index = std::stoi(structInfo.extra[0]);
				var.value[0] = existingVar->value[index];
			}
		}

		params.push_back(var);
	}
	foundFunction = false;
	globalFunctionName = name;
	userSentParamCount = params.size();
	functionType = "";

	output = coreFunctions(params);

	// Non-core functions
	for (auto func : HCL::functions) {
		if (useFunction(func.type, func.name, func.minParamCount, func.params.size())) {
			// Save the info and reset it all so that the interpreter doesn't spout random info.
			std::string oldCurFile = HCL::curFile;
			int oldLineCount = HCL::lineCount;
			std::vector<HCL::variable> oldVars = HCL::variables;
			H
			CL::resetRuntimeInfo();
			HCL::curFile = func.file;
			HCL::lineCount = func.startingLine;

			for (int i = 0; i < func.params.size(); i++) {
				auto var = func.params[i];
				if (var.value[0].empty()) var.value = params[i].value;

				HCL::variables.push_back(var);
			}

			for (auto line : func.code) {
				HCL::lineCount++;
				HCL::interpreteLine(line);
			}

			// Reset everything back to normal.
			HCL::variables = oldVars;
			HCL::curFile = oldCurFile;
			HCL::lineCount = oldLineCount;

			functionType = func.type;
			function = func;
			break;
		}
	}

	if (foundFunction) {
		functionPointer.params = params;
		function = functionPointer;
		functionPointer = {};

		return FOUND_SOMETHING;
	}
	return FOUND_NOTHING;
}


bool useFunction(std::string type, std::string name, int minParamCount, int maxParamCount) {
	if (name == globalFunctionName && maxParamCount < userSentParamCount)
		HCL::throwError(true, "Too many parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", userSentParamCount, name.c_str(), maxParamCount);
	if (name == globalFunctionName && minParamCount > userSentParamCount)
		HCL::throwError(true, "Too few parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", userSentParamCount, name.c_str(), minParamCount);


	if (name == globalFunctionName) {
		foundFunction = true;
		functionType = type;

		functionPointer.type = type;
		functionPointer.name = name;
		functionPointer.minParamCount = minParamCount;
	}

	return name == globalFunctionName;
}


void assignFuncReturnToVar(HCL::variable* existingVar, std::string funcName, std::string funcParam) {
	HCL::function func;
	void* output;
	checkForFunctions(funcName, funcParam, func, output);

	if (output != nullptr) {
		existingVar->type = func.type;
		if (func.type == "int" || func.type == "bool")
			existingVar->value[0] = std::to_string(voidToInt(output));
		else if (func.type == "string")
			existingVar->value[0] = voidToString(output);
	}
	else if (output == nullptr && (func.type == "int" || func.type == "bool")) { // The function returned a 0, however a 0 is just nullptr so we have to use a "hack" to get the int.
		existingVar->type = func.type;
		existingVar->value[0] = std::to_string(voidToInt(output));
	}
	else {
		existingVar->value[0] = "";
	}
}


void* coreFunctions(std::vector<HCL::variable> params) {
	if (useFunction("void", "print", 1, 2)) {
		std::string end = "\n";
		if (params.size() == 2) end = params[1].value[0];

		print(params[0], end);

		// Since our function doesn't return anything, we return nothing.
	}

	else if (useFunction("int", "createFolder", 1, 2)) {
		int mode = 0777;

		if (params[0].type != "string") HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param 'path' is string-only)", true, params[0].type.c_str());
		if (params.size() == 2) mode = std::stoi(params[1].value[0]);

		int result = createFolder(params[0].value[0], mode);
		return intToVoid(result);  // Returns the result as int.
	}

	else if (useFunction("int", "removeFolder", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", true, params[0].type.c_str(), "path");

		int result = removeFolder(params[0].value[0]);

		return intToVoid(result); // Returns the result as int.
	}

	else if (useFunction("int", "createFile", 1, 3)) {
		std::string content = "";
		bool useUtf8BOM = false;

		if (params[0].type != "string") HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[0].type.c_str(), "path");
		if (params.size() > 1) content = params[1].value[0];
		if (params.size() > 2) {
			if (params[2].type == "bool" || params[2].type == "int")
				useUtf8BOM = stringToBool(params[2].value[0]);
			else
				HCL::throwError(true, "Cannot input a '%s' type to a bool-only parameter (param '%s' is bool-only)", params[2].type.c_str(), "useUtf8BOM");
		}

		int result = createFile(params[0].value[0], content, useUtf8BOM);

		return intToVoid(result);
	}

	else if (useFunction("string", "readFile", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[0].type.c_str(), "path");

		std::string result = readFile(params[0].value[0]);

		return stringToVoid(result); // Returns the result as int.
	}

	else if (useFunction("int", "writeFile", 2, 3)) {
		std::string mode = "w";

		if (params[0].type != "string") HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[0].type.c_str(), "path");
		if (params.size() == 3) {
			mode = params[2].value[0];
			if (params[2].type != "string") HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[0].type.c_str(), "mode");
		}

		int result = writeFile(params[0].value[0], params[1].value[0], mode);

		return intToVoid(result); // Returns the result as int.
	}

	else if (useFunction("int", "removeFile", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param 'path' is string-only)", true, params[0].type.c_str());

		int result = removeFile(params[0].value[0]);

		return intToVoid(result); // Returns the result as int.
	}

	else if (useFunction("int", "copyFile", 2, 2)) {
		std::vector<std::string> parNames = {"source", "input"};
		for (int i = 0; i < parNames.size(); i++) {
			if (params[i].type != "string")
				HCL::throwError(true, "Cannot input a '%s' type to a core-type only parameter (param '%s' is core-type only)", true, params[i].type.c_str(), parNames[i].c_str());
		}

		int result = copyFile(params[0].value[0], params[1].value[0]);

		return intToVoid(result); // Returns the result
	}

	else if (useFunction("int", "moveFile", 2, 2)) {
		std::vector<std::string> parNames = {"source", "input"};
		for (int i = 0; i < parNames.size(); i++) {
			if (params[i].type != "string")
				HCL::throwError(true, "Cannot input a '%s' type to a core-type only parameter (param '%s' is core-type only)", true, params[i].type.c_str(), parNames[i].c_str());
		}

		int result = moveFile(params[0].value[0], params[1].value[0]);

		return intToVoid(result); // Returns the result
	}

	else if (useFunction("int", "convertToDds", 2, 2)) {
		std::vector<std::string> parNames = {"input", "output"};
		for (int i = 0; i < parNames.size(); i++) {
			if (params[i].type != "string")
				HCL::throwError(true, "Cannot input a '%s' type to a core-type only parameter (param '%s' is core-type only)", true, params[i].type.c_str(), parNames[i].c_str());
		}

		int result = convertToDds(params[0].value[0], params[1].value[0]);

		return intToVoid(result); // Returns the result
	}

	else if (useFunction("bool", "pathExists", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param 'path' is string-only)", true, params[0].type.c_str());

		bool result = pathExists(params[0].value[0]);

		return intToVoid(result); // Returns the result
	}

	else if (useFunction("bool", "find", 2, 2)) {
		std::vector<std::string> parNames = {"line, str"};
		for (int i = 0; i < parNames.size(); i++) {
			if (!coreTyped(params[i].type))
				HCL::throwError(true, "Cannot input a '%s' type to a core-type only parameter (param '%s' is core-type only)", true, params[i].type.c_str(), parNames[i].c_str());
		}

		int res = find(params[0].value[0], params[1].value[0]);

		return intToVoid(res);
	}

	else if (useFunction("int", "writeLocalisation", 3, 3)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", true, params[0].type.c_str(), "file");

		int res = writeLocalisation(params[0].value[0], params[1].value[0], params[2].value[0]);

		return intToVoid(res);
	}

	else if (useFunction("string", "replaceAll", 3, 3)) {
		std::vector<std::string> parNames = {"str, oldString", "newString"};
		for (int i = 0; i < parNames.size(); i++) {
			if (!coreTyped(params[i].type))
				HCL::throwError(true, "Cannot input a '%s' type to a core-type only parameter (param '%s' is core-type only)", true, params[i].type.c_str(), parNames[i].c_str());
		}
		std::string res = replaceAll(params[0].value[0], params[1].value[0], params[2].value[0]);

		return stringToVoid(res);
	}
	//std::string getFilenameFromPath(std::string path)
	else if (useFunction("string", "getFilenameFromPath", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", true, params[0].type.c_str(), "path");

		std::string res = getFilenameFromPath(params[0].value[0]);

		return stringToVoid(res);
	}

	return nullptr;
}