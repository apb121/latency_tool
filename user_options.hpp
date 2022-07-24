#ifndef USER_OPTIONS_HPP
#define USER_OPTIONS_HPP

#include <bitset>
#include <vector>
#include <string>
#include <map>

#include "cache_info.hpp"

struct File;
struct UDType;

struct UserOptions
{
    std::bitset<8> flags;
    int coex = 1;
    int comp = 16;
    int ranking_length = 10;
    std::vector<std::string> file_names;
    std::vector<File> files;
    std::vector<UDType> all_user_types;
    std::map<std::string, size_t> udtype_sizes;
    Processor proc;
    int parse_flags(int argc, char** argv);
};

#endif