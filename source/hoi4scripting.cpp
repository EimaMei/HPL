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

    if (useRegex(line, R"(^\s*([^\s\(]+)\((.*)\)\s*$)") && HPL::matches.size() == 2) {
		HPL::function f; HPL::variable res;

		if (HPL::arg.debugAll || HPL::arg.debugLog) {
			std::cout << HPL::arg.curIndent << "HSM: LOG: [USE][FUNCTION]: " << HPL::curFile << ":" << HPL::lineCount << ": <name>(<params>): " << HPL::matches.str(1) << "(" << HPL::matches.str(2) << ")" << std::endl;
		}

		executeFunction(HPL::matches.str(1), HPL::matches.str(2), f, res);
		buffer += xToStr(res.value);
	}
	else
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


namespace HSM {
	std::string line;
	int equalBrackets = 1;
}