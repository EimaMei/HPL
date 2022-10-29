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
#include <core.hpp>
#include <helper.hpp>

bool typeIsValid(std::string type, HCL::structure* info/* = NULL*/) {
	if (coreTyped(type)) return true;

	// Didn't find a core type, maybe it'll find a struct instead.
	for (auto s : HCL::structures) {
		if (type == s.name) {
			info = &s;
			return true;
		}
	}

	return false; // Didn't find anything.
}


bool coreTyped(std::string type) {
	for (auto coreType : coreTypes) {
		if (type == coreType) return true;
	}
	
	return false;
}


HCL::variable* getVarFromName(std::string varName, HCL::variable* var/* = NULL*/) {
	HCL::structure s;
	for (auto& v : HCL::variables) {
		if (v.name == varName) {
			return &v;
		}

		if (typeIsValid(v.type, &s) && !coreTyped(v.type)) { // A custom type
			for (int i = 0; i < s.value.size(); i++) {
				auto member = s.value[i];
				if (varName == (v.name + "." + member.name)) {
					var->type = v.extra[i];
					var->name = varName;
					var->value = v.value;
					var->extra = {std::to_string(i)};
					return &v;
				}
			}
		}
	}
	return {};
}