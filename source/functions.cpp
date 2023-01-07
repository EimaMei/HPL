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
/*
=================================================
|             CORE FUNCTIONS OF HPL             |
=================================================
*/

#define _CRT_SECURE_NO_WARNINGS

#include <interpreter.hpp>
#include <functions.hpp>
#include <helper.hpp>

#include <sys/stat.h>
#include <filesystem>

#ifdef _WIN32
#include <direct.h>
#include <io.h>

#define mkdir(dir, mode) _mkdir(dir)
#define F_OK 0
#define access _access
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include <deps/SOIL2.h>


void print(HPL::variable msg, std::string end/* = \n*/) {
	std::string output;

	if (isVars(msg.value)) {
		output += "{";
		auto& _struct = getVars(msg.value);

		for (int memberIndex = 0; memberIndex < _struct.size(); memberIndex++) {
			auto& member = _struct[memberIndex];

			if (member.has_value()) {
				if (member.type == "string")
					output += "\"" + xToStr(member.value) + "\"";
				else
					output += xToStr(member.value);
			}

			if (_struct.size() > 1 && ((memberIndex + 1) < _struct.size()))
				output += ", ";
		}
		output += "}";
	}
	else
		output = xToStr(msg.value);

	std::printf("%s%s", output.c_str(), end.c_str());
}


std::string func_str(HPL::variable value) { return xToStr(value.value); }
int func_int(HPL::variable value) { return xToType<int>(value.value); }
float func_float(HPL::variable value) { return xToType<float>(value.value); }
bool func_bool(HPL::variable value) { return xToType<bool>(value.value); }
int len(HPL::variable value) { return xToStr(value.value).size(); }


int createFolder(std::string path) {
	int check;
	std::string fullPath;
	std::vector<std::string> folders = split(path, "/");

	for (const auto& folder : folders) {
		fullPath += folder;
		check = mkdir(fullPath.c_str(), 0777);
		fullPath += "/";
	}

    return check;
}


int removeFolder(std::string path) {
	return std::filesystem::remove_all(path);
}


int createFile(std::string path, std::string content/* = ""*/, bool useUtf8BOM/* = false*/) {
	FILE* f = fopen(path.c_str(), "r");

	if (f == NULL) {
		f = fopen(path.c_str(), "w");

		if (!content.empty() && f != NULL) {
			if (useUtf8BOM)
				fprintf(f, "\xEF\xBB\xBF");
			fprintf(f, "%s", content.c_str());
		}
		else if (f == NULL)
			return -1;
	}
	else
		return -1;
    fclose(f);

	return 0;
}


std::string readFile(std::string path) {
	FILE* f = fopen(path.c_str(), "rb");
	std::string text;
    char* buffer = nullptr;

    if (f != NULL) {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        buffer = (char*)malloc(fsize + 1);
        size_t size = fread(buffer, 1, fsize, f);
		buffer[size] = 0;
		text = buffer;

		if (text[0] == '\xEF' && text[1] == '\xBB' && text[2] == '\xBF') // Utf-8 bom text, remove the 3 first bytes just in case.
			text.erase(0, 3);
    }
	else { return ""; }
	fclose(f);
	free(buffer);

    return text;
}


int writeFile(std::string path, std::string content, std::string mode/* = "w"*/) {
	FILE* f = fopen(path.c_str(), mode.c_str());

    if (f != NULL)
        fprintf(f, "%s", content.c_str());
	else
		return -1;
    fclose(f);

    return 0;
}


int writeToMultipleLines(std::string path, int lineBegin, int lineEnd, std::string content, std::string mode/* = "w"*/) {
	FILE* f = fopen(path.c_str(), "r");

	if (f != NULL) {
		FILE* fw = fopen("replace.tmp", "w");
		int lineCount = 0;

		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);
		char* buf = new char[size];

		if (lineBegin < 0 || lineEnd < 0) { // Since the provided line count is negative, we gotta count backwards now.
			int entireCount = 0;
			char* tempBuf = (char*)malloc(size + 1);
			size_t res = fread(tempBuf, 1, size, f);
			tempBuf[size] = 0;

			for(int i = 0; i < res; i++) {
				if (tempBuf[i] == '\n')
					entireCount++;
			}

			if (lineBegin < 0)
				lineBegin += entireCount + 2;
			if (lineEnd < 0)
				lineEnd += entireCount + 2;

			free(tempBuf);
			fseek(f, 0, SEEK_SET);
		}

		std::string str;
		while (fgets(buf, size + 1, f)) {
			lineCount++;

			if (lineBegin <= lineCount && lineEnd >= lineCount) {
				if (str.empty()) {
					if (!find(mode, "w"))
						str = buf + content;
					else
						str = content;

					fputs(str.c_str(), fw);
				}
			}
			else
				fputs(buf, fw);
		}
		fclose(f);
		fclose(fw);

		remove(path.c_str());
		rename("replace.tmp", path.c_str());
	}
	else
		return -1;

	return 0;
}


int writeToLine(std::string path, int line, std::string content, std::string mode/* = "w"*/) {
	return writeToMultipleLines(path, line, line, content, mode);
}


int removeFile(std::string path) {
	return removeFolder(path);
}


int copyFile(std::string source, std::string output) {
	std::string txt = readFile(source);
	int res = -1;
	if (!txt.empty()) {
		res = writeFile(output, txt);
	}

	return res;
}


int moveFile(std::string source, std::string output) {
	int res = copyFile(source, output);
	if (res == 0) {
		res = removeFile(source);
	}

	return res;
}


int writeLocalisation(std::string file, std::string name, std::string description) {
	return writeFile(file, "\n\t" + name + ":0 \"" + description + "\"", "a");
}


int convertToDds(std::string input, std::string output) {
	int w, h, channels;

	unsigned char* img = SOIL_load_image(input.c_str(), &w, &h, &channels, SOIL_LOAD_AUTO);

	if (img != NULL) {
		int num = SOIL_save_image(output.c_str(), SOIL_SAVE_TYPE_DDS, w, h, channels, img);
		SOIL_free_image_data(img);

		return num - 1;
	}

	return -1;
}


bool pathExists(std::string path) {
	return (access(path.c_str(), F_OK) == 0);
}


std::string getFilenameFromPath(std::string path) {
	return path.substr(path.find_last_of("/\\") + 1);
}