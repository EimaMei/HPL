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

#include <parser.hpp>
#include <core.hpp>
#include <helper.hpp>

#include <iostream>


bool parseVariable(const std::string& line, HPL::vector& matches) {
	bool spaces = true;
	bool string = false;
	std::string buffer;

	for (int i = 0; i < line.size(); i++) {
		const auto& x = line[i];

		if (!string && (x == '+' || x == '-' || x == '*' || x == '/' || x == '=' || x == '<' || x == '>' || x == '!')) {
			if (matches.str(1) == "#include" && (x == '<' || x == '>')) {
				buffer += x;
				continue;
			}


			if (!buffer.empty()) {
				matches.value.push_back(buffer);
				buffer.clear();
			}

			buffer += x;

			while (true) {
				i++;

				const auto& nextX = line[i];
				if (nextX == '+' || nextX == '-' || nextX == '*' || nextX == '/' || nextX == '=') {
					buffer += std::string(1, nextX);
					break;
				}
				else if (nextX == ' ') {
					if (x == '=')
						break;
					else
						continue;
				}
				else {
					if (x == '=' || x == '<' || x == '>' || x == '!') {
						i--;
						break;
					}

					HPL::throwError(true, "baded");
				}
			}

			if (operatorList.find(buffer) != operatorList.cend()) {
				matches.value.push_back(buffer);
				spaces = true;
				buffer.clear();
			}
			else
				HPL::throwError(true, "wtf did you just do?: %s", buffer.c_str());
		}

		else if (x != ' ') {
			spaces = false;
			buffer += x;

			if (x == '\"') {
				if (!string)
					string = true;
				else if (i == line.size() - 1)
					string = false;
			}
		}
		else if (x == ' ' && !spaces) {
			if (string) {
				buffer += x;
				break;
			}
			matches.value.push_back(buffer);
			spaces = true;
			buffer.clear();
		}
	}
	if (!buffer.empty())
		matches.value.push_back(buffer);

    return matches.size() != 0;
}


bool fixIfStatements(std::vector<std::string>& params, const bool alreadyWent/* = false*/) {
	std::string _operator;
	bool failed = false;
	HPL::variable previousVar;
	std::string param;

	for (int i = 0; i < params.size(); i++) {
		auto& p = params[i];
		if (!alreadyWent)
			p = removeFrontAndBackSpaces(p);

		HPL::variable var;
		if (!setCorrectValue(var, p, false))
			HPL::throwError(true, "Variable '%s' doesn't exist (Cannot check the value from something that doesn't exist).", p.c_str());
		param = xToStr(var.value);

		failed = ((param == "false" || param == "0") && _operator.empty());

		if (i + 1 < params.size() && _operator.empty()) {
			if (operatorList.find(params[i + 1]) != operatorList.end()) {
				i++;
				_operator = params[i];
				previousVar = var;
				continue;
			}
			else if (failed && params[i + 1] == "&&") {
				break;
			}
			else if (!failed && params[i + 1] == "||") {
				continue;
			}
			else if (failed) {
				break;
			}
		}

		else if (!_operator.empty()) {
			failed = !compareVars(previousVar, var, _operator);

			if (HPL::arg.debugAll || HPL::arg.debugLog)
				std::cout << HPL::arg.curIndent << "LOG: [OPERATOR][RELATION]: " << HPL::curFile << ":" << HPL::lineCount << ": <value 1> <operator> <value 2>: " << xToStr(previousVar.value) << " " << _operator << " " << xToStr(var.value) << " (" << (failed != true ? "true" : "false") << ")" << std::endl;

			_operator.clear();

			if (failed) {
				if (i + 1 < params.size() && params[i + 1] == "||") {
					failed = false;
					continue;
				}
			}
			break;
		}
	}

    return !failed;
}


bool fixIfStatements(std::vector<HPL::object>& params) {
	std::string _operator;
	bool failed = false;
	HPL::variable var, previousVar;
	std::string param;

	for (int i = 0; i < params.size(); i++) {
		auto& execute = params[i];


		if (execute.isVar)
			var = *getPVar(execute.value);
		else
			var.value = execute.value;


		param = xToStr(var.value);
		failed = ((param == "false" || param == "0") && _operator.empty());

		//std::cout << i << " | " << param << " | " << xToStr(previousVar.value) << std::endl;

		if (i + 1 < params.size() && _operator.empty()) {
			std::string upcomingParam = xToStr(params[i + 1].value);

			//std::cout << upcomingParam << std::endl;

			if (operatorList.find(upcomingParam) != operatorList.end()) {
				i++;
				_operator = upcomingParam;
				previousVar = var;
				continue;
			}
			else if (failed && upcomingParam == "&&") {
				break;
			}
			else if (!failed && upcomingParam == "||") {
				i++;
				continue;
			}
			else if (failed) {
				break;
			}
		}

		else if (!_operator.empty()) {
			failed = !compareVars(previousVar, var, _operator);

			//std::cout << failed << " " << xToStr(previousVar.value) << _operator << xToStr(var.value) << "\n";

			if (HPL::arg.debugAll || HPL::arg.debugLog)
				std::cout << HPL::arg.curIndent << "LOG: [OPERATOR][RELATION]: " << HPL::curFile << ":" << HPL::lineCount << ": <value 1> <operator> <value 2>: " << xToStr(previousVar.value) << " " << _operator << " " << xToStr(var.value) << " (" << (failed != true ? "true" : "false") << ")" << std::endl;

			_operator.clear();

			if (failed) {
				std::string upcomingParam = xToStr(params[i + 1].value);

				if (i + 1 < params.size() && upcomingParam == "||") {
					failed = false;
					continue;
				}
			}
			break;
		}
	}

    return !failed;
}