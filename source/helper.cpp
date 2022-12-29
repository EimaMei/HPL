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

#include <iostream>
#include <cstring>

// Each commit split just keeps getting more and more complex...
std::vector<std::string> split(std::string str, std::string value, std::string charScope/* = "\0\0"*/) {
	std::vector<std::string> list;
	std::string buf, buf2;
	int counter = 0;
	bool found;
	std::string lastScope = "";

	for (auto x : str) {
		found = false;
		buf += x;

		if (counter == 0)
			buf2 += x;
		else
			buf2 += '\0';

		for (int i = 0; i < charScope.size(); i += 2) {
			if (charScope[i] != '\0' && x == charScope[i] && (x != charScope[i + 1] || (x == charScope[i + 1] && !find(lastScope, std::string(1, x))))) {
				counter--;
				found = true;
				lastScope += charScope[i + 1];
			}
			else if (charScope[i + 1] != '\0' && x == charScope[i + 1] && !lastScope.empty() && find(lastScope, std::string(1, charScope[i + 1]))) {
				std::string d(1, charScope[i + 1]);
				counter++;
				found = true;
				lastScope = replaceOnce(lastScope, d, "");
			}
		}

		if (buf2.find(value) != std::string::npos && counter == 0 && !found) {
			std::string msg = buf.substr(0, buf2.find(value));
			list.insert(list.end(), msg);
			buf.clear();
			buf2.clear();
		}
	}
	if (buf.size() != 0) list.insert(list.end(), buf);

	return list;
}


bool useRegex(std::string str, std::string regexText) {
	std::smatch matches;
	bool res = std::regex_search(str, matches, std::regex(regexText));

	// For some reason after 'useRegex' and 'HPL::matches' is out
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
	HPL::matches.clear();
	for (int i = 1; i < matches.size(); i++)
		HPL::matches.push_back(matches.str(i));

	return res;
}


bool useIterativeRegex(std::string str, std::string regexText) {
	std::smatch match;
	HPL::matches.clear();
	bool res;
    while ((res = std::regex_search(str, match, std::regex(regexText)))) {
		HPL::matches.push_back(match.str(1));
        str = match.suffix();
    }

	return res;
}


std::string removeSpaces(std::string str) {
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
	return str;
}


std::string removeFrontAndBackSpaces(std::string str) {
	int i = 0, i2 = 0;

	for (const auto& c : str) { // Find the front spaces.
		if (c == ' ')
			i++;
		else
			break;
	}
	str = str.substr(i, str.size()); // Delete the front spaces.


	for (i2 = str.size() - 1; 0 < i2; ) { // Find the back spaces.
		if (str[i2] == ' ')
			i2--;
		else
			break;
	}
	str = str.substr(0, i2 + 1); // Delte the back spaces.

	return str;
}


std::string unstringify(std::string str, bool noChecks/* = false*/, char character/* = '"'*/) {
	if ((str.front() == 'f' && str[1] == '\"' && str.back() == '\"'))
		return str;

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
 	str = replaceAll(str, ".", ""); // For any possible floats.
    if (str.empty()) // String was just dots for whatever reason, not a decimal eitherway.
		return false;

    return str.find_first_not_of("0123456789+-/*()") == std::string::npos;
}


bool isStr(std::string str) {
	return (str.front() == '\"' && str.back() == '\"') || (str.front() == 'f' && str[1] == '\"' && str.back() == '\"');
}


std::string replaceAll(std::string str, std::string oldString, std::string newString) {
	size_t pos = 0;
	while ((pos = str.find(oldString, pos)) != std::string::npos) {
		str.replace(pos, oldString.length(), newString);
		pos += newString.length();
	}
	return str;
}


std::string replaceOnce(std::string str, std::string oldString, std::string newString) {
	if (find(str, oldString))
		str.replace(str.find(oldString), oldString.size(), newString);

	return str;
}


