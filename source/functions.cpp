/*
=================================================
|             CORE FUNCTIONS OF HCL             |
=================================================
*/

#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdio>

#include <interpreter.hpp>
#include <functions.hpp>
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

	std::printf("%s%s", output.c_str(), end.c_str());
}


int createFolder(std::string path) {
    return std::filesystem::create_directories(path);
}


int removeFolder(std::string path) {
    return static_cast<int>(std::filesystem::remove_all(path));
}


int createFile(std::string path, std::string content/* = ""*/, bool useUtf8BOM/* = false*/) {
	std::filesystem::path p = path;

    if (!std::filesystem::create_directories(p.parent_path())) return -1;

    auto out = std::ofstream(p);
    out << content;

	return 0;
}


std::string readFile(std::string path)
{
    std::stringstream out;
    auto in = std::ifstream(path);
    std::string ln;
    while (std::getline(in, ln))
        out << ln;

    return out.str();
}


int writeFile(std::string path, std::string content) {
    auto out = std::ofstream(path);
    out << content;
    return 0;
}


int writeToLine(std::string path, int line, std::string content) {
    auto out = std::ofstream(path);
    out.seekp(0);
    auto in = std::ifstream(path);

    auto buf = std::stringstream();
    size_t c = 0;
    std::string ln;
    for (;std::getline(in, ln), c < line; c++)
        buf << ln << '\n';

    buf << content << '\n';

    while (std::getline(in, ln))
        buf << ln << '\n';

    in.close();
    out << buf.str();

	return 0;
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
	return writeFile(file, "\n\t" + name + ":0 \"" + description + "\"");
}


int convertToDds(std::string input, std::string output) {
	int w, h, channels;
	
	unsigned char *png = SOIL_load_image(input.c_str(), &w, &h, &channels, SOIL_LOAD_AUTO);

	if (png != nullptr) // If loading the image failed.
		return SOIL_save_image(output.c_str(), SOIL_SAVE_TYPE_DDS, w, h, channels, png);
	else
		return -1;
}


bool pathExists(std::string path) {
	return std::filesystem::exists(path);
}


std::string getFilenameFromPath(std::string path) {
	return std::filesystem::path(path).filename();
}