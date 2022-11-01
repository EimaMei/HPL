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
#include <string>
#include <vector>
#include <interpreter.hpp>

// The core types of the language.
extern std::vector<std::string> coreTypes;

// Reformats the params to be turned into variables.
// Afterwards, check what function the user specified
// and execute it.
int checkForFunctions(std::string name, std::string params, HCL::function& func, void*& output);
// Sorts out any shenanigans with functions.
bool useFunction(std::string type, std::string name, int minParamCount, int maxParamCount);
// Defines the core functions and returns the function's return if successful.
void* coreFunctions(std::vector<HCL::variable> params);
// Sets the variable's value to the return of specified function.
void assignFuncReturnToVar(HCL::variable* existingVar, std::string funcName, std::string funcParam);

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

// Creates a folder.
// int createFolder(string path, int mode = 0777)
int createFolder(std::string path, int mode = 0777);
// Removes a folder.
// int removeFolder(string path)
int removeFolder(std::string path);

// Creates a file.
// int createFile(string path, string content = "", bool useUtf8BOM = false)
int createFile(std::string, std::string content = "", bool useUtf8BOM = false);
// Reads a file.
// string readFile(string path)
std::string readFile(std::string path);
// Writes into a file.
// int writeFile(string path, string content, string mode = "w")
int writeToFile(std::string path, std::string content, std::string mode = "w");
// Removes a file.
// int delete(string path)
int removeFile(std::string path);

// Converts an image to .dds and copies said converted image to "output"
// int convertToDds(string input, string output)
int convertToDds(std::string input, std::string output);