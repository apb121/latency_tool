#include "binary_analyser.hpp"

int Binary::get_functions(UserOptions& uo)
{
    std::string nm_cmd = "nm -v -C -l --radix=d --print-size " + file_name + " > ./temp_files/nmtmp.txt";
    if (uo.flags.test(EXISTING_TEMP_FILES))
    {
        std::ifstream exist_test("./temp_files/nmtmp.txt");
        if (!exist_test.good())
        {
            exist_test.close();
            std::cout << std::endl << std::endl << "There is no existing temp (nm) file for the binary!" << std::endl;
            return -1;
        }
    }
    else
    {
        int ret_nm = system(nm_cmd.c_str());
        if (ret_nm)
        {
            std::cout << "nm failed!" << std::endl;
            return -1;
        }
    }
    char buf[5][512];
    std::ifstream nm_file;
    nm_file.open("./temp_files/nmtmp.txt");
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
        nm_file >> std::ws;
        std::string name = "";
        char c = nm_file.get();
        while (c != '(' && c != '\n' /*!isspace(c)*/)
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
        if (!uo.flags.test(ALL_FUNCTIONS) && name == "_start")
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
        if (!uo.flags.test(ALL_FUNCTIONS) && UDT_func == false && buf[2][0] != 'T')
        {
            while (nm_file.get() != '\n' && !nm_file.eof()) {};
            continue;
        }
        std::string location;
        while (!nm_file.eof() && c != '\n' && c != '/')
        {
            c = nm_file.get();
        }
        location += c;
        nm_file >> buf[4];
        location += buf[4];
        size_t address = atoi(buf[0]);
        size_t size = atoi(buf[1]);

        if (size < uo.comp)
        {
            continue;
        }

        /* (nm includes duplicates for some reason...) */

        functions_list.insert({address, Function(address, size, name, location)});
        
        nm_file >> std::ws;
    }
    return 0;
}

int Binary::populate_competition_vectors(UserOptions& uo)
{
    /*
        identify functions that compete with each other for cache sets
    */

    int critical_stride = 0;
    if (uo.proc.l1i)
    {
        critical_stride = uo.proc.l1i->get_critical_stride();
    }
    if (critical_stride <= 0)
    {
        std::cout << std::endl << "(Warning: no L1 instruction cache critical stride was specified. Defaulting to 4096 bytes.)" << std::endl << std::endl;
        critical_stride = 4096;
    }
    int associativity = 0;
    if (uo.proc.l1i)
    {
        associativity = uo.proc.l1i->get_assoc();
    }
    if (associativity == 0)
    {
        std::cout << std::endl << "(Warning: no L1 instruction cache associativity was specified. Defaulting to 8.)" << std::endl << std::endl;
        associativity = 8;
    }
    int* span = new int[critical_stride];
    for (auto& i : functions_list)
    {
        if (i.second.get_size() < uo.comp)
        {
            continue;
        }
        if (i.second.get_size() > critical_stride)
        {
            std::cout << std::endl << "=== Large function detected ===" << std::endl << std::endl;
            std::cout << "Function name: " << i.second.get_name() << std::endl << std::endl;
            std::cout << "This function spans between " << i.second.get_size() / critical_stride << " and " << (i.second.get_size() / critical_stride) + 1 << " critical strides. This means that, when it executes, it will occupy " << (i.second.get_size() / critical_stride) + 1 << " cache ways, and may therefore compete with itself for cache space." << std::endl << std::endl;
        }
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
            
            if (j.second.get_size() < uo.comp)
            {
                continue;
            }
            
            if (j.second.get_address() == i.second.get_address()) { continue; }
            int comp_stride = 0;
            for (size_t k = 0; k < j.second.get_size(); ++k)
            {
                if (span[(j.second.get_address() + k) % critical_stride] == 1)
                {
                    ++comp_stride;
                }
            }
            if (comp_stride > uo.comp) /* if the whole group must overlap by this much, so must each member */
            {
                i.second.competes_with.insert(j.second.get_address());
            }
        }
    }
    delete [] span;
    return 0;
}

