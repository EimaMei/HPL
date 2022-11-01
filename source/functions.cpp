/*
=================================================
|             CORE FUNCTIONS OF HCL             |
=================================================
*/

#include <interpreter.hpp>
#include <core.hpp>
#include <helper.hpp>

#include <sys/stat.h>
#include <dirent.h>

void print(HCL::variable msg, std::string end/* = \n*/) {
	std::string output;
	std::string vtype = msg.type;

	if (msg.value.size() > 1)
		output += "{";

	for (int i = 0; i < msg.value.size(); i++) {
		auto value = msg.value[i];
		if (i < msg.extra.size()) vtype = msg.extra[i];

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
		fclose(f);
		f = fopen(path.c_str(), "w");
		if (!content.empty()) {
			if (useUtf8BOM) fprintf(f, "\xEF\xBB\xBF");
			fprintf(f, "%s", content.c_str());
		}
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