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
#include <string.h>

/* ======================== CORE FUNCTIONS OF HCL ======================== */
// Each function define here must follow this format:
//      <Description of the function>
//      <Function implementation in HCL>
//      <The actual function defnition in C++>
//
// It's also pretty good to space out functions depending
// on which group they're categorized as for readability
// (for example, instead of having no space between 'print', 
// 'createFolder' and 'removeFolder', it would be best to
// put a space between 'print' and 'createFolder').

// Prints something out in the terminal.
// void print(var msg, string end = "\n")
void print(HCL::variable msg, std::string end = "\n");

// Converts value to str.
// string str(var value)
std::string func_str(HCL::variable value);
// Converts value to int`.
// int int(var value)
int func_int(HCL::variable value);
// Converts value to float.
// float float(var value)
float func_float(HCL::variable value);
// Converts value to bool.
// bool bool(var value)
bool func_bool(HCL::variable value);
// Gets the length of the value
// int len(var value)
int len(HCL::variable value);

// Creates a folder.
// int createFolder(string path)
int createFolder(std::string path);
// Removes a folder.
// int removeFolder(string path)
int removeFolder(std::string path);

// Creates a file.
// int createFile(string path, string content = "", bool useUtf8BOM = false)
int createFile(std::string path, std::string content = "", bool useUtf8BOM = false);
// Reads a file.
// string readFile(string path)
std::string readFile(std::string path);
// Writes into a file.
// int writeFile(string path, string content, string mode = "w")
int writeFile(std::string path, std::string content, std::string mode = "w");
// Removes a file.
// int removeFile(string path)
int removeFile(std::string path);
// Copies a file to a new path.
// int copyFile(string source, string output)
int copyFile(std::string source, std::string output);
// Moves a file to a new path.
// int moveFile(string source, string output)
int moveFile(std::string source, std::string output);
// Writes to a specific line. If mode is 'w', then the line gets overwritten. If 'a', then the line gets appended with 'content'.
// int writeToLine(string path, int line, string content, string mode = "w")
int writeToLine(std::string path, int line, std::string content, std::string mode = "w");
// Replaces multiple lines with the given content.
// int writeToMultipleLines(string path, int lineBegin, int lineEnd, string content, string mode = "w")
int writeToMultipleLines(std::string path, int lineBegin, int lineEnd, std::string content, std::string mode = "w");
// Writes to a pre-existing localisation file.
// int writeLocalisation(string file, string name, string description)
int writeLocalisation(std::string file, std::string name, std::string description);

// Converts 'input' to .dds and copies the converted image to 'output'
// int convertToDds(string input, string output)
int convertToDds(std::string input, std::string output);

// Checks if a path already exists.
// int pathExists(string path)
bool pathExists(std::string path);
// Gets the filename from the path (eg. /usr/bin/somefile.img would turn to somefile.img).
// string getFilenameFromPath(string path)
std::string getFilenameFromPath(std::string path);

// Checks if a string is in 'line'.
// bool find(string line, string str)
// (Check helper.hpp for the C++ definition)

// Replaces all instances of 'oldString' with 'newString' in 'str'
// string replaceAll(string str, string oldString, string newString);
// (Check helper.hpp for the C++ definition)
