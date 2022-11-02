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


std::vector<std::string> split(std::string str, std::string value, char charScope/* = NULL*/, char charScope2/* = NULL*/) {
	std::vector<std::string> list;
	std::string buf;
	int counter = 0;

	for (auto x : str) {
		buf += x;
		if ((charScope != '\0' && x == charScope) || (charScope2 != '\0' && x == charScope2 && counter == 1)) {
			counter = !counter;
		}
		else if (buf.find(value) != std::string::npos) {
			if (counter == 0) {
				std::string msg = buf.substr(0, buf.find_first_of(value));
				list.insert(list.end(), msg);
				buf.clear();
			}
		}
	}
	if (buf.size() != 0) list.insert(list.end(), buf);
	
	return list;
}


bool useRegex(std::string str, std::string regexText) {
	std::smatch matches;
	bool b = std::regex_search(str, matches, std::regex(regexText));
	
	// For some reason after 'useRegex' and 'HCL::matches' is out
	// of the function scope, the data gets corrupted and spews
	// out random memory in place of actual strings. Why?
	// No clue. This issue only appeared on the Windows version
	// of Clange (15.0.3, 15.0.2 and 14.0.0 has the same issue).
	// Strangely enough, Apple Clang 14 has 0 issues with
	// std::smatch so... win for Apple?
	//
	// Anyhow, instead of using std::smatch as the output,
	// I've decided to create a struct wrapper around std::vector
	// and add in a .str() function. With this approach all
	// code works as indeed without me changing the code much.
	// This also means we'll be using vectors for regex outputs,
	// which I don't particulary mind as std::vector seems to be
	// much more reliable than std::smatch.
	HCL::matches.clear();
	for (int i = 1; i < matches.size(); i++)
		HCL::matches.push_back(matches.str(i));

	return b;
}


bool useIterativeRegex(std::string str, std::string regexText) {
	std::regex s = std::regex(regexText);
    auto words_begin = std::sregex_iterator(str.begin(), str.end(), s);
    auto words_end = std::sregex_iterator();

	HCL::matches.clear();
    for (std::sregex_iterator i = words_begin; i != words_end; i++) {
        std::smatch match = *i;
		HCL::matches.push_back(match.str());
    }

	return (HCL::matches.size() != 0);
}


std::string removeSpaces(std::string str) {
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
	return str;
}


std::string unstringify(std::string str, bool noChecks/* = false*/, char character/* = '"'*/) {
	if (str[0] == character || noChecks)
		str.erase(0, 1);
	if (str[str.size() - 1] == character || noChecks)
		str.pop_back();

	return str;
}


std::string getPathFromFilename(std::string filename) {
	return filename.substr(0, filename.find_last_of("/\\"));
}


bool find(std::string line, std::string str) {
	return (line.find(str) != std::string::npos);
}


bool isInt(std::string str) {
  return str.find_first_not_of("0123456789+-") == std::string::npos;
}


std::string replace(std::string str, std::string oldString, std::string newString) {
    size_t pos;
    while ((pos = str.find(oldString, pos)) != std::string::npos) {
        str.replace(pos, oldString.length(), newString);
        pos += newString.length();
    }
	return str;
}


std::string replaceOnce(std::string str, std::string oldString, std::string newString) {
    str.replace(str.find(oldString), oldString.size(), newString);

    return str;
}


bool stringToBool(std::string str) {
    if (str == "true")
        return true;
    else if (str == "1")
        return true;
    else if (str == "0")
        return false;
    else if (str == "false")
        return false;
    else
		HCL::throwError(true, "Cannot convert '%s' to a bool", str.c_str());

	return false;
}


bool typeIsValid(std::string type, HCL::structure* info/* = NULL*/) {
	if (coreTyped(type)) return true;

	// Didn't find a core type, maybe it'll find a struct instead.
	for (auto s : HCL::structures) {
		if (type == s.name) {
			info->name = s.name;
			info->value = s.value;
			return true;
		}
	}

	return false; // Didn't find anything.
}


bool coreTyped(std::string type) {
	for (auto coreType : coreTypes) {
		if (type == coreType) return true;
	}
	
	return false;
}


HCL::variable* getVarFromName(std::string varName, HCL::variable* var/* = NULL*/) {
	HCL::structure s;
	for (auto& v : HCL::variables) {
		if (v.name == varName) {
			return &v;
		}

		if (typeIsValid(v.type, &s) && !coreTyped(v.type)) { // A custom type
			for (int i = 0; i < s.value.size(); i++) {
				auto member = s.value[i];
				if (varName == (v.name + "." + member.name)) {
					var->type = v.extra[i];
					var->name = varName;
					var->value = v.value;
					var->extra = {std::to_string(i)};
					return &v;
				}
			}
		}
	}
	return {};
}


int getValueFromFstring(std::string ogValue, std::string& output) {
	// Checks if the string is even an f-string
	if (ogValue.front() == 'f' && ogValue[1] == '\"' && ogValue.back() == '\"') {
		// Get every match of {words inside curly brackets}
		useIterativeRegex(ogValue, R"(\{[\w\.]+\})");
		ogValue.erase(0, 1); // Remove the F letter.
		for (auto value : HCL::matches.value) {
			value = unstringify(value, true); // Remove the curly brackets.
			HCL::variable structVar;
			HCL::variable* var = getVarFromName(value, &structVar);

			if (var == NULL) { // Can't use f-string without providing a variable obviously...
				HCL::throwError(true, "Variable '%s' doesn't exist.", value.c_str());
			}
			else {
				value = "{" + value + "}";
				size_t pos = ogValue.find(value);

				if (!structVar.name.empty()) { // Is a struct member.
					int index = std::stoi(structVar.extra[0]);
					ogValue = replaceOnce(ogValue, value, structVar.value[index]);
				}
				else // Regular variable.
					ogValue = replaceOnce(ogValue, value, var->value[0]);
			}
		}
		output = unstringify(ogValue);

		return 0;
	}
	return -1;
}