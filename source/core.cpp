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
	"string", // Works just like std::string/const char*.
	"int",    // Regular int.
	"float",
	"bool",   // Just an int except it can only be 0 or 1.
	"scope",  // A scope variable, meaning HOI4 code can be executed inside of it.
	"var"     // Generic type.
};
std::vector<std::string> coreFunctionList = {
	// Misc.
	"print",
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

bool foundFunction = false;
HCL::function globalFunction;
int userSentParamCount;


int executeFunction(std::string name, std::string info, HCL::function& function, void*& output, std::string& returnTypeOutput, bool dontCheck/* = false*/) {
	std::vector<std::string> values = split(info, ",", "(){}\"\"");
	std::vector<HCL::variable> params;
	bool pass = (false != dontCheck);
	HCL::arg.curIndent += "\t";

	if (!pass) { // If the error checking isn't disabled.
		for (auto func : HCL::functions) {
			if (func.name == name) { // It's an already defined function, it's fine.
				pass = true;
				break;
			}
		}

		if (!pass) {
			for (auto func : coreFunctionList) {
				if (func == name) { // It's a core function, it's fine.
					pass = true;
					break;
				}
			}

			if (!pass) // No function was found.
				HCL::throwError(true, "Function '%s' doesn't exist (Either the function is defined nowhere or it's a typo)", name.c_str());
		}
	}

	for (auto& p : values) {
		HCL::variable var = {"NO_TYPE", "NO_NAME", {"NO_VALUE"}}; HCL::variable structInfo;
		p = unstringify(p, false, ' ');
		std::string oldMatch = p;
		p = unstringify(p);

		HCL::variable* existingVar = getVarFromName(oldMatch, &structInfo);

		// Checks if the parameter is just a function.
		if (useRegex(p, R"(^\s*([^\s\(]+)\((.*)\)\s*$)")) {
			// If so, get the name and params of said parameter.
			std::vector<HCL::vector> funcValues, oldFuncValues;
			std::string str = p;
			while (true) {
				// Check if the param isn't just a function.
				useRegex(str, R"(\s*([^\s\(]+)\((.*)\)\s*)");
				if (!HCL::matches.str(2).empty() || !str.empty()) {
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
				assignFuncReturnToVar(&var, funcValues[i].str(1), funcValues[i].str(2), true);

				if ((i + 1) < funcValues.size()) { // If there are more functions inside the param.
					auto& noodles = funcValues[i + 1].value[1]; // Get the next function.

					std::string msg = var.value[0]; // Get the return from the current function.
					if (!isInt(p) && p != "true" && p != "false")
						msg = '\"' + var.value[0] + '\"'; // Since it's a string, we have to add quotes

					// Since the next function's param is gonna be "{currentFunctionName}({currentFunctionParams})",
					// we have to replace that with the current function's output (aka msg).
					noodles = replaceOnce(noodles, oldFuncValues[i].str(1) + "(" + oldFuncValues[i].str(2) + ")", msg);
				}
			}
		}
		else if (existingVar == NULL) { // If parameter isn't a variable.
			getValueFromFstring(oldMatch, p);

			// Core types.
			if (find(oldMatch, "\""))
				var.type = "string";
			else if (isInt(p) && !find(p, "."))
				var.type = "int";
			else if (isInt(p) && find(p, "."))
				var.type = "float";
			else if (p == "true" || p == "false") {
				var.type = "bool";
				p = std::to_string(stringToBool(p));
			}
			// Non-core types.
			else if (p.front() == '{' && p.back() == '}') {
				p = unstringify(p, true);
				useIterativeRegex(p, R"(([^,\s]+))");
				var.value = HCL::matches.value;
				var.type = "struct";
			}
			else
				HCL::throwError(true, "Variable '%s' doesn't exist", p.c_str());

			if (var.type != "struct")
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

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << HCL::arg.curIndent << "LOG: [FIND][PARAM]: " << HCL::curFile << ":" << HCL::lineCount << ": <type> <name> = <value>: " << var.type << " " << var.name << " = ";
			print(var);
		}
	}
	foundFunction = false;
	globalFunction.name = name;
	userSentParamCount = params.size();

	output = coreFunctions(params);

	if (foundFunction) {
		function = globalFunction;
		returnTypeOutput = function.type;
		globalFunction = {};

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << HCL::arg.curIndent << "[FIND][FUNCTION](0): " << HCL::curFile << ":" << HCL::lineCount << ": <type> <name> | <output> <output's type>: " << function.type << " " << function.name << " | " << output << " " << returnTypeOutput << std::endl;
			HCL::arg.curIndent.pop_back();
		}

		return FOUND_SOMETHING;
	}

	// Non-core functions
	for (auto func : HCL::functions) {
		if (useFunction(func.type, func.name, func.minParamCount, func.params.size())) {
			// Save the info and reset it all so that the interpreter doesn't spout random info.
			std::string oldCurFile = HCL::curFile;
			int oldLineCount = HCL::lineCount;
			std::vector<HCL::variable> oldVars = HCL::variables;

			HCL::resetRuntimeInfo();
			HCL::curFile = func.file;
			HCL::lineCount = func.startingLine;

			for (int i = 0; i < func.params.size(); i++) {
				auto var = func.params[i];
				if (i < params.size()) var.value = params[i].value;

				HCL::variables.push_back(var);
			}

			for (auto& line : func.code) {
				HCL::lineCount++;
				HCL::interpreteLine(line);

				if (HCL::functionOutput != nullptr || (HCL::functionReturnType == "bool" && (HCL::functionOutput == 0x00 || voidToInt(HCL::functionOutput) == 0x01))) // If the function returned something, exit.
					break;
			}
			// Set the output value and type.
			output = HCL::functionOutput;
			returnTypeOutput = HCL::functionReturnType;
			// Reset the saved output value and type.
			HCL::functionOutput = nullptr;
			HCL::functionReturnType = "";


			// If a global variable was edited in the function, save the changes.
			for (auto& oldV : oldVars) {
				for (auto newV : HCL::variables) {
					if (oldV.name == newV.name)
						oldV.value = newV.value;
				}
			}
			// Reset everything back to normal.
			HCL::variables = oldVars;
			HCL::curFile = oldCurFile;
			HCL::lineCount = oldLineCount;

			globalFunction = func;
			foundFunction = true;

			if (HCL::arg.debugAll || HCL::arg.debugLog) {
				std::cout << HCL::arg.curIndent << "[FIND][FUNCTION](1): " << HCL::curFile << ":" << HCL::lineCount << ": <type> <name> | <output> <output's type>: " << func.type << " " << func.name << " | " << output << " " << returnTypeOutput << std::endl;
				HCL::arg.curIndent.pop_back();
			}
			break;
		}
	}

	if (foundFunction) {
		function = globalFunction;
		globalFunction = {};

		return FOUND_SOMETHING;
	}
	return FOUND_NOTHING;
}


bool useFunction(std::string type, std::string name, int minParamCount, int maxParamCount) {
	if (name == globalFunction.name && maxParamCount < userSentParamCount)
		HCL::throwError(true, "Too many parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", userSentParamCount, name.c_str(), maxParamCount);
	if (name == globalFunction.name && minParamCount > userSentParamCount)
		HCL::throwError(true, "Too few parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", userSentParamCount, name.c_str(), minParamCount);


	if (name == globalFunction.name) {
		foundFunction = true;
		globalFunction.type = type;
		globalFunction.minParamCount = minParamCount;
	}

	return foundFunction;
}


int assignFuncReturnToVar(HCL::variable* existingVar, std::string funcName, std::string funcParam, bool dontCheck/* = false*/) {
	HCL::function func; std::string returnType;
	void* output;
	executeFunction(funcName, funcParam, func, output, returnType, dontCheck);

	if (output != nullptr) {
		if (returnType == "struct") {
			HCL::structure* s = getStructFromName(func.type);
			if (s != nullptr) {
				if (existingVar->type != func.type) {
					//HCL::throwError(true, "later");
				}
				else {
					auto result = static_cast<std::vector<std::string>*>(output);
					existingVar->value.clear();
					returnType = func.type;

					if (result->empty())
						existingVar->value.push_back("");

					for (auto& r : *result)
						existingVar->value.push_back(r);

				}

				return FOUND_SOMETHING;
			}
		}

		if (func.type != returnType)
			HCL::throwError(true, "Cannot return a '%s' type (the return type for '%s' is '%s', not '%s')", returnType.c_str(), funcName.c_str(), func.type.c_str(), returnType.c_str());

		existingVar->type = func.type;
		if (func.type == "int" || func.type == "bool")
			existingVar->value[0] = std::to_string(voidToInt(output));
		else if (func.type == "float")
			existingVar->value[0] = std::to_string(voidToFloat(output));
		else if (func.type == "string")
			existingVar->value[0] = voidToString(output);
	}
	else if (output == nullptr && (func.type == "int" || func.type == "bool" || func.type == "float")) { // The function returned a 0, however a 0 is just nullptr so we have to use a "hack" to get the int.
		existingVar->type = func.type;
		existingVar->value[0] = std::to_string(voidToInt(output));
	}
	else {
		existingVar->value[0] = "";
	}

	return FOUND_SOMETHING;
}


void* coreFunctions(std::vector<HCL::variable> params) {
	// void print(var msg, string end = "\n")
	if (useFunction("void", "print", 1, 2)) {
		std::string end = "\n";
		if (params.size() == 2) end = params[1].value[0];

		print(params[0], end); // Since our function doesn't return anything, we return nothing.
	}
	// int createFolder(string path, int mode = 0777)
	else if (useFunction("int", "createFolder", 1, 2)) {
		int mode = 0777;

		if (params[0].type != "string") HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param 'path' is string-only)", true, params[0].type.c_str());
		if (params.size() == 2) {
			if (params[0].type == "int")
				mode = std::stoi(params[1].value[0]);
			else
				HCL::throwError(true, "Cannot input a '%s' type to an int-only parameter (param '%s' is int-only)", true, params[1].type.c_str(), "mode");
		}


		int result = createFolder(params[0].value[0], mode);
		return intToVoid(result);  // Returns the result as int.
	}
	// int removeFolder(string path)
	else if (useFunction("int", "removeFolder", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", true, params[0].type.c_str(), "path");

		int result = removeFolder(params[0].value[0]);

		return intToVoid(result); // Returns the result as int.
	}
	// int createFile(string path, string content = "", bool useUtf8BOM = false)
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
	// string readFile(string path)
	else if (useFunction("string", "readFile", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[0].type.c_str(), "path");

		std::string result = readFile(params[0].value[0]);

		return stringToVoid(result); // Returns the result as int.
	}
	// int writeFile(string path, string content, string mode = "w")
	else if (useFunction("int", "writeFile", 2, 3)) {
		std::string mode = "w";

		if (params[0].type != "string") HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[0].type.c_str(), "path");
		if (params.size() == 3) {
			mode = params[2].value[0];
			if (params[2].type != "string") HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[2].type.c_str(), "mode");
		}

		int result = writeFile(params[0].value[0], params[1].value[0], mode);

		return intToVoid(result); // Returns the result as int.
	}
	// int writeToLine(string path, int line, string content, string mode = "w")
	else if (useFunction("int", "writeToLine", 3, 4)) {
		std::string mode = "w";

		if (params[0].type != "string") HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[0].type.c_str(), "path");
		if (params[1].type != "int") HCL::throwError(true, "Cannot input a '%s' type to an int-only parameter (param '%s' is int-only)", params[1].type.c_str(), "line");
		if (params.size() == 4) {
			if (params[3].type != "string")
				HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[3].type.c_str(), "mode");
			else
				mode = params[3].value[0];
		}
		int result = writeToLine(params[0].value[0], std::stoul(params[1].value[0]), params[2].value[0], mode);

		return intToVoid(result); // Returns the result as int.
	}
	// int writeToLine(string path, int lineBegin, int lineEnd, string content, string mode = "w")
	else if (useFunction("int", "writeToMultipleLines", 4, 5)) {
		std::string mode = "w";

		if (params[0].type != "string") HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[0].type.c_str(), "path");
		if (params[1].type != "int") HCL::throwError(true, "Cannot input a '%s' type to an int-only parameter (param '%s' is int-only)", params[1].type.c_str(), "lineBegin");
		if (params[2].type != "int") HCL::throwError(true, "Cannot input a '%s' type to an int-only parameter (param '%s' is int-only)", params[1].type.c_str(), "lineEnd");
		if (params.size() == 5) {
			if (params[4].type != "string")
				HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", params[3].type.c_str(), "mode");
			else
				mode = params[4].value[0];
		}
		int result = writeToMultipleLines(params[0].value[0], std::stoul(params[1].value[0]), std::stoul(params[2].value[0]), params[3].value[0], mode);

		return intToVoid(result); // Returns the result as int.
	}
	// int removeFile(string path)
	else if (useFunction("int", "removeFile", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param 'path' is string-only)", true, params[0].type.c_str());

		int result = removeFile(params[0].value[0]);

		return intToVoid(result); // Returns the result as int.
	}
	// int copyFile(string source, string output)
	else if (useFunction("int", "copyFile", 2, 2)) {
		std::vector<std::string> parNames = {"source", "input"};
		for (int i = 0; i < parNames.size(); i++) {
			if (params[i].type != "string")
				HCL::throwError(true, "Cannot input a '%s' type to a core-type only parameter (param '%s' is core-type only)", true, params[i].type.c_str(), parNames[i].c_str());
		}

		int result = copyFile(params[0].value[0], params[1].value[0]);

		return intToVoid(result); // Returns the result
	}
	// int moveFile(string source, string output)
	else if (useFunction("int", "moveFile", 2, 2)) {
		std::vector<std::string> parNames = {"source", "input"};
		for (int i = 0; i < parNames.size(); i++) {
			if (params[i].type != "string")
				HCL::throwError(true, "Cannot input a '%s' type to a core-type only parameter (param '%s' is core-type only)", true, params[i].type.c_str(), parNames[i].c_str());
		}

		int result = moveFile(params[0].value[0], params[1].value[0]);

		return intToVoid(result); // Returns the result
	}
	// int convertToDds(string input, string output)
	else if (useFunction("int", "convertToDds", 2, 2)) {
		std::vector<std::string> parNames = {"input", "output"};
		for (int i = 0; i < parNames.size(); i++) {
			if (params[i].type != "string")
				HCL::throwError(true, "Cannot input a '%s' type to a core-type only parameter (param '%s' is core-type only)", true, params[i].type.c_str(), parNames[i].c_str());
		}

		int result = convertToDds(params[0].value[0], params[1].value[0]);

		return intToVoid(result); // Returns the result
	}
	// int pathExists(string path)
	else if (useFunction("bool", "pathExists", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param 'path' is string-only)", true, params[0].type.c_str());

		bool result = pathExists(params[0].value[0]);

		return intToVoid(result); // Returns the result
	}
	// bool find(string line, string str)
	else if (useFunction("bool", "find", 2, 2)) {
		std::vector<std::string> parNames = {"line, str"};
		for (int i = 0; i < parNames.size(); i++) {
			if (!coreTyped(params[i].type))
				HCL::throwError(true, "Cannot input a '%s' type to a core-type only parameter (param '%s' is core-type only)", true, params[i].type.c_str(), parNames[i].c_str());
		}

		int res = find(params[0].value[0], params[1].value[0]);

		return intToVoid(res);
	}
	// int writeLocalisation(string file, string name, string description)
	else if (useFunction("int", "writeLocalisation", 3, 3)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", true, params[0].type.c_str(), "file");

		int res = writeLocalisation(params[0].value[0], params[1].value[0], params[2].value[0]);

		return intToVoid(res);
	}
	// string replaceAll(string str, string oldString, string newString);
	else if (useFunction("string", "replaceAll", 3, 3)) {
		std::string res = replaceAll(params[0].value[0], params[1].value[0], params[2].value[0]);

		return stringToVoid(res);
	}
	// string getFilenameFromPath(string path)
	else if (useFunction("string", "getFilenameFromPath", 1, 1)) {
		if (params[0].type != "string")
			HCL::throwError(true, "Cannot input a '%s' type to a string-only parameter (param '%s' is string-only)", true, params[0].type.c_str(), "path");

		std::string res = getFilenameFromPath(params[0].value[0]);

		return stringToVoid(res);
	}

	return nullptr;
}