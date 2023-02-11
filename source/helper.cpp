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

#include <iostream>
#include <sstream>
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
				if (buf[buf.size() - 2] == '\\' && x == '\"')
					break;

				counter--;
				found = true;
				lastScope += charScope[i + 1];
			}
			else if (charScope[i + 1] != '\0' && x == charScope[i + 1] && !lastScope.empty() && find(lastScope, std::string(1, charScope[i + 1]))) {
				if (buf[buf.size() - 2] == '\\' && x == '\"')
					break;

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
	std::cout << colorText("WARNING: ", HPL::OUTPUT_RED) << "Using outdated function 'useRegex' at line '" << colorText(std::to_string(HPL::lineCount), HPL::OUTPUT_YELLOW) << "' (regex_text = " << regexText << ")" << std::endl;
	std::smatch matches;
	bool res = std::regex_search(str, matches, std::regex(regexText));

	// Note (as of 2023-01-16/HPL 0.2.7): It's very discouraged to use 'useRegex'
	// and to instead create your own parser function. Mainly because the function
	// is very performance heavy as not only does it use regex for the parsing,
	// but also has to copy contents from std::matches to HPL::vector after each
	// function usage. This function is only kept for backwards compatibility with
	// older code.
	//
	// Old notes: For some reason after 'useRegex' and 'HPL::matches' is out
	// of the function scope, the data gets corrupted and spews
	// out random memory in place of actual strings. Why?
	// No clue. This issue only appeared on the Windows version
	// of Clang (15.0.3, 15.0.2 and 14.0.0 has the same issue).
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
	for (int i = 1; i < matches.size(); i++) {
		if (!matches.str(i).empty())
			HPL::matches.push_back(matches.str(i));
	}

	return res;
}


