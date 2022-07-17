#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <map>
#include <unistd.h>
#include <fstream>

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

    char buf[65536];
    string cmd_compile = "g++ -g -falign-functions=65536 ./associativity_test_i.cpp -o ./associativity_test_i";
    string cmd_no_arg = "perf stat -x , --append -o assoctmp.txt -e L1-icache-load-misses -r 1000 ./associativity_test_i ";
    string cmd_rm_assoctmp = "rm ./assoctmp.txt";
    int ret_compile = system(cmd_compile.c_str());
    if (ret_compile)
    {
      cout << "Compilation failed!" << endl;
    }
    int ret_rm_assoctmp = system(cmd_rm_assoctmp.c_str());
    for (int i = 2; i <= 16; i += 2)
    {
      string cmd_full = cmd_no_arg + to_string(i);
      FILE* cmd_stream;
      cmd_stream = popen(cmd_full.c_str(), "r");
      pclose(cmd_stream);
    }
    ifstream assoc_file;
    assoc_file.open("./assoctmp.txt");
    string atstr;
    int assoc = -1;
    int assoc_arr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int i = 0;
    while (getline(assoc_file, atstr))
    {
      if (atstr.size() == 0 || atstr[0] == '#')
      {
        continue;
      }
      size_t found = atstr.find_first_not_of("0123456789");
      if (found == 0)
      {
        continue;
      }
      assoc_arr[i] = stoi(atstr.substr(0, found));
      if (i > 1 && assoc_arr[i] - assoc_arr[i - 1] < assoc_arr[i - 1] - assoc_arr[i - 2])
      {
        cout << "Suggested associativity: " << ((i - 1) + 1) * 2 << endl;
        break;
      }
      i++;
    }
    return 0;
}
