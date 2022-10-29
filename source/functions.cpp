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
#include <functions.hpp>
#include <iostream>

std::vector<std::string> split(std::string str, std::string value, char charScope/* = NULL*/) {
	std::vector<std::string> list;
	std::string buf;
	int counter = 0;

	for (auto x : str) {
		buf += x;
		if (charScope != '\0' && x == charScope) {
			counter = !counter;
		}
		if (buf.find(value) != std::string::npos) {
			if (counter == 0) {
				std::string msg = buf.substr(0, buf.find_last_of(value));
				list.insert(list.end(), msg);
				buf.clear();
			}
			else buf.pop_back();
		}
	}
	list.insert(list.end(), buf);
	
	return list;
}


bool useRegex(std::string str, std::string regexText) {
	return std::regex_search(str, HCL::matches, std::regex(regexText));
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