std::string convertBackslashes(std::string str) {
	char* here = (char*)str.c_str();
	size_t len = str.size();
	int num;
	int numlen;

	while (NULL != (here = strchr(here, '\\'))) {
		numlen = 1;
		switch (here[1]) {
			case '\\': *here = '\\'; break;
			case '\"': *here = '\"'; break;
			case 'r':  *here = '\r'; break;
			case 'n':  *here = '\n'; break;
			case 't':  *here = '\t'; break;
			case 'v':  *here = '\v'; break;
			case 'a':  *here = '\a'; break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				numlen = sscanf(here, "%o", &num);
				*here = (char)num;
				break;

			case 'x':
				numlen = sscanf(here, "%x", &num);
				*here = (char)num;
				break;
		}
		num = here - str.c_str() + numlen;
		here++;
		memmove(here, here + numlen, len - num);
	}
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
		HPL::throwError(true, "Cannot convert '%s' to a bool", str.c_str());

	return false;
}



double eval(std::string expr, int& errorCode) {
	errorCode = 0;
	return 0;
}


std::string xToStr(allowedTypes val) {
	if (std::holds_alternative<std::string>(val))
		return std::get<std::string>(val);

	else if (std::holds_alternative<int>(val))
		return std::to_string(std::get<int>(val));

	else if (std::holds_alternative<float>(val))
		return std::to_string(std::get<float>(val));

	else if (std::holds_alternative<bool>(val))
		return std::get<bool>(val) == true ? "true" : "false";

	else if (std::holds_alternative<std::vector<HPL::variable>>(val)) {
		std::string result = "{";
		auto& _struct = std::get<std::vector<HPL::variable>>(val);

		for (int i = 0; i < _struct.size(); i++) {
			auto& member = _struct[i];

			if (member.type == "string")
				result += '\"' + xToStr(member.value) + '\"';
			else
				result += xToStr(member.value);

			if (_struct.size() > 1 && ((i + 1) < _struct.size()))
				result += ", ";
		}
		result += "}";

		return result;
	}

	return std::string();
}


