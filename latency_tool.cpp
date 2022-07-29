#include <iostream>
#include <bit>
#include <bitset>

#include "cache_info.hpp"
#include "class_parser.hpp"
#include "user_options.hpp"
#include "binary_analyser.hpp"

/*
    normal usage:

            ./latency_tool <binary_file> <source_files>

    flags:
            -s, --source-code-only {7}
            -b, --binary-only {6} (?) (make sure to look at all functions with binary only)
            -c, --cache-info-only {5}
            -m=..., --manual-cache=<size>:<associativity>:<linesize>::... {4}
            -n, --no-empirical-tests (default critical stride = 4096?, default assocativity = 8?) {3}
            -k, --keep-temporary-files (make sure to add checks for if files exist before deleting them) {2}
            -l= --coexecution-level= (default = ? maybe 1)
            -t= --competition-threshold= (default = ? maybe 64, or maybe 16)
            -r= --ranking-length= (default = 10)
            -e, --use-existing-temp-files (automatically switches on -k flag) {1}
*/

int main(int argc, char** argv)
{
    UserOptions uo;

    UDType a("", "");
    a.a[0] = 'b';
    a.aa[0] = 'c';

    int ret = uo.parse_flags(argc, argv);
    if (ret) { return ret; }

    ret = uo.run_cache_setup();
    if (ret) { return ret; }

    // serious issue with source code only option and using binary for class info

    FileCollection fc(uo.file_names);
    fc.detect_types();

    return 0;

    ret = uo.run_analysis();
    if (ret) { return ret; }

    return 0;
}