#ifndef USER_OPTIONS_HPP
#define USER_OPTIONS_HPP

#include <unistd.h>
#include <bitset>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <regex>

#include "cache_info.hpp"
#include "class_parser.hpp"
#include "binary_analyser.hpp"

#define SOURCE_CODE_ONLY 7
#define BINARY_ONLY 6
#define CACHE_INFO_ONLY 5
#define MANUAL_CACHE 4
#define NO_EMPIRICAL 3
#define KEEP_TEMP 2
#define EXISTING_TEMP_FILES 1

struct UserOptions
{
    std::bitset<8> flags;
    int coex = 1;
    int comp = 256;
    int ranking_length = 10;
    std::vector<std::string> file_names;
    std::vector<File> files;
    std::vector<UDType> all_user_types;
    std::map<std::string, size_t> udtype_sizes;
    Processor proc;
    int parse_flags(int argc, char** argv);
    int run_file_setup();
    int run_cache_setup();
    int run_analysis();
    ~UserOptions();
};

#endif