bool typeIsValid(std::string type, HPL::structure* info/* = NULL*/) {
	if (coreTyped(type))
		return true;

	// Didn't find a core type, maybe it'll find a struct instead.
	for (auto s : HPL::structures) {
		if (type == s.name) {
			if (info != nullptr) {
				info->name = s.name;
				info->value = s.value;
			}
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


std::string getTypeFromValue(std::string value) {
	if (isStr(value))
		return "string";
	else if (isInt(value) && !find(value, "."))
		return "int";
	else if (isInt(value) && find(value, "."))
		return "float";
	else if (value == "true" || value == "false")
		return "bool";
	else if (value.front() == '{' && value.back() == '}')
		return "struct";
	else if (value == "==" || value == "!=" || value == ">=" || value == "<=" || value == "<" || value == ">")
		return "relational-operator";
	else if (value == "&&" || value == "||")
		return "logical-operator";

	return "";
}


bool setCorrectValue(HPL::variable& var, std::string value) {
	HPL::variable* existingVar = getVarFromName(value);

	if ((var.type.empty() || var.type == "NO_TYPE") && existingVar == nullptr)
		var.type = getTypeFromValue(value);

	if (existingVar != nullptr) {
		if (var.type == "NO_TYPE") {
			var = *existingVar;
			return true;
		}

		if (existingVar->type == "string")
			value = '\"' + xToStr(existingVar->value) + '\"';
		else
			value = xToStr(existingVar->value);
	}

	if (var.type == "string" && isStr(value)) {
		getValueFromFstring(value, value);

		auto plusShenanigans = split(value, "+", "\"\"(){}"); // C's '+' strike again! We gotta organize everything ffs.
		std::string res;

		for (const auto& sentence : plusShenanigans) {
			std::string output = removeFrontAndBackSpaces(sentence);

			if (!isStr(output)) {
				HPL::variable uselessVar = {.type = "NO_TYPE"};

				setCorrectValue(uselessVar, output);
				output = xToStr(uselessVar.value);
			}

			res += unstringify(output);
		}

		var.value = convertBackslashes(res);
		return true;
	}

	else if (var.type == "int") {
		var.value = xToType<int>(value);
		return true;
	}

	else if (var.type == "float") {
		var.value = xToType<float>(value);
		return true;
	}

	else if (var.type == "bool") {
		var.value = stringToBool(value);
		return true;
	}

	else if (var.type == "struct" || (value.front() == '{' && value.back() == '}')) {
		useIterativeRegex(unstringify(value, true), R"(([^\,\s]+))");
		std::vector<HPL::variable> output;
		HPL::variable coreTypedVariable;
		auto oldMatches = HPL::matches.value;

		for (auto& v : oldMatches) {
			HPL::variable* var = getVarFromName(v);
			coreTypedVariable.reset_all();

			if (var != nullptr)
				output.push_back(*var);

			else if (setCorrectValue(coreTypedVariable, v))
				output.push_back(coreTypedVariable);
			
			else if (v.empty())
				output.push_back({});

			else
				HPL::throwError(true, "Variable '%s' doesn't exist (Cannot set a member to something that doesn't exist)", v.c_str());
		}
		var.value = output;

		if (var.type.empty() || var.type == "NO_TYPE") {
			var.type = "struct"; // We'll deal with this later in the code.
		}

		return true;
	}
	else if (useRegex(value, R"(^\s*([^\s\(]+)\((.*)\)\s*$)")) {
		assignFuncReturnToVar(&var, HPL::matches.str(1), HPL::matches.str(2));
		return true;
	}
	/*else if (!(res = extractMathFromValue(value, existingVar)).empty()) // A math expression.
		value = res;*/

	else if (var.type == "relational-operator" || var.type == "logical-operator") {
		var.value = value;
		return true;
	}

	return false;
}


HPL::variable* getVarFromName(std::string varName) {
	for (auto& v : HPL::variables) {
		if (v.name == varName) {
			return &v;
		}

		if (find(varName, v.name + ".")) { // A custom type
			HPL::structure* s = getStructFromName(v.type);
			auto& varValues = getVars(v.value);

			for (int i = 0; i < varValues.size(); i++) {
				auto& member = s->value[i];

				if (varName == (v.name + "." + member.name)) {
					varValues[i].name = member.name;
					return &varValues[i];
				}
			}
		}
	}
	return nullptr;
}


HPL::structure* getStructFromName(std::string name) {
	for (auto& s : HPL::structures) {
		if (s.name == name)
			return &s;
	}

	return nullptr;
}


int getValueFromFstring(std::string ogValue, std::string& output) {
	// Checks if the string is even an f-string.
	if (ogValue.front() == 'f' && ogValue[1] == '\"' && ogValue.back() == '\"') {
		// Get every match of {words inside curly brackets}.
		useIterativeRegex(ogValue, R"(\{([\w\(\)\[\]\.]+)\})");
		ogValue.erase(0, 1); // Remove the F letter.


		for (auto value : HPL::matches.value) {
			HPL::variable var = {.type = "NO_TYPE"};
			bool res = setCorrectValue(var, value);

			if (!res) // Can't use f-string without providing any valid value obviously...
				HPL::throwError(true, "Argument '%s' is invalid (Either it's a variable that doesn't exist or something else entirely)", value.c_str());
			else {
				value = "{" + value + "}";
				ogValue = replaceOnce(ogValue, value, xToStr(var.value));
			}
		}
		output = ogValue;

		return 0;
	}
	return -1;
}


std::string extractMathFromValue(std::string expr, HPL::variable* var) {
	int mathError;
	double res = eval(expr, mathError);

	if (mathError != -1) { // Actual math is in the value.
		if (var->type == "int")
			return std::to_string((int)res);
		else if (var->type == "float")
			return std::to_string((float)res);
	}

	return "";
}


std::string printFunction(HPL::function func) {
	std::string str = func.type + " " + func.name + "(";

	for (int i = 0; i < func.params.size(); i++) {
		auto& p = func.params[i];
		str += p.type + " " + p.name;

		if (p.has_value()) {
			str += " = ";
			if (p.type == "string")
				str += '\"' + xToStr(p.value) + '\"';
			else
				str += xToStr(p.value);
		}
		if (i != func.params.size() - 1)
			str += ", ";
	}
	str += ")";

	return str;
}