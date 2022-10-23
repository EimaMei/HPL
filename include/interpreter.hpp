#pragma once
#include <string>
#include <regex>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#define SAVE_NOTHING 0
#define SAVE_STRUCT 1
#define SAVE_FUNC 2

#define FOUND_NOTHING 0
#define FOUND_SOMETHING 1

namespace HCL {
	enum RETURN_OUTPUT { OUTPUT_GREEN, OUTPUT_YELLOW, OUTPUT_RED, OUTPUT_PURPLE, OUTPUT_BLUE };
	enum varType { STRING, INT, SCOPE, CUSTOM };

	struct variable {
		std::string name;
		varType type; std::string strType;
		std::string value;
	};
	
	// Interpreter configs.
	extern bool debug;
	
	// Interpreter runtime information.
	extern std::string curFile;
	extern std::string line;
	extern int lineCount;
	extern int mode;
	extern std::smatch matches;

	// Definitions that are saved in memory.
	extern std::vector<variable> variables;
	extern std::vector<std::pair<variable, std::vector<variable>>> functions;
	extern std::vector<std::pair<std::string, std::vector<variable>>> structures;

	// Sets the color for the text that'll get printed.
	std::string colorText(std::string txt, RETURN_OUTPUT type);

	// Opens a file and interpretes each line.
	void interpreteFile(std::string file);
	// Enables debug mode (At the end it'll print everything that the interpreter remembers).
	void debugMode();
	// Resets the interpreter's runtime information.
	void resetRuntimeInfo();

	// Checks for any includes.
	int checkIncludes();
	// Check for any variables (structs, functions, global etc).
	int checkVariables();
	// Checks if it's a function.
	int checkFunctions();
	// Checks if it's a structure (if so, configure the runtime information).
	int checkStruct();

	// Checks if 'str' is in HC4::line
	bool find(std::string str);
}