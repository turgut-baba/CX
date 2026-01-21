// TypherCompiler.cpp : Defines the entry point for the application.
//

#include "TypherCompiler.h"
#include "Parser.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

std::string read_file(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path))
        throw std::runtime_error("File does not exist: " + path.string());

    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open file: " + path.string());

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

bool check_file_end(std::string filename, const char* suffix)
{
    const size_t name_size = filename.size();
    if (name_size < 3) {
        return false;
    }
    char result_suffix[4];
    const size_t suffix_index = name_size - 3;
    for (int i = suffix_index; i < name_size; i++) {
        result_suffix[i - suffix_index] = filename[i];
    }
    result_suffix[3] = '\0';
    return(std::strcmp(result_suffix, suffix) == 0);
}

int main(int argc, char** argv)
{
    if (argc < 2) 
    {
        std::cout << "Please enter a file name." << std::endl;
        return 1;
    }

    char* entry_file = argv[1];
    if (!check_file_end(entry_file, ".ty")) {
        std::cout << "Please enter a '.ty' file." << std::endl;
        return 1;
    }

    try {
        std::string file_buffer = read_file(entry_file);
        Parser::Parser parse = Parser::Parser(file_buffer);
        parse.parse();
        parse.PrintAST(); // DEBUG
    }
    catch (std::exception& e) {
        std::cout << "Err: " << e.what() << std::endl;
    }

    

	return 0;
}
