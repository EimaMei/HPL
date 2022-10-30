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

// Splits the sentence each time it encounters the 'value'. If 'charLimit' isn't null, then what happens is that the array doesn't get split if 'value' is between two 'charLimit'.
// For example, string "1,2,3,'4,5,6,7,8',9" would output "4,5,6,7,8" if 'value' is "," and charLimit is "'" 
std::vector<std::string> split(std::string str, std::string value, char charLimit = '\0');
// Checks if the line matches the regex.
bool useRegex(std::string str, std::string regexText);
// Removes any whitespace in a sentence.
std::string removeSpaces(std::string str);
// Removes the double quotes from strings. If 'noChecks' is enabled then it doesn't check if the string has double quotes in the front and back, which just essentially removes both the front and back char for any string.
std::string unstringify(std::string str, bool noChecks = false, char character = '"');
// Gets the path from the filename (eg. /usr/bin/somefile.img would turn to /usr/bin).
std::string getPathFromFilename(std::string filename);
// Checks if something is in the line.
bool find(std::string line, std::string str);
// Checks if string is an int.
bool isInt(std::string str);

/* HCl specific helper functions. */

// If a type exists. If the type is a struct, then `info` becomes
// the pointer to the struct.
bool typeIsValid(std::string type, HCL::structure* info = NULL);
// If a type is a core type.
bool coreTyped(std::string type);
// Gets the variable's value by its name and returns a pointer of it.
// If the `varName` is a struct member, then regardlessly the function
// will attempt to find it. If successful, `var` will get the attributes
// of the member, however `var.extra[0]` becomes the index of where the
// member was in the struct. 
HCL::variable* getVarFromName(std::string varName, HCL::variable* var = NULL);