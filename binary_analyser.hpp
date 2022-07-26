#ifndef BINARY_ANALYSER_HPP
#define BINARY_ANALYSER_HPP

#include <iostream>
#include <vector>
#include <set>
#include <map>

#include "user_options.hpp"
#include "class_parser.hpp"

struct UserOptions;

class Function
{
    size_t address;
    size_t size;
    std::string name;
    std::string file_location;
    public:
    std::set<size_t> competes_with; /* competes for cache sets with */
    std::set<size_t> coexecutes_with; /* often executed in conjunction with (according to the objdump) */
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
        std::cout << "If we get here, we have a problem!" << std::endl;
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
    int rec_problem_find(size_t current_addr, std::set<size_t>& current_group, int max_depth);
};

#endif