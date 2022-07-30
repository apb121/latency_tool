#ifndef CLASS_PARSER_HPP
#define CLASS_PARSER_HPP

#include <vector>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <list>
#include <forward_list>
#include <deque>
#include <cstring>
#include <string>
#include <map>
#include <numeric>
#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>
#include <algorithm>

/* variable_name, variable_size, variable_type, alignment */
using variable_info_new = std::tuple<std::string, size_t, std::string, size_t>;

struct Member
{
    std::string name;
    size_t size = 0;
    std::string align_string;
    size_t alignment = 0;
};

class UDType_new
{
    std::string name;
    size_t total_size = 0;
    public:
    std::vector<Member> member_variables;
    UDType_new(std::string name, std::vector<Member>& member_variables, size_t total_size)
        : name(name), member_variables(member_variables), total_size(total_size) {}
    UDType_new(UDType_new const& other)
    {
        name = other.name;
        total_size = other.total_size;
        for (int i = 0; i < other.member_variables.size(); ++i)
        {
            member_variables.push_back(other.member_variables[i]);
        }
    }
    std::string get_name() { return name; }
    size_t get_total_size() { return total_size; }
    void set_name(std::string new_name) { name = new_name; }
    void set_size(size_t new_size) { total_size = new_size; }
};

class FileCollection
{
    std::vector<std::string> file_names;
    std::vector<UDType_new> udtypes;
    public:
    FileCollection(std::vector<std::string>& file_names_in/*, bool binary*/)
    {
        for (int i = 0; i < file_names_in.size()/* - (binary ? 1 : 0)*/; ++i)
        {
            file_names.push_back(file_names_in[i]);
        }
    }
    int detect_types();
};

/* variable_name, variable_size, variable_type, alignment_type */
using variable_info = std::tuple<std::string, size_t, std::string, std::string>;

size_t get_type_size(std::string type, std::string array_match);

struct UDType
{
    char a[5] = {'a', 'b', 'c', 'd', 'e'};
    std::array<char, 4> aa;
    std::array<char, 4> aaa;
    std::array<std::array<char, 2>, 500> aaaa;
    std::string name;
    std::string class_info;
    std::vector<variable_info> types_list;
    bool has_virtual = false;
    size_t total_size;
    bool is_child = false;
    std::string parent_name = "";
    UDType* parent_class = nullptr;
    bool has_auto = false;
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