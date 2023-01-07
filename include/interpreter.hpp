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

#pragma once

#include <string>
#include <regex>
#include <vector>
#include <variant>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#define WINDOWS 1
#endif

// Interpreter modes.
#define MODE_DEFAULT 0

#define MODE_SAVE_STRUCT          0x0001
#define MODE_SAVE_FUNC            0x0002
#define MODE_SAVE_SCOPE           0x0004
#define MODE_SCOPE_IF_STATEMENT   0x0006
#define MODE_SCOPE_IGNORE_ALL     0x0008

#define MODE_CHECK_STRUCT         0x0101
#define MODE_CHECK_FUNC           0x0102
#define MODE_CHECK_SCOPE          0x0104
#define MODE_CHECK_IF_STATEMENT   0x0106
#define MODE_CHECK_IGNORE_ALL     0x0108

// Outputs returns.
#define FOUND_NOTHING 0
#define FOUND_SOMETHING 1
#define FOUND_ERROR -1

// Get <type>.
#define getStr(value) std::get<std::string>(value)
#define getInt(value) std::get<int>(value)
#define getFloat(value) std::get<float>(value)
#define getBool(value) std::get<bool>(value)
#define getVars(value) std::get<std::vector<HPL::variable>>(value)

#define isVars(value) std::holds_alternative<std::vector<HPL::variable>>(value)


namespace HPL {
	enum RETURN_OUTPUT { OUTPUT_NOTHING, OUTPUT_BLACK, OUTPUT_RED, OUTPUT_GREEN, OUTPUT_YELLOW, OUTPUT_BLUE, OUTPUT_PURPLE, OUTPUT_CYAN, OUTPUT_GRAY };
	struct configArgs {
		bool interprete = true;

		bool help;
		bool version;
		bool strict;

		bool debugAll;
		bool debugPrint;
		bool debugLog;
		bool breakpoint; std::pair<std::string, int> breakpointValues;

		bool dumpJson;

		std::string curIndent;
	};
	#define allowedTypes std::variant<std::monostate, std::string, int, float, bool, std::vector<HPL::variable>>

	struct variable {
		std::string type;
		std::string name;
		allowedTypes value; // Every possible type in HPL. Note that std::vector<variable> is for structs, while the std::any one is for extra data, like localisation and arrays

		bool has_value() { return value.index() != 0; }
		void reset_value() { value = std::monostate{}; }
		void reset_all() { type = std::string(); name = std::string(); value = std::monostate{}; }
	};
	struct structure {
		std::string name;
		std::vector<variable> value;
		int minParamCount;
	};
	struct function {
		std::string type;
		std::string name;
		std::vector<variable> params;
		std::vector<std::string> code;
		int minParamCount;

		std::string file; // Used for the error message.
		int startingLine;
	};
	struct vector { // A very small "implementation" of std::smatches.
		std::vector<std::string> value;

		std::string str(int index) {
			if (index > value.size()) return "";

			return value[index - 1];
		}
		void clear() { value.clear(); }
		size_t size() { return value.size(); }
		bool empty() { return value.empty(); }
		void push_back(std::string x) { value.push_back(x); }
		std::string operator[](int index) { return value[index]; }
	};

	// Interpreter configs.
	extern configArgs arg;

	// Interpreter runtime information.
	extern std::string curFile;
	extern std::string line;
	extern int lineCount;
	extern int mode;
	extern HPL::vector matches;
	extern int equalBrackets;
	extern int scopeIndex;
	extern int oldMode;

	// Definitions that are saved in memory.
	extern std::vector<variable> variables;
	extern std::vector<structure> structures;
	extern std::vector<function> functions;
	extern std::vector<function> ifStatements;
	extern HPL::variable functionOutput;

	extern std::vector<variable> cachedVariables; // Out of scoped variables for the JSON dumper.

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
	// Checks for any modes.
	int checkModes();
	// Checks for any conditions.
	int checkConditions();
	// Checks if it's a structure (if so, configure the runtime information).
	int checkStruct();
	// Checks for any functions.
	int checkFunctions();
	// Checks for any variables.
	int checkVariables();

	// Throws an intepreter error if something is wrong. This is very similar to 'printf', however as of now only '%s' and '%i' are supported.
	void throwError(bool sendRuntimeError, std::string text, ...);
	// Gets the current mode's name.
	std::string getModeName();
}