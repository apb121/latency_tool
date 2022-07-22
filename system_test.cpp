#include <iostream>

#include "cache_info.hpp"
#include "class_parser.hpp"
#include "binary_analyser.hpp"

using namespace std;

int main()
{
    using namespace std;
    L1d_cache l1d(
      sysconf(_SC_LEVEL1_DCACHE_SIZE),
      sysconf(_SC_LEVEL1_DCACHE_LINESIZE),
      sysconf(_SC_LEVEL1_DCACHE_ASSOC)
    );
    L1i_cache l1i(
      sysconf(_SC_LEVEL1_ICACHE_SIZE),
      sysconf(_SC_LEVEL1_ICACHE_LINESIZE),
      sysconf(_SC_LEVEL1_ICACHE_ASSOC)
    );
    L2_cache l2(
      sysconf(_SC_LEVEL2_CACHE_SIZE),
      sysconf(_SC_LEVEL2_CACHE_LINESIZE),
      sysconf(_SC_LEVEL2_CACHE_ASSOC)
    );
    L3_cache l3(
      sysconf(_SC_LEVEL3_CACHE_SIZE),
      sysconf(_SC_LEVEL3_CACHE_LINESIZE),
      sysconf(_SC_LEVEL3_CACHE_ASSOC)
    );
    L4_cache l4(
      sysconf(_SC_LEVEL4_CACHE_SIZE),
      sysconf(_SC_LEVEL4_CACHE_LINESIZE),
      sysconf(_SC_LEVEL4_CACHE_ASSOC)
    );

    int ls = l2.get_linesize();
    /*
    int ret = l1d.empirical_assoc_test();

    cout << "Suggested data-cache associativity: " << ret << endl;

    ret = l1i.empirical_assoc_test();

    cout << "Suggested instruction-cache associativity: " << ret << endl;

    ret = l1d.empirical_stride_test();

    cout << "Suggested data-cache critical stride: " << ret << endl;

    ret = l1i.empirical_stride_test();

    cout << "Suggested instruction-cache critical stride: " << ret << endl;
    */
    
    //File class_size_test("./class_test.cpp");

    File class_test("./class_test.cpp");
    File class_parser("./class_parser.hpp");
    File cache_info("./cache_info.hpp");
    File binary_analyser("./binary_analyser.hpp");

    class_test.detect_types();
    // class_test.suggest_optimised_orderings();
    
    class_parser.detect_types();
    // class_parser.suggest_optimised_orderings();

    cache_info.detect_types();
    // cache_info.suggest_optimised_orderings();

    binary_analyser.detect_types();
    // binary_analyser.suggest_optimised_orderings();

    vector<UDType> all_types;
    vector<UDType> types = class_parser.get_types();
    all_types.insert(all_types.end(), types.begin(), types.end());
    types = cache_info.get_types();
    all_types.insert(all_types.end(), types.begin(), types.end());
    types = binary_analyser.get_types();
    all_types.insert(all_types.end(), types.begin(), types.end());

    Binary b("associativity_test_i_4096", all_types);
    b.get_functions();
    b.populate_competition_vectors(4096);
    b.populate_coexecution_vectors();
    b.find_problem_function_groups();

    return 0;
}
