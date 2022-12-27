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
#pragma once
#include <interpreter.hpp>
#include <vector>
#include <string>

/* General functions. */

// Splits the sentence each time it encounters the 'value'. If 'charScope' isn't null, then the array doesn't get split if 'value' is between 'charScope[0]' and 'charScope[1]'.
std::vector<std::string> split(std::string str, std::string value, std::string charScope = "\0\0");
// Checks if the line matches the regex.
bool useRegex(std::string str, std::string regexText);
// Checks if there are multiple matches from the regex.
bool useIterativeRegex(std::string str, std::string regexText);
// Removes any whitespace in a sentence.
std::string removeSpaces(std::string str);
// Removes any whitespace in the front or back of a string.
std::string removeFrontAndBackSpaces(std::string str);
// Removes the double quotes from strings. If 'noChecks' is enabled then it doesn't check if the string has double quotes in the front and back, which just essentially removes both the front and back char for any string.
std::string unstringify(std::string str, bool noChecks = false, char character = '"');
// Gets the path from the filename (eg. /usr/bin/somefile.img would turn to /usr/bin).
std::string getPathFromFilename(std::string filename);
// Checks if something is in the line.
bool find(std::string line, std::string str);
// Checks if string is an int.
bool isInt(std::string str);
//Checks if a string is *actually* a string or f-string.
bool isStr(std::string str);
// Replaces all instances of 'oldString' with 'newString' in 'str'
std::string replaceAll(std::string str, std::string oldString, std::string newString);
// Replaces the FIRST instance of 'oldString' with 'newString' in 'str'
std::string replaceOnce(std::string, std::string oldString, std::string newString);
// Converts a string to a bool.
bool stringToBool(std::string str);
// Fixes string where a backslash and letter are treated as different letters (eg. "\n" will now properly get converted to '\n').
std::string convertBackslashes(std::string str);
// Converts a math operation to a single double (UNFINISHED, NEEDS REFINING).
double eval(std::string expr, int& errorCode);

// Converts 'val' to a string and return it.
// (eg. xToStr(234) would return a string "234")
std::string xToStr(allowedTypes val);
// Converts 'val' to the type given and returns it.
// (eg. xToType<int>("27") would return an int 27)
template <class T>
T xToType(allowedTypes val) {
	if (std::is_same_v<T, int> || std::is_same_v<T, bool> || std::is_same_v<T, float>) {
		if (std::holds_alternative<std::string>(val))
			return (T)std::stod(std::get<std::string>(val));

		else if (std::holds_alternative<int>(val))
			return (T)std::get<int>(val);

		else if (std::holds_alternative<float>(val))
			return (T)std::get<float>(val);

		else if (std::holds_alternative<bool>(val))
			return (T)std::get<bool>(val);
	}

	return T();
}

/* HCl specific helper functions. */

// If a type exists. If the type is a struct, then `info` becomes
// the pointer to the struct.
bool typeIsValid(std::string type, HCL::structure* info = NULL);
// If a type is a core type.
bool coreTyped(std::string type);
// Gets the core type from value. If it cannot determine the type,
// then it's most likely a struct (or a type that doesn't exist).
std::string getTypeFromValue(std::string value);
// Corrects the string value and returns a `HCL::variable`, with the
// `.type` being the value's original type, and the `.value` being
// the inputed value in a correct type. (eg. "3" would output
// {.type = "int", .value = 3}).
bool setCorrectValue(HCL::variable& var, std::string value);
// Gets the variable's value by its name and returns a pointer of it.
// If the `varName` is a struct member, then regardlessly it'll look
// for said member's type and values.
HCL::variable* getVarFromName(std::string varName);
// Fixes the sentence from being f-string to a normal string.
int getValueFromFstring(std::string ogValue, std::string& output);
// Get the struct from name. If no struct is found, returns a nullptr.
HCL::structure* getStructFromName(std::string name);
// Convert math expression to an actual result (UNFINISHED).
std::string extractMathFromValue(std::string expr, HCL::variable* var);
// Returns a string "<type> <name>(<params>)"
std::string printFunction(HCL::function func);