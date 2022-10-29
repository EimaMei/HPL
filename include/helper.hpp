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
#include <vector>
#include <string>

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