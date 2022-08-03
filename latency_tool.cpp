#include <iostream>
#include <bit>
#include <bitset>

#define ALL_FUNCTIONS 6
#define CACHE_INFO_ONLY 5
#define MANUAL_CACHE 4
#define NO_EMPIRICAL 3
#define KEEP_TEMP 2
#define EXISTING_TEMP_FILES 1

#include "cache_info.hpp"
#include "data_analyser.hpp"
#include "user_options.hpp"
#include "binary_analyser.hpp"

/*
    normal usage:

            ./latency_tool <binary_file> <source_files>

    flags:
            -a, --all-functions {6} (?) (make sure to look at all functions with binary only)
            -c, --cache-info-only {5}
            -m=..., --manual-cache=<size>:<associativity>:<linesize>::... {4}
            -n, --no-empirical-tests (default critical stride = 4096, default assocativity = 8) {3}
            -k, --keep-temporary-files {2}
            -l= --coexecution-level= (default = 1)
            -t= --competition-threshold= (default = 256)
            -r= --ranking-length= (default = 10)
            -e, --use-existing-cache-temp-files (automatically switches on -k flag) {1}s
*/

int main(int argc, char** argv)
{
    TemplateClass<int> i; // remove these + test classes from hpps
    int i_t = i.get_t();

    TemplateClass<char> c;
    char c_t = c.get_t();

    Large l;
    l.a[0] = 'a';

    Inefficient in;
    in.c = 'c';

    UserOptions uo;

    int ret = uo.parse_flags(argc, argv);
    if (ret) { return ret; }

    ret = uo.run_file_setup();
    if (ret) { return ret; }

    ret = uo.check_requirements();
    if (ret) { return ret; }

    ret = uo.run_cache_setup();
    if (ret) { return ret; }

    if (uo.flags.test(CACHE_INFO_ONLY)) { return 0; }

    ret = uo.run_analysis();
    if (ret) { return ret; }

    return 0;
}