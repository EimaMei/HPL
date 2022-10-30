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

// Core functions and types.
#include <interpreter.hpp>
#include <core.hpp>
#include <helper.hpp>

#include <iostream>
#include <sys/stat.h>
#include <dirent.h>

std::vector<std::string> coreTypes = {
	"string", // Works just like std::string/const char*.
	"int",    // Regular int.
	"bool",   // Just an int except it can only be 0 or 1.
	"scope",  // A scope variable, meaning HOI4 code can be executed inside of it.
	"var"     // Generic type.
};
bool found = false;
std::string globalName;
int userSentParamCount;


int checkForFunctions(std::string name, std::string info) {
	std::vector<std::string> values = split(info, ",", '\"'); // Split info into paramaters.
	std::vector<HCL::variable> params;

	for (auto& p : values) {
		HCL::variable var = {"?", "?", {"?"}}; HCL::variable structInfo;
		useRegex(p, R"(\s*(\".*\"|\{.*\}|[^\s*]*)\s*)"); // We only get the actual value and remove any unneeded whitespaces/quotes.
		p = unstringify(HCL::matches.str(1));

		HCL::variable* existingVar = getVarFromName(p, &structInfo);

		if (existingVar == NULL) { // If parameter isn't a variable.
			if (find(HCL::matches.str(1), "\""))
				var.type = "string";
			else if (isInt(p))
				var.type = "int";
			else if (p == "true" || p == "false")
				var.type = "bool";
			else
				var.type = "var";

			var.value[0] = p;
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
	found = false;
	globalName = name;
	userSentParamCount = params.size();

	coreFunctions(params);

	// Non-core functions
	for (auto func : HCL::functions) {
		if (useFunction(func.name, func.minParamCount, func.params.size())) {
			std::string oldCurFile = HCL::curFile;
			int oldLineCount = HCL::lineCount;
			HCL::resetRuntimeInfo();

			std::vector<HCL::variable> oldVars = HCL::variables;

			for (int i = 0; i < func.params.size(); i++) {
				auto var = func.params[i];
				if (var.value[0].empty()) var.value = params[i].value;

				HCL::variables.push_back(var);
			}

			for (auto line : func.code) {
				HCL::interpreteLine(line);
			}
			HCL::variables = oldVars;
			HCL::curFile = oldCurFile;
			HCL::lineCount = oldLineCount;
		}
	}

	if (found) return FOUND_SOMETHING;
	return FOUND_NOTHING;
}


bool useFunction(std::string name, int minParamCount, int maxParamCount) {
	if (name == globalName && maxParamCount < userSentParamCount)
		HCL::throwError("Too many parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", userSentParamCount, name.c_str(), maxParamCount);
	if (name == globalName && minParamCount > userSentParamCount)
		HCL::throwError("Too few parameters were provided (you provided '%i' arguments when function '%s' requires at least '%i' arguments)", userSentParamCount, name.c_str(), minParamCount);
	found = true;
	
	return name == globalName;
}

/*
=================================================
|             CORE FUNCTIONS OF HCL             |
=================================================
*/

void coreFunctions(std::vector<HCL::variable> params) {
	if (useFunction("print", 1, 2)) {
		std::string end = "\n";
		if (params.size() == 2) end = params[1].value[0];

		print(params[0], end);
	}
	else if (useFunction("createFolder", 1, 2)) {
		int mode = 0777;
		if (params.size() == 2) mode = std::stoi(params[1].value[0]);

		createFolder(params[0], mode);
	}
	else if (useFunction("removeFolder", 1, 1)) {
		removeFolder(params[0]);
	}
}


void print(HCL::variable msg, std::string end/* = \n*/) {
	std::string output;
	std::string vtype = msg.type;

	if (msg.value.size() > 1)
		output += "{";

	for (int i = 0; i < msg.value.size(); i++) {
		auto value = msg.value[i];
		if (i < msg.extra.size()) vtype = msg.extra[i];

		output += value;

		if (msg.value.size() > 1 && ((i + 1) < msg.value.size()))
			output += ", ";
	}

	if (msg.value.size() > 1) output += "}";

	printf("%s%s", output.c_str(), end.c_str());
}


int createFolder(HCL::variable path, int mode/* = 0777*/) {
	if (path.type != "string")
		HCL::throwError("Cannot input a '%s' type to a string-only parameter (param 'path' is string-only)", path.type.c_str());

	int check;
	std::string fullPath;
	std::vector<std::string> folders = split(replace(path.value[0], '\\', '/'), "/");

	for (auto folder : folders) {
		fullPath += folder;
		check = mkdir(fullPath.c_str(), mode);
		fullPath += "/";
	}
  
    return check;
    
}


int removeFolder(HCL::variable path) {
	if (path.type != "string")
		HCL::throwError("Cannot input a '%s' type to a string-only parameter (param 'path' is string-only)", path.type.c_str());

	int check = remove(replace(path.value[0], '\\', '/').c_str());
  
    return check;
}