bool useIterativeRegex(std::string str, std::string regexText) {
	std::cout << colorText("WARNING: ", HPL::OUTPUT_RED) << "Using outdated function 'useIterativeRegex' at line '" << colorText(std::to_string(HPL::lineCount), HPL::OUTPUT_YELLOW) << "' (regex_text = " << regexText << ")" << std::endl;

	// Note (as of 2023-01-16/HPL 0.2.7): It's very discouraged to use 'useIterativeRegex'
	// and to instead create your own parser function. Mainly because the function
	// is very performance heavy as not only does it use regex for the parsing,
	// but also has to copy contents from std::matches to HPL::vector after each
	// function usage. This function is only kept for backwards compatibility with older
	// code.

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
		if (c == ' ' || c == '\t')
			i++;
		else
			break;
	}
	str = str.substr(i, str.size()); // Delete the front spaces.


	for (i2 = str.size() - 1; 0 < i2; ) { // Find the back spaces.
		if (str[i2] == ' ' || str[i2] == ';')
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


std::string removeFrontAndBackLetters(std::string str) {
	str.erase(0, 1);
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
	for( size_t pos = 0; ; pos += newString.length() ) {
		// Locate the substring to newString
		pos = str.find( oldString, pos );
		if( pos == std::string::npos ) break;
		// Replace by erasing and inserting
		str.erase( pos, oldString.length() );
		str.insert( pos, newString );

	}

	return str;
}


std::string replaceOnce(std::string str, std::string oldString, std::string newString) {
	if (find(str, oldString))
		str.replace(str.find(oldString), oldString.size(), newString);

	return str;
}


std::string convertBackslashes(std::string str) {
	size_t pos = 0;

	while ((pos = str.find('\\', pos)) != std::string::npos) {
		switch (str[pos + 1]) {
			case '\\': str.replace(pos, 2, "\\"); break;
			case '\"': str.replace(pos, 2, "\""); break;
			case 'r':  str.replace(pos, 2, "\r"); break;
			case 'n':  str.replace(pos, 2, "\n"); break;
			case 't':  str.replace(pos, 2, "\t"); break;
			case 'v':  str.replace(pos, 2, "\v"); break;
			case 'a':  str.replace(pos, 2, "\a"); break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				int num;
				sscanf_s(str.c_str(), "%o", &num);
				str.replace(pos, 2, (&"\\"[num]));
				break;
		}
		pos++;
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
	else {
		HPL::variable var;
		bool b = setCorrectValue(var, str, false);

		if (!b)
			HPL::throwError(true, "Cannot convert '%s' to a bool", str.c_str());

		return xToType<bool>(var.value);
	}

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

	else if (std::holds_alternative<float>(val)) {
		std::ostringstream ss;
		ss << std::get<float>(val);

		return ss.str();
	}

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
	else if (std::holds_alternative<HPL::variable*>(val))
		return xToStr(getPVar(val)->value);

	return std::string{};
}


bool typeIsValid(std::string type, HPL::structure* info/* = nullptr*/) {
	if (coreTyped(type))
		return true;

	// Didn't find a core type, maybe it'll find a struct instead.
	for (auto& s : HPL::structures) {
		if (type == s.name) {
			if (info != nullptr)
				info = &s;
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

	return std::string{};
}


bool setCorrectValue(HPL::variable& var, std::string value, bool onlyChangeValue, HPL::variable** pointerToOriginalVariable/* = nullptr*/) {
	HPL::variable* existingVar;
	HPL::structure* s = nullptr;
	bool result = false;
	bool changeValue = false;
	char frontChar = value.front();

	if (!isalpha(frontChar) && !isdigit(frontChar) && (frontChar != '\"' && frontChar != '{' && frontChar != '|') && (value.size() > 1 && value[1] != '=')) {
		if (!((frontChar == '-' || frontChar == '+') && isInt(value))) {
			changeValue = true;
			value = value.substr(1, -1);
		}
	}

	existingVar = getVarFromName(value);


	/*
	MAJOR NOTE: Fix the issue when the first word isn's a string, even though there's a plus behind it (eg. 343243 + "bfdjsgdfg")

	if (!value.empty()) {
		if (find(value, "+")) {
			auto findStr = split(value, "+", "\"\"{}()");
			std::string buffer;

			for (auto& str : findStr) {
				str = removeFrontAndBackSpaces(str);
				if (isStr(str)) {
					setCorrectValue(var, str, true);
					buffer += xToStr(var.value);
					break;
				}
				else
					buffer +=
			}
		}
	}*/

	if ((var.type.empty() || var.type == "auto") && existingVar == nullptr)
		var.type = getTypeFromValue(value);

	if (value.empty() && existingVar == nullptr && var.type != "scope" && typeIsValid(var.type, s))
		return false;

	if (changeValue)
		setUnaryOperator(var, existingVar, value, frontChar);

	if (existingVar != nullptr) {
		if (!onlyChangeValue)
			var = *existingVar;
		else
			var.value = existingVar->value;

		if (pointerToOriginalVariable != nullptr)
			*pointerToOriginalVariable = existingVar;
	}
	else if (pointerToOriginalVariable != nullptr)
		*pointerToOriginalVariable = nullptr;

	if (s != nullptr) { // The returned type from `typeIsValid` returned a struct
		if (value.empty()) { // Nothing is set, meaning it's just the struct's default arguments.
			std::vector<HPL::variable> res = s->value;
			var.value = res;

			result = true;
		}
		else if (value.front() == '{' && value.back() == '}') { // Oh god oh fuck it's a custom list.
			// We don't split the string by the comma if the comma is inside double quotes
			// Meaning "This, yes this, exact test string" won't be split. We also remove the curly brackets before splitting.
			std::vector<std::string> valueList = split(unstringify(value, true), ",", "(){}\"\"");
			std::vector<HPL::variable> result;

			if (valueList.size() > s->value.size()) {
				HPL::throwError(true, "Too many values are provided when declaring the variable '%s' (you provided '%i' arguments when struct type '%s' has only '%i' members).", var.name.c_str(), valueList.size(), var.type.c_str(), s->value.size());
			}

			// The first removes the spaces, then the double quotes.
			for (int i = 0; i < s->value.size(); i++) {
				if (i < s->value.size() && i < valueList.size()) {
					HPL::variable var;
					setCorrectValue(var, removeFrontAndBackSpaces(valueList[i]), true);

					result.push_back(var);
				}
				else { // Looks like the user didn't provide the entire argument list. That's fine, though we must check for any default options.
					if (s->value[i].has_value()) {
						result.push_back(s->value[i]);
					}
					else if (HPL::arg.strict)
						HPL::throwError(true, "Too few values are provided to fully initialize a struct (you provided '%i' arguments when struct type '%s' has '%i' members).", valueList.size(), var.type.c_str(), s->value.size());
					else
						HPL::throwError(true, "Shouldn't happen?");
				}
			}
			var.value = result;
		}
	}


	if (s != nullptr || existingVar != nullptr)
		result = true;

	else if (var.type == "scope") {
		var.reset_value();

		if (!(value == "{}" || value.empty())) {
			HPL::mode = (value == "{" ? MODE_SAVE_SCOPE : MODE_CHECK_SCOPE);
			HPL::variables[0].value = true; // Scope mode is ON!
		}

		result = true;
	}

	else if (var.type == "string") {
		if (!isStr(value))
			std::cout << "Hm: " << value << std::endl;

		getValueFromFstring(value, value);

		auto plusShenanigans = split(value, "+", "\"\"(){}"); // C's '+' strike again! We gotta organize everything ffs.
		std::string res;

		for (const auto& sentence : plusShenanigans) {
			std::string output = removeFrontAndBackSpaces(sentence);

			if (!isStr(output)) {
				HPL::variable uselessVar;

				setCorrectValue(uselessVar, output, false);
				output = xToStr(uselessVar.value);
			}

			res += unstringify(output);
		}

		var.value = convertBackslashes(res);
		result = true;
	}

	else if (var.type == "int") {
		var.value = xToType<int>(value);
		result = true;
	}

	else if (var.type == "float") {
		var.value = xToType<float>(value);
		result = true;
	}

	else if (var.type == "bool") {
		var.value = stringToBool(value);
		result = true;
	}
//
	else if (var.type == "struct" || (value.front() == '{' && value.back() == '}')) {
		useIterativeRegex(unstringify(value, true), R"(([^\,\s]+))"); // get the members.

		HPL::structure* _struct = getStructFromName(var.type);
		std::vector<HPL::variable> output;
		auto oldMatches = HPL::matches.value;
		int index = 0;

		for (auto& v : oldMatches) {
			HPL::variable* var = getVarFromName(v);
			HPL::variable coreTypedVariable;

			if (_struct != nullptr)
				coreTypedVariable = _struct->value[index];

			if (var != nullptr)
				output.push_back(*var);

			else if (setCorrectValue(coreTypedVariable, v, false))
				output.push_back(coreTypedVariable);

			else if (v.empty() && _struct != nullptr)
				output.push_back(_struct->value[index]);

			else if (v.empty())
				output.push_back({});

			else
				HPL::throwError(true, "Variable '%s' doesn't exist (Cannot set a member to something that doesn't exist)", v.c_str());

			index++;
		}

		if (_struct != nullptr && _struct->value.size() > index + 1) {
			for (int i = index; i < _struct->value.size(); i++) {
				output.push_back(_struct->value[i]);
			}
		}

		var.value = output;

		if (var.type.empty())
			var.type = "struct"; // We'll deal with this later in the code.

		result = true;
	}
	//else if (useRegex(value, R"(^\s*([^\s\(]+)\((.*)\)\s*$)")) {
	//	assignFuncReturnToVar(&var, HPL::matches.str(1), HPL::matches.str(2));
	//	result = true;
//
	//	if (changeValue)
	//		setUnaryOperator(var, existingVar, value, frontChar);
	//}
	/*else if (!(result = extractMathFromValue(value, existingVar)).empty()) // A math expresultsion.
		value = result;*/

	else if (var.type == "relational-operator" || var.type == "logical-operator") {
		var.value = value;
		result = true;
	}

	if (HPL::arg.debugAll || HPL::arg.debugLog) {
		std::string buffer;
		if (existingVar)
			buffer = printVar(*existingVar) + " (" + (result ? "true" : "false") + ")";
		else
			buffer = printVar(var) + " (" + (result ? "true" : "false") + ")";

		std::cout << HPL::arg.curIndent  << HPL::colorText("LOG: [FUNCTION][SET-CORRECT-VALUE]: ", HPL::OUTPUT_PURPLE) << HPL::curFile << ":" << HPL::lineCount << ": <og value> = <info> (<is set>): " << value << " = " << buffer << std::endl;
	}

	return result;
}


HPL::variable* getVarFromName(std::string varName) {
	for (auto& v : HPL::variables) {
		if (v.name == varName) {
			auto value = xToStr(v.value);

			if (value == "{}") {
				HPL::structure* s = getStructFromName(v.type);
				if (s != nullptr)
					v.value = s->value;
			}

			return &v;
		}

		else if (find(varName, v.name + ".")) { // A custom type
			HPL::structure* s = getStructFromName(v.type);

			if (s == nullptr) // Was a false-positive after all, goddamn...
				return nullptr;

			auto listOfMembers = split(varName, ".", "(){}\"\"");
			auto pointer = &v.value;

			std::string baseName;
			std::string baseType, oldBaseType = v.type;

			for (int memberIndex = 0; memberIndex < listOfMembers.size(); memberIndex++) {
				baseName += listOfMembers[memberIndex] + ".";

				for (int varIndex = 0; varIndex < s->value.size(); varIndex++) {
					auto& member = s->value[varIndex];
					baseType = member.type;


					if (find(varName, (baseName + member.name))) {
						if (varName == (baseName + member.name)) {
							if (v.has_value()) {
								auto& values = getVars(*pointer);
								HPL::variable* valueLocation;
								std::string value;

								if (varIndex < values.size())
									valueLocation = &values[varIndex];
								else
									valueLocation = &s->value[varIndex];

								value = xToStr(valueLocation->value);

								valueLocation->name = member.name;

								if (value == "{}")
									valueLocation->value = s->value[varIndex].value;

								return valueLocation;
							}
							else {
								auto* valueLocation = &s->value[varIndex];
								valueLocation->name = member.name;

								return valueLocation;
							}
						}
						else {
							s = getStructFromName(member.type);
							if (v.has_value()) {
								std::vector<HPL::variable>& varValues = getVars(v.value);
								pointer = &varValues[varIndex].value;
							}
							else {
								v.value = std::vector<HPL::variable>{};
								std::vector<HPL::variable>& varValues = getVars(v.value);
								pointer = &varValues[varIndex].value;
							}
							varIndex = 0;
						}
					}
				}
			}
		}
	}

	return nullptr;
}


void setUnaryOperator(HPL::variable& var, HPL::variable*& existingVar, std::string& value, char& frontChar) {
	if (!var.type.empty() && !(var.type == "int" || var.type == "bool" || var.type == "float" || var.type == "scope"))
		HPL::throwError(true, "Cannot use operator NOT(!) for %s values (Can only use the operator for bool/int/float types, not %s)", var.type.c_str(), var.type.c_str());
	HPL::variable* initVar = &var;

	if (existingVar != nullptr)
		initVar = existingVar;

	if (initVar->type == "bool") {
		if (frontChar == '!')
			initVar->value = !getBool((*initVar));
		else if (frontChar == '-')
			initVar->value = -getBool((*initVar));
		else if (frontChar == '+')
			initVar->value = +xToType<float>(initVar->value);
		else
			HPL::throwError(true, "Unary operator '%c' doesn't exist (You can use !/-/+, but not '%c')", &frontChar, &frontChar);
	}
	else if (initVar->type == "int" || initVar->type == "float") {
		if (frontChar == '!')
			initVar->value = !xToType<float>(initVar->value);
		else if (frontChar == '-')
			initVar->value = -xToType<float>(initVar->value);
		else if (frontChar == '+')
			initVar->value = +xToType<float>(initVar->value);
		else
			HPL::throwError(true, "Unary operator '%c' doesn't exist (You can use !/-/+, but not '%c')", &frontChar, &frontChar);

		if (existingVar->type == "int")
			initVar->value = (int)getFloat((*initVar));
	}
	else if (initVar->type == "scope")
		initVar->value = "NOT = { " + getStr((*initVar)) + "}";

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
			HPL::variable var;
			bool res = setCorrectValue(var, value, false);

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


std::string printVar(HPL::variable var) {
	std::string str = var.type + " " + var.name;

	if (var.has_value()) {
		str += " = ";
		if (var.type == "string")
			str += "\"" + replaceAll(xToStr(var.value), R"(\)", "\\") + "\"";
		else
			str += replaceAll(xToStr(var.value), R"(\)", "\\");
	}

	return str;
}


HPL::timer startTimer() {
	auto start = std::chrono::system_clock::now();
	return HPL::timer{.id = start};
}


void updateTimer(HPL::timer& t) {
	int h = 0, m = 0, s = 0, ms = 0;

	std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - t.id;
	double time =  elapsed_seconds.count();

	std::string str = std::to_string(time);
	str = str.substr(str.find(".") + 1, -1).substr(0, 2);
	ms = std::stoi(str);

	if (m >= 60)
		h = m / 60;
	if (time >= 60.0)
		m = abs(time) / 60;

	s = time - (m * 60);

	t = {h, m, s, ms, t.past, t.id};
}


bool compareVars(HPL::variable& var1, HPL::variable& var2, std::string symbol) {
	if (var1.type == "string") {
		if (var2.type != "string")
			HPL::throwError(true, "baded");

		var2.value = xToStr(var2.value);

		switch (operatorList[symbol]) {
			case equal_equal:   return (getStr(var1) == getStr(var2));
			case not_equal:     return (getStr(var1) != getStr(var2));
			case greater_than:  return (getStr(var1) >  getStr(var2));
			case lesser_than:   return (getStr(var1) <  getStr(var2));
			case equal_greater: return (getStr(var1) >= getStr(var2));
			case equal_lesser:  return (getStr(var1) <= getStr(var2));
			default: break;
		}
	}
	else if (var1.type == "int") {
		if (var2.type == "string")
			HPL::throwError(true, "baded");

		var2.value = xToType<int>(var2.value);

		switch (operatorList[symbol]) {
			case equal_equal:   return (getInt(var1) == getInt(var2));
			case not_equal:     return (getInt(var1) != getInt(var2));
			case greater_than:  return (getInt(var1) >  getInt(var2));
			case lesser_than:   return (getInt(var1) <  getInt(var2));
			case equal_greater: return (getInt(var1) >= getInt(var2));
			case equal_lesser:  return (getInt(var1) <= getInt(var2));
			default: break;
		}
	}
	else if (var1.type == "float") {
		if (var2.type == "string")
			HPL::throwError(true, "baded");

		var2.value = xToType<float>(var2.value);

		switch (operatorList[symbol]) {
			case equal_equal:   return (getFloat(var1) == getFloat(var2));
			case not_equal:     return (getFloat(var1) != getFloat(var2));
			case greater_than:  return (getFloat(var1) >  getFloat(var2));
			case lesser_than:   return (getFloat(var1) <  getFloat(var2));
			case equal_greater: return (getFloat(var1) >= getFloat(var2));
			case equal_lesser:  return (getFloat(var1) <= getFloat(var2));
			default: break;
		}
	}
	else if (var1.type == "bool") {
		if (var2.type == "string")
			HPL::throwError(true, "baded");

		var2.value = xToType<bool>(var2.value);

		switch (operatorList[symbol]) {
			case equal_equal:   return (getBool(var1) == getBool(var2));
			case not_equal:     return (getBool(var1) != getBool(var2));
			case greater_than:  return (getBool(var1) >  getBool(var2));
			case lesser_than:   return (getBool(var1) <  getBool(var2));
			case equal_greater: return (getBool(var1) >= getBool(var2));
			case equal_lesser:  return (getBool(var1) <= getBool(var2));
			default: break;
		}
	}
	else if (var1.type == "scope")
		HPL::throwError(true, "very baded");

	return false;
}


std::ostream& operator<<(std::ostream &os, const HPL::timer& t) {
	return os << t.h << ":" << t.min << ":" << t.s << ":" << t.ms;
}