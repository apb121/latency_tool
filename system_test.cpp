#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <map>
#include <unistd.h>

using namespace std;

struct Cache
{
  std::string name = "";
  size_t size = 0;
  int linesize = -1;
  int assoc = -1;
  int critical_stride = -1;
  Cache(std::string name, size_t size, int linesize, int assoc)
    : name(name), size(size), linesize(linesize), assoc(assoc)
  {
    if (size <= 0)
    {
      cerr << "This processor does not have an " << name << " cache" << endl;
    }
    else if (assoc <= 0)
    {
      cerr << "This processor does not specify the associativity of its " << name << " cache." << endl;
    }
    else
    {
      critical_stride = size / assoc;
      cout << "The critical stride of the " << name << " cache is " << critical_stride << " bytes." << endl;
    }
  }
};

int main()
{
    using namespace std;
    Cache l1d("L1d",
		   sysconf(_SC_LEVEL1_DCACHE_SIZE),
		   sysconf(_SC_LEVEL1_DCACHE_LINESIZE),
		   sysconf(_SC_LEVEL1_DCACHE_ASSOC));
    Cache l1i("L1i",
		   sysconf(_SC_LEVEL1_ICACHE_SIZE),
		   sysconf(_SC_LEVEL1_ICACHE_LINESIZE),
		   sysconf(_SC_LEVEL1_ICACHE_ASSOC));
    Cache l2("L2",
		   sysconf(_SC_LEVEL2_CACHE_SIZE),
		   sysconf(_SC_LEVEL2_CACHE_LINESIZE),
		   sysconf(_SC_LEVEL2_CACHE_ASSOC));
    Cache l3("L3",
		   sysconf(_SC_LEVEL3_CACHE_SIZE),
		   sysconf(_SC_LEVEL3_CACHE_LINESIZE),
		   sysconf(_SC_LEVEL3_CACHE_ASSOC));
    Cache l4("L4",
		   sysconf(_SC_LEVEL4_CACHE_SIZE),
		   sysconf(_SC_LEVEL4_CACHE_LINESIZE),
		   sysconf(_SC_LEVEL4_CACHE_ASSOC));
    
    return 0;
}
