#pragma once
#include <vector>
#include <regex>
#include <string.h>

// Splits the sentence each time it encounters the 'value'.
std::vector<std::string> split(std::string str, std::string value);
// Checks if the line matches the regex.
bool useRegex(std::string str, std::string regexText);
// Removes any whitespace in a sentence.
std::string removeSpaces(std::string str);
// Removes the double quotes from strings.
std::string unstringify(std::string str, bool noChecks = false);
// Gets the path from the filename (eg. /usr/bin/somefile.img would turn to /usr/bin).
std::string getPathFromFilename(std::string filename);