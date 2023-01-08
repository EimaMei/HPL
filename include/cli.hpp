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
#include <iostream>
#include <string>
#include <vector>

#define VERSION "HPL 0.2.6"
#define AUTHORS "Created by EimaMei/Sacode"

#if defined(__clang__)
    #define COMPILER removeFrontAndBackSpaces(std::string("Clang version ") + std::string(__clang_version__)).c_str() // mfw random space exists randomly on linux and windows.
#elif defined(__GNUC__) && !defined(__clang__)
	#define COMPILER "GCC version " + __GNUC__ + "." + __GNUC_MINOR__ + "." + __GNUC_PATCHLEVEL__
#elif __MSC_VER__
	#define COMPILER "MSVC version" + __MSC_VER__
#else
	#define COMPILER "Unknown compiler"
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#if defined(_WIN64)
		#define OS "Windows 64-bit"
	#else
		#define OS "Windows 32-bit"
	#endif
#elif defined(__APPLE__)
	#define OS "Darwin"
#elif defined(__linux__)
	#define OS "Linux"
#endif


// Checks if the input equals to the CLI argument.
void checkArg(std::string arg, std::string input, bool& config, bool& res);
// Checks if the input equals to the CLI arguments.
void checkArgs(std::vector<std::string> args, std::string input, bool& config, bool& res);
// Prints the help.
void printHelp();
// Dumps the entire project's JSON.
void dumpJson();