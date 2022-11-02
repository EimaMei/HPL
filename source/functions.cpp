/*
=================================================
|             CORE FUNCTIONS OF HCL             |
=================================================
*/

#define _CRT_SECURE_NO_WARNINGS

#include <interpreter.hpp>
#include <core.hpp>
#include <helper.hpp>

#include <sys/stat.h>
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

void print(HCL::variable msg, std::string end/* = \n*/) {
	std::string output;
	std::string vtype = msg.type;

	if (msg.value.size() > 1)
		output += "{";

	for (int i = 0; i < msg.value.size(); i++) {
		auto value = msg.value[i];
		if (i < msg.extra.size()) vtype = msg.extra[i];
		if (vtype == "bool")
			value = (value == "1" ? "true" : "false");

		output += value;

		if (msg.value.size() > 1 && ((i + 1) < msg.value.size()))
			output += ", ";
	}

	if (msg.value.size() > 1) output += "}";

	printf("%s%s", output.c_str(), end.c_str());
}


int createFolder(std::string path, int mode/* = 0777*/) {
	int check;
	std::string fullPath;
	std::vector<std::string> folders = split(path, "/");

	for (auto folder : folders) {
		fullPath += folder;
		check = mkdir(fullPath.c_str(), mode);
		fullPath += "/";
	}
  
    return check;
}


int removeFolder(std::string path) {
	int check = remove(path.c_str());
  
    return check;
}


int createFile(std::string path, std::string content/* = ""*/, bool useUtf8BOM/* = false*/) {
	int output = 0;

	FILE* f = fopen(path.c_str(), "r");
	if (f == NULL) {
		f = fopen(path.c_str(), "w");
		if (!content.empty() && f != NULL) {
			if (useUtf8BOM) fprintf(f, "\xEF\xBB\xBF");
			fprintf(f, "%s", content.c_str());
		}
		else if (f == NULL) { output = -1; }
	}
	else {
		output = -1;
	}
    fclose(f);

	return output;
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
	fclose(f);
	free(buffer);
  
    return text;
}


int writeToFile(std::string path, std::string content, std::string mode/* = "w"*/) {
	FILE* f = fopen(path.c_str(), mode.c_str());
 
    if (f != NULL) {
        fprintf(f, "%s", content.c_str());
    }
	else {
		fclose(f);
		return -1;
	}
    fclose(f);
  
    return 0;
}


int removeFile(std::string path) {
	return removeFolder(path);
}


int copyFile(std::string source, std::string output) {
	std::string txt = readFile(source);
	int res = -1;
	if (!txt.empty()) {
		res = writeToFile(output, txt);
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


int convertToDds(std::string input, std::string output) {
	int w, h, channels;
	
	unsigned char* png = SOIL_load_image(input.c_str(), &w, &h, &channels, SOIL_LOAD_AUTO);

	if (png != NULL) // If loading the image failed.
		return SOIL_save_image(output.c_str(), SOIL_SAVE_TYPE_DDS, w, h, channels, png);
	else
		return -1;
}


bool pathExists(std::string path) {
	return (access(path.c_str(), F_OK) == 0);
}