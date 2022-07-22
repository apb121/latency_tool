#include <iostream>
#include <tuple>
#include <fstream>
#include <vector>

class Function
{
    size_t address;
    size_t size;
    std::string name;
    std::string file_location;
    public:
    std::set<size_t> competes_with; //competes for cache sets with
    std::set<size_t> coexecutes_with; //often executed in conjunction with (according to the objdump)
    std::set<size_t> competes_and_coexecutes_with;
    Function(size_t address, size_t size, std::string name, std::string file_location)
        : address(address), size(size), name(name), file_location(file_location)
    {

    }
    Function()
    {
        std::cout << "If we get here, we've got a problem!" << std::endl;
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
        : file_name(file_name), user_types(user_types)
    {

    }
    void get_functions()
    {
        std::string nm_cmd = "nm -v -C -l --radix=d --print-size " + file_name + " >> nmtmp.txt";
        std::string nm_rm = "rm nmtmp.txt";
        system(nm_rm.c_str());
        int ret_nm = system(nm_cmd.c_str());
        if (ret_nm)
        {
            std::cout << "nm failed!" << std::endl;
        }
        std::ifstream nm_file;
        char buf[5][512];
        nm_file.open("nmtmp.txt");
        while (!nm_file.eof())
        {
            nm_file >> buf[0];
            if (!isdigit(buf[0][0]))
            {
                while (nm_file.get() != '\n' && !nm_file.eof()) {};
                continue;
            }
            nm_file >> buf[1];
            if (!isdigit(buf[1][0]))
            {
                while (nm_file.get() != '\n' && !nm_file.eof()) {};
                continue;
            }
            nm_file >> buf[2];
            if (buf[2][0] != 'T' && buf[2][0] != 'W')
            {
                while (nm_file.get() != '\n' && !nm_file.eof()) {};
                continue;
            }
            nm_file >> std::ws;
            std::string name = "";
            char c = nm_file.get();
            while (c != '(' && !isspace(c))
            {
                name += c;
                c = nm_file.get();
            }
            if (c == '\n')
            {
                continue;
            }
            if (c == '(')
            {
                name += c;
                int bracket_depth = 1;
                c = nm_file.get();
                while (bracket_depth > 0)
                {
                    if (c == '(') { bracket_depth += 1; }
                    if (c == ')') { bracket_depth -= 1; }
                    name += c;
                    c = nm_file.get();
                }
            }
            if (name == "_start")
            {
                while (nm_file.get() != '\n' && !nm_file.eof()) {};
                continue;
            }
            bool UDT_func = false;
            for (int i = 0; i < user_types.size(); ++i)
            {
                if (name.substr(0, user_types[i].get_name().length() + 2) == user_types[i].get_name() + "::")
                {
                    UDT_func = true;
                }
            }
            if (UDT_func == false && buf[2][0] != 'T')
            {
                while (nm_file.get() != '\n' && !nm_file.eof()) {};
                continue;
            }
            std::string location;
            while (c != '/')
            {
                c = nm_file.get();
            }
            location += c;
            nm_file >> buf[4];
            location += buf[4];
            size_t address = atoi(buf[0]);
            size_t size = atoi(buf[1]);

            /* nm includes duplicates... */

            functions_list.insert({address, Function(address, size, name, location)});
            
            nm_file >> std::ws;
        }
    }
    void populate_competition_vectors(int critical_stride)
    {
        int span[critical_stride];
        for (auto& i : functions_list)
        {
            for (int j = 0; j < critical_stride; ++j)
            {
                span[j] = 0;
            }
            size_t start = i.second.get_address();
            for (int j = 0; j < i.second.get_size(); ++j)
            {
                span[(start + j) % critical_stride] = 1;
            }
            for (auto j : functions_list)
            {
                if (j.second.get_address() == i.second.get_address()) { continue; }
                for (int k = 0; k < j.second.get_size(); ++k)
                {
                    if (span[(j.second.get_address() + k) % critical_stride] == 1)
                    {
                        i.second.competes_with.insert(j.second.get_address()); //the other way around is added because all pairs are checked twice anyway
                        break;
                    }
                }
            }
        }
    }
    void populate_coexecution_vectors()
    {
        std::string objdump_cmd = "objdump -d -C -Mintel --no-show-raw-insn " + file_name + " >> objdumptmp.txt";
        std::string objdump_rm = "rm objdumptmp.txt";
        system(objdump_rm.c_str());
        int ret_objdump = system(objdump_cmd.c_str());
        if (ret_objdump)
        {
            std::cout << "objdump failed!" << std::endl;
        }
        std::ifstream objdump_file;
        objdump_file.open("objdumptmp.txt");
        std::string line;
        objdump_file >> std::ws;
        while(getline(objdump_file, line))
        {
            int pos = 0;
            while (pos < line.length() && isspace(line[pos])) { pos++; }
            if (!isdigit(line[pos])) { continue; }
            line = line.substr(pos);
            int end = line.find_first_not_of("0123456789abcdef");
            std::string address = line.substr(0, end);
            pos = end;
            while (pos < line.length() && isspace(line[pos]) || line[pos] == ':') { pos++; }
            int ins_start = pos;
            while (pos < line.length() && !isspace(line[pos])) { pos++; }
            int ins_end = pos;
            std::string instruction = line.substr(ins_start, ins_end - ins_start);
            std::string call_dest;
            if (instruction == "call")
            {
                pos = line.find("call") + 4;
                while (pos < line.length() && isspace(line[pos])) { pos++; }
                int dest_start = pos;
                while (pos < line.length() && !isspace(line[pos])) { pos++; }
                int dest_end = pos;
                call_dest = line.substr(dest_start, dest_end - dest_start);
                if (call_dest.find_first_not_of("0123456789abcdef") == std::string::npos)
                {
                    for (auto& i : functions_list)
                    {
                        long dec_addr_src = strtol(address.c_str(), nullptr, 16);
                        if ((dec_addr_src >= i.second.get_address()) && (dec_addr_src < i.second.get_address() + i.second.get_size()))
                        {
                            for (auto& j : functions_list)
                            {
                                if (j.first == i.first) { continue; }
                                long dec_addr_dest = strtol(call_dest.c_str(), nullptr, 16);
                                if (j.second.get_address() == dec_addr_dest)
                                {
                                    i.second.coexecutes_with.insert(dec_addr_dest);
                                    j.second.coexecutes_with.insert(i.second.get_address());
                                }
                            }
                        }
                    }
                }
            }
        }
        // level 2 ?

        int num_extra_levels = 1; // number of levels of indirection of coexecution

        for (int level = 0; level < num_extra_levels; ++level)
        {
            for (auto& i : functions_list)
            {
                for (auto& j : i.second.coexecutes_with)
                {
                    for (auto& k : functions_list[j].coexecutes_with)
                    {
                        i.second.coexecutes_with.insert(k);
                    }
                }
            }
        }
    }
    void find_problem_function_groups()
    {
        for (auto& i : functions_list)
        {
            std::set_intersection(
                i.second.competes_with.begin(), i.second.competes_with.end(),
                i.second.coexecutes_with.begin(), i.second.coexecutes_with.end(),
                std::inserter(i.second.competes_and_coexecutes_with, i.second.competes_and_coexecutes_with.begin())
            );
        }

        // after the function...
        // hopefully find a way to evalute the problematic-ness of the group
            // size of group
            // number of coexecutions
            // amount of overlap

        std::set<size_t> current_group;
        int overlap[4096];

        for (auto& i : functions_list)
        {
            std::cout << "Outer search: " << functions_list[i.first].get_name() << ": " << i.first << std::endl;
            for (int j = 0; j < 4096; ++j) { overlap[j] = 0; }
            current_group.insert(i.first);
            for (int j = 0; j < functions_list[i.first].get_size(); ++j)
            {
                overlap[(functions_list[i.first].get_address() + j) % 4096] = 1;
            }
            rec_problem_find(i.first, current_group, 5, overlap);
            current_group.erase(current_group.find(i.first));
        }
        std::cout << "Ok..." << std::endl;
        std::cout << problem_groups.size() << std::endl;
    }
    void rec_problem_find(size_t current_addr, std::set<size_t>& current_group, int max_depth, int* overlap)
    {
        for (auto& next_func : functions_list[current_addr].competes_and_coexecutes_with)
        {
            if (current_group.find(next_func) != current_group.end())
            {
                continue;
            }
            int new_overlap[4096];
            int next_func_span[4096];
            for (int i = 0; i < 4096; ++i)
            {
                next_func_span[i] = 0;
                new_overlap[i] = overlap[i]; // reset overlap representation for next recursion
            }
            for (int i = 0; i < functions_list[next_func].get_size(); ++i)
            {
                next_func_span[(next_func + i) % 4096] = 1; // overlap of proposed recursion
            }
            bool any_overlap = false;
            for (int i = 0; i < 4096; ++i) // replace with critical stride size
            {
                if (new_overlap[i] > 0 && next_func_span[i] > 0)
                {
                    new_overlap[i] = 1; // overlap of proposed recursion and existing group
                    any_overlap = true;
                }
                else
                {
                    overlap[i] = 0;
                }
            }
            if (any_overlap)
            {
                current_group.insert(next_func);
                if (current_group.size() == max_depth)
                {
                    problem_groups.insert(current_group);
                }
                else
                {
                    rec_problem_find(next_func, current_group, max_depth, new_overlap);
                }
                current_group.erase(current_group.find(next_func));
            }
        }
    }
};