#include <string>
#include <vector>
#define VER_STRING {major, minor, patch}

#define VERSION "HCL 0.0.1"
#define AUTHORS "Created by Sacode"

#if defined(__clang__)
    #define COMPILER "Clang version " + __clang_version__
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
#elif defined(__unix__)
	#define OS "Unix OS"
#endif

void checkArg(std::string arg, std::string input, bool& config, bool& res) {
	if (input == ("-" + arg)) {
		config = true;
		res = true;
	}
	else if (input == ("-no" + arg)) {
		config = false;
		res = true;
	}
}


void checkArgs(std::vector<std::string> args, std::string input, bool& config, bool& res) {
	for (auto& a : args) {
		checkArg(a, input, config, res);
		if (res)
			break;
	}
}


void printHelp() {
	std::cout << HCL::colorText(VERSION, HCL::OUTPUT_GREEN) << std::endl
			  << AUTHORS << std::endl
			  << (std::string)COMPILER << " on " << OS << std::endl
			  << HCL::colorText("ARGS:", HCL::OUTPUT_YELLOW) << "\n\t"
			  		<< HCL::colorText("<FILE>", HCL::OUTPUT_GREEN)                                                << "                     Selected file to be interpreted." << "\n"
			  << HCL::colorText("OPTIONS:", HCL::OUTPUT_YELLOW) << "\n\t"
			  		<< HCL::colorText("-help", HCL::OUTPUT_GREEN) << ", " << HCL::colorText("-h  ", HCL::OUTPUT_GREEN) << "                Prints the available CLI options as well as the the version, authors, compiler and OS of the HCL executable." << "\n\t"
			  		<< HCL::colorText("-debug", HCL::OUTPUT_GREEN) << ", " << HCL::colorText("-g ", HCL::OUTPUT_GREEN) << "                Enables all debug procedures (logging and printing debug information)." << "\n\t"
					<< HCL::colorText("-log", HCL::OUTPUT_GREEN) << ", " << HCL::colorText("-l   ", HCL::OUTPUT_GREEN) << "                Logs and prints every noteworthy event that the interpreter has got." << "\n\t"
			  		<< HCL::colorText("-strict", HCL::OUTPUT_GREEN) << ", " << HCL::colorText("-s", HCL::OUTPUT_GREEN) << "                Enables a strict mode, where you have a limited amount of available features to make less confusing code/massive mistakes (Barely implemented).";
}