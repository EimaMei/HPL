#include <interpreter.hpp>
#include <functions.hpp>


std::vector<std::string> split(std::string str, std::string value) {
	std::vector<std::string> list;
	std::string buf;
	int lastCharPos=0;

	for (auto x : str) {
		buf += x;
		if (buf.find(value) != std::string::npos) {
			std::string msg = buf.substr(0,  buf.find(value));
			list.insert(list.end(), msg);
			buf = "";
		}
	}
	list.insert(list.end(), buf);
	
	return list;
}


bool useRegex(std::string str, std::string regexText) {
	return std::regex_search(str, HCL::matches, std::regex(regexText));
}


std::string removeSpaces(std::string str) {
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
	return str;
}


std::string unstringify(std::string str, bool noChecks/* = false*/) {
	if ((str[0] == '"' && str[str.size() - 1] == '"') || noChecks) {
		str.pop_back();
		str.erase(0, 1);
	}

	return str;
}


std::string getPathFromFilename(std::string filename) {
	return filename.substr(0, filename.find_last_of("/\\"));
}