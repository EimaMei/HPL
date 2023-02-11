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
#include <interpreter.hpp>

#include <string>
#include <vector>
#include <map>

enum operatorsEnum {
    equal,
    plus_equal, minus_equal, multiply_equal, divide_equal, module_equal,
    plus_plus, minus_minus,

    equal_equal, not_equal,
    greater_than, lesser_than, equal_greater, equal_lesser,
    and_and, or_or
};


// The core types of the language.
extern std::vector<std::string> coreTypes;
//
extern std::map<std::string, operatorsEnum> operatorList;

// Reformats the params to be turned into variables.
// Afterwards, check what function the user specified
// and execute it.
int executeFunction(std::string name, std::string params, HPL::function& func, HPL::variable& output, bool dontCheck = false);
// Checks if the specific function got used.
bool useFunction(HPL::function func, std::vector<HPL::variable>& userParams);
// Defines the core functions and returns the function's return if successful.
allowedTypes coreFunctions(std::vector<HPL::variable>& params);
// Sets the variable's value to the return of specified function.
int assignFuncReturnToVar(HPL::variable* existingVar, std::string funcName, std::string funcParam, bool dontCheck = false);