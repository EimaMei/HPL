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
#include <string>
#include <regex>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

// Save functions.
#define SAVE_NOTHING 0
#define SAVE_STRUCT 1
#define SAVE_FUNC 2

// Outputs returns.
#define FOUND_NOTHING 0
#define FOUND_SOMETHING 1
#define FOUND_ERROR -1

// Convert core type to void.
#define intToVoid(integer) (void*)(uintptr_t)integer
#define stringToVoid(str) static_cast<void*>(new std::string(str))

// Convert void back to the specified core type.
#define voidToInt(pointer) (int)(uintptr_t)pointer
#define voidToString(pointer) *static_cast<std::string*>(pointer)


struct hclVector {
	std::vector<std::string> value;

	std::string str(int index) {
		if (index > value.size()) return "";

		return value[index - 1];
	}
};

namespace HCL {
	enum RETURN_OUTPUT { OUTPUT_NOTHING, OUTPUT_BLACK, OUTPUT_RED, OUTPUT_GREEN, OUTPUT_YELLOW, OUTPUT_BLUE, OUTPUT_PURPLE, OUTPUT_CYAN, OUTPUT_GRAY };

	struct variable {
		std::string type;
		std::string name;
		std::vector<std::string> value;
		std::vector<std::string> extra;
	};
	struct structure {
		std::string name;
		std::vector<variable> value;
	};
	struct function {
		std::string type;
		std::string name;
		std::vector<variable> params;
		std::vector<std::string> code;
		int minParamCount;
	};
	
	// Interpreter configs.
	extern bool debug;
	extern bool strict;
	
	// Interpreter runtime information.
	extern std::string curFile;
	extern std::string line;
	extern int lineCount;
	extern int mode;
	extern hclVector matches;

	// Definitions that are saved in memory.
	extern std::vector<variable> variables;
	extern std::vector<structure> structures;
	extern std::vector<function> functions;

	// Sets the color for the text that'll get printed.
	std::string colorText(std::string txt, RETURN_OUTPUT type, bool light = false);

	// Opens a file and interpretes each line.
	void interpreteFile(std::string file);
	// Interpretes a single line.
	int interpreteLine(std::string line);
	// Enables debug mode (At the end it'll print everything that the interpreter remembers).
	void debugMode();
	// Prints out information about a vector of variables.
	void debugPrintVar(std::vector<variable> vars, std::string indent = "\t", std::string end = "\n");
	//Prints out information about a vector of structures.
	void debugPrintStruct(std::vector<structure> structList, std::string indent = "\t");
	// Prints out information about a vector of variables.
	void debugPrintFunc(std::vector<function> func, std::string indent = "\t");
	// Resets the interpreter's runtime information.
	void resetRuntimeInfo();

	// Checks for any includes.
	int checkIncludes();
	// Checks if it's a structure (if so, configure the runtime information).
	int checkStruct();
	// Checks for any functions.
	int checkFunctions();
	// Check for any variables.
	int checkVariables();

	// Throws an intepreter error if something is wrong. This is very similar to 'printf', however as of now only '%s' and '%i' are supported.
	void throwError(bool sendRuntimeError, std::string text, ...);
}