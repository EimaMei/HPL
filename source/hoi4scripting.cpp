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

#include <scope/hoi4scripting.hpp>

#include <interpreter.hpp>
#include <core.hpp>
#include <helper.hpp>

#include <iostream>


std::string HSM::interpreteLine(std::string str) {
	std::string buffer;
	std::string tabs;
	bool found = false;

	if (find(str, "#")) // Ignore comments
		str = split(str, "#", "\"\"")[0];

	line = str;

	bool leftBracket = useRegex(line, R"(^\s*(\})\s*$)");
	bool rightBracket = useRegex(line, R"(^.*\s*\=\s*\{\s*$)");


	if (leftBracket)
		equalBrackets--;

	for (int i = 1; i < equalBrackets; i++)
		tabs += '\t';

	if (rightBracket)
		equalBrackets++;



	//if (checkModes(buffer) == FOUND_SOMETHING) found = true;
	if (checkConditions(buffer) == FOUND_SOMETHING) found = true;
	else if (checkFunctions(buffer) == FOUND_SOMETHING) found = true;
	//else if (checkVariables(buffer) == FOUND_SOMETHING) found = true;

	if (!found)
		buffer += line;

	if (leftBracket && equalBrackets == 0) {
		HPL::mode = MODE_DEFAULT;
		HPL::variables[0].value = false;

		auto& value = getStr(HPL::variables[HPL::scopeIndex].value);
		value.pop_back();

		return std::string{};
	}
	else
		buffer += "\n";

	buffer = tabs + removeFrontAndBackSpaces(buffer);


	return buffer;
}


int HSM::checkConditions(std::string& buffer) {
	if (useRegex(line, R"(^\s*if\s*(.*)\s*$)")) {
		std::string oldValue = removeFrontAndBackSpaces(HPL::matches.str(1));

		if (oldValue.back() == '{') {
			oldValue.pop_back();
		}
		else {
			//HPL::mode = MODE_CHECK_IF_STATEMENT;
		}

		oldValue = removeFrontAndBackSpaces(oldValue);
		auto params = split(oldValue, " ", "\"\"{}()"); // NOTE: Need to a fix a bug when there's no space between an operator and two values (eg. 33=="@34")

		std::string _operator;
		std::string additionalSyntax, additionalSyntax2;

		// HSM stuff
		buffer += "if = {\n\tlimit = {";
		equalBrackets++;

		for (int i = 0; i < params.size(); i++) {
			auto& p = params[i];
			p = removeFrontAndBackSpaces(p);

			HPL::variable var;
			setCorrectValue(var, p, false);
			p = xToStr(var.value);

			if (_operator == "==" || _operator == "!=") {
				bool res = (_operator == "==" ? stringToBool(p) : !stringToBool(p));

				if (!res) {
					additionalSyntax = "NOT = { ";
					additionalSyntax2 += " }";
					_operator.clear();
				}
				p.clear();
			}
			else if (p == "&&")
				continue;
			else if (p == "||") {
				additionalSyntax = "OR = { ";
				additionalSyntax2 = " }";
				_operator = "||";
				continue;
			}
			else if (_operator.empty()) {
				additionalSyntax.clear();
				additionalSyntax2.clear();
			}

			if (i + 1 < params.size()) {
				auto value = params[i + 1];

				if (value == "==" || value == "!=" || value == ">=" || value == "<=" || value == "<" || value == ">") {
					additionalSyntax2 = p;
					_operator = value;
					i++;
					continue;
				}
			}

			buffer += "\n\t\t" + additionalSyntax + p + additionalSyntax2;

			/*if (!_operator.empty()) {
				bool res = false;

				if (_operator == "==") {
					res = (params[i - 2] == p);
				else if (_operator == "!=")
					res = (params[i - 2] != p);
				else if (_operator == ">=")
					res = (params[i - 2] >= p);
				else if (_operator == "<=")
					res = (params[i - 2] <= p);
				else if (_operator == ">")
					res = (params[i - 2] >  p);
				else if (_operator == "<")
					res = (params[i - 2] <  p);

				if (HPL::arg.debugAll || HPL::arg.debugLog) {
					std::cout << arg.curIndent << "LOG: [OPERATOR][RELATION]: " << curFile << ":" << lineCount << ": <value 1> <operator> <value 2>: " << params[i - 2] << " " << _operator << " " <<  p << " (" << (res == true ? "true" : "false") << ")" << std::endl;
				}
			}*/
		}
		buffer += "\n\t}";

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


int HSM::checkFunctions(std::string& buffer) {
	if (useRegex(line, R"(^\s*([^\s\(]+)\((.*)\)\s*$)") && HPL::matches.size() == 2) {
		HPL::function f; HPL::variable res;

		if (HPL::arg.debugAll || HPL::arg.debugLog) {
			std::cout << HPL::arg.curIndent << "HSM: LOG: [USE][FUNCTION]: " << HPL::curFile << ":" << HPL::lineCount << ": <name>(<params>): " << HPL::matches.str(1) << "(" << HPL::matches.str(2) << ")" << std::endl;
		}

		executeFunction(HPL::matches.str(1), HPL::matches.str(2), f, res);
		buffer += xToStr(res.value);

		return FOUND_SOMETHING;
	}

	return FOUND_NOTHING;
}


namespace HSM {
	std::string line;
	int equalBrackets = 1;
}