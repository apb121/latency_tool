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
    size_t calculate_size();
    size_t calculate_size(std::vector<Member> proposed_types_list);
    bool suggest_optimisation(int critical_stride);
};

class FileCollection
{
    std::vector<std::string> file_names;
    std::vector<UDType_new> udtypes;
    public:
    void populate_file_names(std::vector<std::string> file_names_in)
    {
        file_names = file_names_in;
    }
    int detect_types();
    size_t get_alignment(std::string alignment_string);
    bool suggest_optimisations(int critical_stride);
    std::vector<UDType_new> get_user_types() { return udtypes; }
};

#endif