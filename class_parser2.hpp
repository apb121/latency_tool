#ifndef CLASS_PARSER_HPP
#define CLASS_PARSER_HPP

#include <vector>
#include <cstring>
#include <string>
#include <map>

using variable_info = std::tuple<std::string, size_t, std::string, std::string>;

// variable_name, variable_size, variable_type, alignment_type
size_t get_type_size(std::string type, std::string array_match);

class File
{
    std::string file_name;
    std::vector<UDType> user_defined_types;
    public:
    File(std::string file_name)
        : file_name(file_name) {}
    std::string get_file_name() { return file_name; }
    std::vector<UDType> get_types() { return user_defined_types; }
    std::vector<UDType> detect_types();
    void suggest_optimised_orderings(std::map<std::string, size_t> udtype_sizes);
};

//test

struct G
{
  char c;
  double d;
  short s;
  int i;
};

struct Arr
{
  char a[512];
};

#endif