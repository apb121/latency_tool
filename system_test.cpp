#include <iostream>

#include "cache.hpp"
#include "class_parser.hpp"

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

    int ret = l1d.empirical_assoc_test();

    cout << "Suggested data-cache associativity: " << ret << endl;

    ret = l1i.empirical_assoc_test();

    cout << "Suggested instruction-cache associativity: " << ret << endl;

    ret = l1d.empirical_stride_test();

    cout << "Suggested data-cache critical stride: " << ret << endl;

    ret = l1i.empirical_stride_test();

    cout << "Suggested instruction-cache critical stride: " << ret << endl;

    File class_size_test("./class_test.cpp");

    class_size_test.detect_types();
    class_size_test.suggest_optimised_orderings();

    return 0;
}
