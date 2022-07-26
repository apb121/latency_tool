#ifndef CLASS_PARSER_HPP
#define CLASS_PARSER_HPP

#include <vector>
#include <cstring>
#include <string>
#include <map>
#include <numeric>

using variable_info = std::tuple<std::string, size_t, std::string, std::string>;

/* variable_name, variable_size, variable_type, alignment_type */
size_t get_type_size(std::string type, std::string array_match);

struct UDType
{
    std::string name;
    std::string class_info;
    std::vector<variable_info> types_list;
    bool has_auto = false;
    bool has_virtual = false;
    size_t total_size;
    bool is_child = false;
    std::string parent_name = "";
    UDType* parent_class = nullptr;
    public:
    UDType(std::string name, std::string class_info)
        : name(name), class_info(class_info) {}
    std::string get_name() { return name; }
    UDType* get_parent() { return parent_class; }
    std::string get_parent_name() { return parent_name; }
    size_t get_size() { return total_size; }
    void detect_variables(std::map<std::string, size_t> udtype_sizes);
    size_t calculate_size(std::map<std::string, size_t> udtype_sizes);
    size_t calculate_size(std::vector<variable_info> proposed_types_list, std::map<std::string, size_t> udtype_sizes);
    bool suggest_optimisations(std::map<std::string, size_t> udtype_sizes, int critical_stride);
};

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

//tests

struct G
{
  char c;
  double d;
  short s;
  int i;
};

struct Large
{
    char a[1024];
};

#endif