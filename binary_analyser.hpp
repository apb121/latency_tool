#ifndef BINARY_ANALYSER_HPP
#define BINARY_ANALYSER_HPP

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <cmath>

#include "user_options.hpp"
#include "data_analyser.hpp"

struct UserOptions;

struct ProblemGroup
{
    std::set<size_t> functions;
    size_t coexecutions = 0;
    size_t overlap = 0;
    ProblemGroup(std::set<size_t> functions, size_t coexecutions, size_t overlap)
        : functions(functions), coexecutions(coexecutions), overlap(overlap) {}
};

class Function
{
    size_t address;
    size_t size;
    std::string name;
    std::string file_location;
    public:
    std::set<size_t> competes_with; /* competes for cache sets with */
    std::set<size_t> coexecutes_with; /* executed in conjunction with (according to the objdump) */
    std::set<size_t> competes_and_coexecutes_with;
    Function(size_t address, size_t size, std::string name, std::string file_location)
        : address(address), size(size), name(name), file_location(file_location) {}
    Function()
    {
        /*
            this function is required when [] is called
            with a non-existent key in the functions_list member
            of the Binary class.
            this should never happen, so it is a problem
            if this code is reached!
        */
        std::cout << "Unknown function error." << std::endl;
        exit(1);
    }
    size_t get_address() { return address; }
    size_t get_size() { return size; }
    std::string get_name() { return name; }
    std::string get_file_location() { return file_location; }
};

class Binary
{
    std::string file_name;
    std::map<size_t, Function> functions_list;
    std::set<std::set<size_t>> problem_groups;
    std::vector<UDType> user_types;
    public:
    Binary(std::string file_name, std::vector<UDType> user_types)
        : file_name(file_name), user_types(user_types) {}
    int get_functions(UserOptions& uo);
    int populate_competition_vectors(UserOptions& uo);
    int populate_coexecution_vectors(UserOptions& uo);
    int find_problem_function_groups(UserOptions& uo);
    int rec_problem_find(std::set<std::set<size_t>>& current_group, std::set<std::set<size_t>>& next_size_groups);
};

#endif