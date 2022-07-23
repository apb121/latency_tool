#include <iostream>
#include <fstream>

#include "binary_analyser.hpp"
#include "user_options.hpp"
#include "class_parser.hpp"

void Binary::get_functions(UserOptions& uo)
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
    /*
        identifies functions from the disassembled binary.
        this is better than using the source code for numerous reasons;
        for example, it will identify compiler-generated functions that
        might cause cache issues, such as
        -- default constructors
        -- copy constructors
        -- move constructors
        -- default/copy/move operator overloads (e.g., assignment)
        -- destructors
        -- (templated functions?)
        it will also avoid wasting time on functions that are written
        but are never called. these do not make their way into the binary
    */
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
        /*
            type T: functions in the code section
            type W: weak object, e.g., function in a class
        */
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
        /*
            identify functions of user-defined classes.
            uses the output of the class parser and the :: operator
        */
        bool UDT_func = false;
        for (size_t i = 0; i < user_types.size(); ++i)
        {
            if (name.substr(0, user_types[i].get_name().length() + 2) == user_types[i].get_name() + "::")
            {
                UDT_func = true;
            }
        }
        if (UDT_func == false/* && !uo.flags.test(BINARY_ONLY) */ && buf[2][0] != 'T')
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

        /* (nm includes duplicates for some reason...) */

        functions_list.insert({address, Function(address, size, name, location)});
        
        nm_file >> std::ws;
    }
}

void Binary::populate_competition_vectors(UserOptions& uo)
{
    /*
        identify functions that compete with each other for cache sets
    */
    int critical_stride = uo.proc.l1i->get_critical_stride();
    int* span = new int[critical_stride];
    for (auto& i : functions_list)
    {
        for (int j = 0; j < critical_stride; ++j)
        {
            span[j] = 0;
        }
        size_t start = i.second.get_address();
        for (size_t j = 0; j < i.second.get_size(); ++j)
        {
            span[(start + j) % critical_stride] = 1;
        }
        for (auto j : functions_list)
        {
            if (j.second.get_address() == i.second.get_address()) { continue; }
            int comp_stride = 0;
            for (size_t k = 0; k < j.second.get_size(); ++k)
            {
                if (span[(j.second.get_address() + k) % critical_stride] == 1)
                {
                    ++comp_stride;
                }
            }
            if (comp_stride > uo.comp) // if the whole group must overlap by this much, so must each member
            {
                i.second.competes_with.insert(j.second.get_address());
            }
        }
    }
    delete [] span;
}

void Binary::populate_coexecution_vectors(UserOptions& uo)
{
    /*
        identify functions that call/are called by each other
        (these are assumed to execute together and therefore
        cache set conflicts become a problem)
    */
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
        size_t pos = 0;
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

    /*
        specify different levels of indirection in coexecution.
        e.g., with one level of indirection, if A() calls B()
        and B() calls C(), A is deemed to coexecute with C.
        similarly, if D() calls E() and D() also calls F(),
        E will be deemed to coexecute with F.
    */

    int num_extra_levels = uo.coex;

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

void Binary::rec_problem_find(size_t current_addr, std::set<size_t>& current_group, int max_depth)
{
    /*
        recursively find groups of functions which
        coexecute with each other and
        compete for cache sets
    */
    for (auto& next_func : functions_list[current_addr].competes_and_coexecutes_with)
    {
        if (current_group.find(next_func) != current_group.end())
        {
            continue;
        }
        bool competes_and_coexecutes_with_all = true;
        for (auto& group_funcs : current_group)
        {
            if (functions_list[group_funcs].competes_and_coexecutes_with.find(next_func) == functions_list[group_funcs].competes_and_coexecutes_with.end())
            {
                competes_and_coexecutes_with_all = false;
                break;
            }
        }
        if (!competes_and_coexecutes_with_all) { continue; }
        if (competes_and_coexecutes_with_all)
        {
            current_group.insert(next_func);
            if (current_group.size() == max_depth)
            {
                problem_groups.insert(current_group);
            }
            else
            {
                rec_problem_find(next_func, current_group, max_depth);
            }
            current_group.erase(current_group.find(next_func));
        }
    }
}

void Binary::find_problem_function_groups(UserOptions& uo)
{
    /*
        find groups of functions that both
        (a) coexecute with each other
        (b) compete for cache sets
        the group-size at which this becomes dangerous
        depends on the associativity of the cache
    */

    for (auto& i : functions_list)
    {
        std::set_intersection(
            i.second.competes_with.begin(), i.second.competes_with.end(),
            i.second.coexecutes_with.begin(), i.second.coexecutes_with.end(),
            std::inserter(i.second.competes_and_coexecutes_with, i.second.competes_and_coexecutes_with.begin())
        );
    }

    std::set<size_t> current_group;

    for (auto& i : functions_list)
    {
        current_group.insert(i.first);
        rec_problem_find(i.first, current_group, uo.proc.l1d->get_assoc());
        current_group.erase(current_group.find(i.first));
    }

    if (problem_groups.size() == 0)
    {
        std::cout << "Based on the instruction-cache's critical stride of " << uo.proc.l1i->get_critical_stride() << " bytes and associativity of " << uo.proc.l1i->get_assoc() << " and the chosen levels of coexecution indirecion and competition overlap, no cache-competition related sources of latency problems have been identified!" << std::endl << std::endl;
        return;
    }

    std::cout << "The following groups of functions have been identified as a potential source of latency problems." << std::endl;
    std::cout << "This has been calculated based on the instruction-cache's critical stride of " << uo.proc.l1i->get_critical_stride() << " bytes and associativity of " << uo.proc.l1i->get_assoc() << ", as well as the chosen coexecution indirection level of " << uo.coex << " and competition overlap threshold of " << uo.comp << " bytes." << std::endl << std::endl;
    
    int num_groups = 0;
    for (auto& i : problem_groups)
    {
        int overlap[4096];
        for (int o = 0; o < 4096; ++o)
        {
            overlap[o] = 1;
        }
        for (auto& j : i)
        {
            for (int o = functions_list[j].get_size(); o < 4096; ++o)
            {
                overlap[(functions_list[j].get_address() + o) % 4096] = 0;
            }
        }
        int count = 0;
        for (int i = 0; i < 4096; ++i)
        {
            if (overlap[i] == 1) { ++count; }
        }

        if (count >= uo.comp)
        {
            ++num_groups;
            std::cout << "===Group===" << std::endl << std::endl;
            for (auto& j : i)
            {
                std::cout << functions_list[j].get_name() << std::endl;
            }
            std::cout << std::endl << "This group overlaps by " << count << " bytes" << std::endl;
            std::cout << std::endl;
        }
    }
    std::cout << "There are " << num_groups << " overlapping groups." << std::endl << std::endl;
}