int Binary::populate_coexecution_vectors(UserOptions& uo)
{
    /*
        identify functions that call/are called by each other
        (these are assumed to execute together and therefore
        cache set conflicts become a problem)
    */
    std::string objdump_cmd = "objdump -d -C -Mintel --no-show-raw-insn " + file_name + " > ./temp_files/objdumptmp.txt";
    std::string objdump_rm = "rm ./temp_files/objdumptmp.txt";
    if (uo.flags.test(EXISTING_TEMP_FILES))
    {
        std::ifstream exist_test("./temp_files/objdumptmp.txt");
        if (!exist_test.good())
        {
            exist_test.close();
            std::cout << std::endl << std::endl << "There is no existing temp (objdump) file for the binary!" << std::endl;
            return 1;
        }
    }
    else
    {
        int ret_objdump = system(objdump_cmd.c_str());
        if (ret_objdump)
        {
            std::cout << "objdump failed!" << std::endl;
            return 1;
        }
    }
    std::ifstream objdump_file;
    objdump_file.open("./temp_files/objdumptmp.txt");
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

    return 0;
}

int Binary::find_problem_function_groups(UserOptions& uo)
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

    std::multimap<size_t, ProblemGroup> problem_groups_ranked;

    int num_groups = 1;

    int max_group_size = 0;

    int jitter_risks = 0;

    int eviction_risks = 0;

    for (int depth = 2 /*uo.proc.l1i->get_assoc() / 2*/; num_groups != 0; ++depth)
    {
        problem_groups.clear();

        std::set<size_t> current_group;

        /* perform recursive search for problem groups of functions */

        for (auto& i : functions_list)
        {
            current_group.insert(i.first);
            rec_problem_find(i.first, current_group, depth);
            current_group.erase(current_group.find(i.first));
        }

        num_groups = problem_groups.size();

        if (num_groups > 0)
        {
            max_group_size = depth;
        }

        if (depth > uo.proc.l1i->get_assoc() - 4 && depth <= uo.proc.l1i->get_assoc())
        {
            jitter_risks += num_groups;
        }
        else if (depth > uo.proc.l1i->get_assoc())
        {
            eviction_risks += num_groups;
        }

        /* rank problem groups */

        for (auto& group : problem_groups)
        {
            int overlap[4096];
            for (int o = 0; o < 4096; ++o)
            {
                overlap[o] = 1;
            }
            for (auto& j : group)
            {
                for (int o = functions_list[j].get_size(); o < 4096; ++o)
                {
                    overlap[(functions_list[j].get_address() + o) % 4096] = 0;
                }
            }
            size_t overlap_extent = 0;
            for (int i = 0; i < 4096; ++i)
            {
                if (overlap[i] == 1) { ++overlap_extent; }
            }
            /* calculate how many times these functions call each other */

            size_t coexecution_count = 0;
            std::ifstream objdump_file("./temp_files/objdumptmp.txt");
            
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
                bool is_relevant = false;
                long dec_addr_src = strtol(address.c_str(), nullptr, 16);
                for (auto& i : group)
                {
                    if ((dec_addr_src > functions_list[i].get_address()) && (dec_addr_src < functions_list[i].get_address() + functions_list[i].get_size()))
                    {
                        is_relevant = true;
                        break;
                    }
                }
                if (!is_relevant) { continue; }
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
                        long dec_addr_dest = strtol(call_dest.c_str(), nullptr, 16);
                        for (auto& i : group)
                        {
                            if (functions_list[i].get_address() == dec_addr_dest)
                            {
                                coexecution_count++;
                                break;
                            }
                        }
                    }
                }
            }
            size_t group_score = (pow(2, group.size()) * overlap_extent) * coexecution_count;
            problem_groups_ranked.insert(std::pair<size_t, ProblemGroup>(group_score, ProblemGroup(group, coexecution_count, overlap_extent)));
        }
    }

    int critical_stride = 0;
    if (uo.proc.l1i)
    {
        critical_stride = uo.proc.l1i->get_critical_stride();
    }
    if (critical_stride == 0)
    {
        critical_stride = 4096;
    }
    int associativity = 0;
    if (uo.proc.l1i)
    {
        associativity = uo.proc.l1i->get_assoc();
    }
    if (associativity == 0)
    {
        associativity = 8;
    }

    if (problem_groups_ranked.size() == 0)
    {
        std::cout << "Based on the instruction-cache's critical stride of " << critical_stride << " bytes and associativity of " << associativity << " and the chosen levels of coexecution indirecion and competition overlap, no cache-competition related sources of latency problems have been identified!" << std::endl << std::endl;
        return 0;
    }

    std::cout << std::endl << "===== Function group analysis =====" << std::endl << std::endl;
    std::cout << "The following groups of functions have been identified as potential sources of latency problems." << std::endl;
    std::cout << "They have been ranked based on a combination of the number of functions in the group and the amount of cache space they compete for." << std::endl;
    std::cout << "This has been calculated based on the instruction-cache's critical stride of " << critical_stride << " bytes and associativity of " << associativity << ", as well as a coexecution indirection level of " << uo.coex << " and competition overlap threshold of " << uo.comp << " bytes." << std::endl << std::endl;
    
    int ranking_num = 0;

    for (std::map<size_t, ProblemGroup>::reverse_iterator i = problem_groups_ranked.rbegin(); i != problem_groups_ranked.rend() && ranking_num < uo.ranking_length; ++i)
    {
        std::cout << "=== Group ===" << std::endl << std::endl;
        size_t score = i->first;
        std::set<size_t> group = i->second.functions;
        for (auto& j : group)
        {
            std::cout << functions_list[j].get_name() << std::endl;
        }

        std::cout << std::endl << "These " << group.size() << " coexecuting functions call each other (directly) " << i->second.coexecutions << " times and compete for the same " << i->second.overlap << "-byte region of the cache" << std::endl << std::endl;

        ++ranking_num;
    }

    // print some analysis including whether
    // the number of functions in the groups indicate that there could be any jitter
    // or indeed any serious competition for the cache

    if (problem_groups_ranked.size() > 1)
    {
        std::cout << std::endl << std::endl << "===== Description =====" << std::endl << std::endl;
        std::cout << "The largest identified group of coexecuting functions that compete for cache space contains " << max_group_size << " functions." << std::endl << std::endl;
        if (jitter_risks > 0)
        {
            std::cout << "The number of groups whose size is close to or equal to the associativity of the L1 instruction cache is: " << jitter_risks << "." << std::endl;
            std::cout << "Such groups of functions may cause an increase in the standard deviation (jitter) among run-time latencies, because there is a risk of cache evictions." << std::endl << std::endl;
            if (eviction_risks > 0)
            {
                std::cout <<  "The number of groups whose size is above the associativity of the L1 instruction cache is: " << eviction_risks << "." << std::endl;
                std::cout << "Such groups of functions are likely to cause an increase in latencies, because they will evict each other from the cache." << std::endl << std::endl;
            }
            std::cout << "The suggestions below may help to mitigate problems that may arise as a result of these functions competing for cache space." << std::endl;
        }
        else
        {
            std::cout << "The size of this group of functions is well below the associativity of the L1 instruction cache, and should therefore not be a source of serious latency issues. It may still be worth trying to reduce the competition between these functions by following the suggestions below." << std::endl;
        }
        // max group size
        // number of groups that may cause jitter / cache evictions
        std::cout << std::endl << std::endl << "===== Suggestions =====" << std::endl << std::endl;
        std::cout << "There are a number of things that programmers can do to mitigate problems that might be identified in this analysis." << std::endl << std::endl;
        std::cout << "(1) rewrite the functions to involve less code!" << std::endl;
        std::cout << "(2) inline small functions that may evict parts of larger functions from the cache so that they do not conflict for cache space" << std::endl;
        std::cout << "(3) combine smaller functions that compete for cache space into fewer, longer functions" << std::endl;
        std::cout << "(4) move functions around in the code so that functions that are called together are placed together in memory and will therefore occupy non-overlapping regions of the cache" << std::endl;
        std::cout << "(5) try the gcc flag --falign-functions=<alignment>, which will force functions onto the specified alignment and which may improve the situation - but may also make it worse!" << std::endl;
    }

    return 0;
}

int Binary::rec_problem_find(size_t current_addr, std::set<size_t>& current_group, int max_depth)
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
    return 0;
}