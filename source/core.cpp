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
	"var"     // Generic type.
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

bool foundFunction = false;
HCL::function globalFunction;


int executeFunction(std::string name, std::string info, HCL::function& function, HCL::variable& output, bool dontCheck/* = false*/) {
	std::vector<std::string> values = split(info, ",", "(){}\"\"");
	std::vector<HCL::variable> params;
	HCL::arg.curIndent += "\t";

	for (auto& p : values) {
		HCL::variable var = {"NO_TYPE", "NO_NAME"};
		p = unstringify(p, false, ' ');
		std::string oldMatch = p;
		p = unstringify(p);

		HCL::variable* existingVar = getVarFromName(oldMatch);

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

					auto msg = xToStr(var.value); // Get the return from the current function.
					if (!isInt(p) && p != "true" && p != "false")
						msg = '\"' + msg + '\"'; // Since it's a string, we have to add quotes

					// Since the next function's param is gonna be "{currentFunctionName}({currentFunctionParams})",
					// we have to replace that with the current function's output (aka msg).
					noodles = replaceOnce(noodles, oldFuncValues[i].str(1) + "(" + oldFuncValues[i].str(2) + ")", msg);
				}
			}
		}
		else if (existingVar == nullptr) // If parameter isn't a variable.
			var = setCorrectValue(oldMatch);

		else if (existingVar != nullptr) // If parameter IS a variable.
			var = *existingVar;

		else
			HCL::throwError(true, "Variable '%s' doesn't exist (Cannot use a variable that doesn't exist).", p.c_str());

		params.push_back(var);

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << HCL::arg.curIndent << "LOG: [FIND][PARAM]: " << HCL::curFile << ":" << HCL::lineCount << ": <type> <name> = <value>: " << var.type << " " << var.name << " = ";
			print(var);
		}
	}
	foundFunction = false;
	globalFunction.name = name;

	output.value = coreFunctions(params);

	if (foundFunction) {
		function = globalFunction;
		output.type = function.type;
		globalFunction = {};

		if (HCL::arg.debugAll || HCL::arg.debugLog) {
			std::cout << HCL::arg.curIndent << "[FIND][FUNCTION](0): " << HCL::curFile << ":" << HCL::lineCount << ": <type> <name> | <output> (<output's type>): " << function.type << " " << function.name << " | " << xToStr(output.value) << " (" << output.type << ")" << std::endl;
			HCL::arg.curIndent.pop_back();
		}

		return FOUND_SOMETHING;
	}

	// Non-core functions
	for (auto func : HCL::functions) {
		if (useFunction(func, params)) {
			// Save the info and reset it all so that the interpreter doesn't spout random info.
			std::string oldCurFile = HCL::curFile;
			int oldLineCount = HCL::lineCount;
			std::vector<HCL::variable> oldVars = HCL::variables;
			int oldMode = HCL::mode;

			HCL::resetRuntimeInfo();
			HCL::curFile = func.file;
			HCL::lineCount = func.startingLine;
			//HCL::equalBrackets

			for (int i = 0; i < func.params.size(); i++) {
				auto var = func.params[i];
				if (i < params.size()) var.value = params[i].value;

				HCL::variables.push_back(var);
			}

			for (auto& line : func.code) {
				HCL::lineCount++;
				HCL::interpreteLine(line);

				if (HCL::functionOutput.has_value()) // If the function returned something, exit.
					break;
			}
			// Set the output value and type.
			output = HCL::functionOutput;
			// Reset the saved output value and type.
			HCL::functionOutput.reset_all();


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
				std::cout << HCL::arg.curIndent << "[FIND][FUNCTION](1): " << HCL::curFile << ":" << HCL::lineCount << ": <type> <name> | <output> (<output's type>): " << func.type << " " << func.name << " | " << xToStr(output.value) << " (" << output.type << ")" << std::endl;
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

	if (!dontCheck)
		HCL::throwError(true, "Function '%s' doesn't exist (Either the function is defined nowhere or it's a typo)", name.c_str());

	return FOUND_NOTHING;
}


bool useFunction(HCL::function func, std::vector<HCL::variable> &sentUserParams) {
	if (func.name == globalFunction.name && func.params.size() < sentUserParams.size())
		HCL::throwError(true, "Too many parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", sentUserParams.size(), func.name.c_str(), func.params.size());
	if (func.name == globalFunction.name && func.minParamCount > sentUserParams.size())
		HCL::throwError(true, "Too few parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", sentUserParams.size(), func.name.c_str(), func.minParamCount);


	if (func.name == globalFunction.name) {
		for (int i = 0; i < func.params.size(); i++) {
			if ((i + 1) <= sentUserParams.size()) {
				if (!coreTyped(func.params[i].type)) {
					auto& userParams = getVars(sentUserParams[i].value);

					HCL::structure* _struct = getStructFromName(func.params[i].type);

					if (_struct->value.size() < userParams.size())
						HCL::throwError(true, "smth smth bigger than struct size");
					if (_struct->minParamCount > userParams.size())
						HCL::throwError(true, "smth smth smaller than struct size");

					for (int x = 0; x < _struct->value.size(); x++) {
						auto& member = _struct->value[x];

						if (member.type != userParams[x].type && member.type != "var")
							HCL::throwError(true, "smth smth bad type (%s, %s)", member.type.c_str());
					}
					sentUserParams[i].type = func.params[i].type; // why?
				}

				if (func.params[i].type != sentUserParams[i].type && func.params[i].type != "var")
					HCL::throwError(true, "Cannot input a '%s' type to a %s-only parameter (param '%s' is %s-only)", sentUserParams[i].type.c_str(), func.params[i].type.c_str(), func.params[i].name.c_str(), func.params[i].type.c_str());
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


int assignFuncReturnToVar(HCL::variable* existingVar, std::string funcName, std::string funcParam, bool dontCheck/* = false*/) {
	HCL::function func;
	HCL::variable output;
	executeFunction(funcName, funcParam, func, output, dontCheck);

	if (output.has_value()) {
		if (output.type == "struct") {
			HCL::structure* s = getStructFromName(func.type);
			if (s != nullptr) {
				if (existingVar->type != func.type) {
					//HCL::throwError(true, "later");
				}
				else {
					auto result = getVars(output.value);
					existingVar->value = result;
				}

				return FOUND_SOMETHING;
			}
		}

		if (func.type != output.type)
			HCL::throwError(true, "Cannot return a '%s' type (the return type for '%s' is '%s', not '%s')", output.type.c_str(), funcName.c_str(), func.type.c_str(), output.type.c_str());

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
	}

	return FOUND_SOMETHING;
}


allowedTypes coreFunctions(std::vector<HCL::variable> params) {
	if (useFunction({"void", "print", {{"var", "msg"}, {"string", "ending", "\n"}}, .minParamCount = 1}, params))
		print(params[0], getStr(params[1].value));

	else if (useFunction({"string", "str", {{"var", "value"}}, .minParamCount = 1}, params))
		return func_str(params[0]);

	else if (useFunction({"int", "int", {{"var", "value"}}, .minParamCount = 1}, params))
		return func_int(params[0]);

	else if (useFunction({"bool", "bool", {{"var", "value"}}, .minParamCount = 1}, params))
		return func_bool(params[0]);

	else if (useFunction({"float", "float", {{"var", "value"}}, .minParamCount = 1}, params))
		return func_float(params[0]);

	else if (useFunction({"int", "createFolder", {{"string", "path"}}, .minParamCount = 1}, params))
		return createFolder(getStr(params[0].value));

	else if (useFunction({"int", "removeFolder", {{"string", "path"}}, .minParamCount = 1}, params))
		return removeFolder(getStr(params[0].value));

	else if (useFunction({"int", "createFile", {{"string", "path"}, {"string", "content", ""}, {"bool", "useUtf8BOM", false}}, .minParamCount = 1}, params))
		return createFile(getStr(params[0].value), getStr(params[1].value), getBool(params[2].value));

	else if (useFunction({"int", "readFile", {{"string", "path"}}, .minParamCount = 1}, params))
		return readFile(getStr(params[0].value));

	else if (useFunction({"int", "writeFile",  {{"string", "path"}, {"string", "content"}, {"string", "mode", "w"}}, .minParamCount = 2}, params))
		return writeFile(getStr(params[0].value), getStr(params[1].value), getStr(params[2].value));

	else if (useFunction({"int", "removeFile", {{"string", "path"}}, .minParamCount = 1}, params))
		return removeFile(getStr(params[0].value));

	else if (useFunction({"int", "copyFile", {{"string", "source"}, {"string", "output"}}, .minParamCount = 2}, params))
		return copyFile(getStr(params[0].value), getStr(params[1].value));

	else if (useFunction({"int", "writeToLine", {{"string", "path"}, {"int", "line"}, {"string", "content"}, {"string", "mode", "w"}}, .minParamCount = 3}, params))
		return writeToLine(getStr(params[0].value), getInt(params[1].value), getStr(params[2].value), getStr(params[3].value));

	else if (useFunction({"int", "writeToMultipleLines", {{"string", "path"}, {"int", "lineBegin"}, {"int", "lineEnd"}, {"string", "content"}, {"string", "mode", "w"}}, .minParamCount = 4}, params))
		return writeToMultipleLines(getStr(params[0].value), getInt(params[1].value), getInt(params[2].value), getStr(params[3].value), getStr(params[4].value));

	else if (useFunction({"int", "writeLocalisation", {{"string", "file"}, {"string", "name"}, {"string", "description"}}, .minParamCount = 3}, params))
		return writeLocalisation(getStr(params[0].value), getStr(params[1].value), getStr(params[2].value));

	else if (useFunction({"int", "convertToDds", {{"string", "input"}, {"string", "output"}}, .minParamCount = 2}, params))
		return convertToDds(getStr(params[0].value), getStr(params[1].value));

	else if (useFunction({"string", "getFilenameFromPath", {{"string", "path"}}, .minParamCount = 1}, params))
		return getFilenameFromPath(getStr(params[0].value));

	//else if (useFunction({"bool", "pathExists", {{"string", "str"}, {"string", "oldString"}, {"string", "newString"}}, .minParamCount = 3}, params))
	//	return replaceAll(getStr(params[0].value), getStr(params[1].value), getStr(params[2].value)[

	else if (useFunction({"string", "replaceAll", {{"string", "str"}, {"string", "oldString"}, {"string", "newString"}}, .minParamCount = 3}, params))
		return replaceAll(getStr(params[0].value), getStr(params[1].value), getStr(params[2].value));

	//else if (useFunction({"bool", "find", {{"string", "str"}, {"string", "oldString"}, {"string", "newString"}}, .minParamCount = 3}, params))
	//	return replaceAll(getStr(params[0].value)[0], params[1].value[0], getStr(params[2].value)[

	return {};
}