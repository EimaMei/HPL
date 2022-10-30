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


std::vector<std::string> split(std::string str, std::string value, char charScope/* = NULL*/) {
	std::vector<std::string> list;
	std::string buf;
	int counter = 0;

	for (auto x : str) {
		buf += x;
		if (charScope != '\0' && x == charScope) {
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
	list.insert(list.end(), buf);
	
	return list;
}


bool useRegex(std::string str, std::string regexText) {
	std::smatch match;
	bool b = std::regex_search(str, match, std::regex(regexText));
	
	// For some reason after 'useRegex' and the matches it out
	// of the function scope, the data gets corrupted and spews
	// out random memory in place of actual strings. Why?
	// No clue. Apple Clang15 doesn't seem to have this issue,
	// but the version seems to have this bug, so I am forced
	// to use a vector instead of std::smatch. Doesn't change
	// much, however it forces me to be more careful with certain
	// matches that aren't 100% guaranteed to appear.
	HCL::matches.value.clear();
	for (int i = 1; i < match.size(); i++)
		HCL::matches.value.push_back(match.str(i));

	return b;
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


std::string replace(std::string str, char oldValue, char newValue) {
    std::replace(str.begin(), str.end(), oldValue, newValue);

    return str;
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