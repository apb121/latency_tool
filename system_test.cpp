#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <map>
#include <unistd.h>

int main()
{
    using namespace std;
    int l1d_size = sysconf(_SC_LEVEL1_DCACHE_SIZE);
    cout << l1d_size << endl;
    return 0;
    char buf[512];
    const char* getconf_cmd = "getconf -a | grep CACHE";
    map<string, size_t> cache_info;
    FILE* cmd_stream;
    cmd_stream = popen(getconf_cmd, "r");
    if (!cmd_stream)
    {
        cerr << "Error opening stream and executing command." << endl;
    }
    while (fgets(buf, sizeof buf, cmd_stream))
    {
        char dimension_name[128];
        size_t dimension_size;
        int buf_begin = 0, str_pos = 0;
        while (isspace(buf[buf_begin])) { buf_begin++; }
        str_pos = buf_begin;
              while (!isspace(buf[str_pos]) && buf[str_pos]) { str_pos++; }
        if (!buf[str_pos])
        {
          cerr << "Error reading getconf output: end of line with no dimension name." << endl;
          continue;
        }
              strncpy(dimension_name, buf + buf_begin, str_pos - buf_begin);
              while (buf[str_pos] && isspace(buf[str_pos])) { str_pos++; }
        if (!buf[str_pos])
        {
          cerr << "Error reading getconf output: " << dimension_name << " does not specify its size." << endl;
          continue;
        }
        int str_end = str_pos;
        while (isdigit(buf[str_end]) || isspace(buf[str_end])) { str_end++; }
        if (buf[str_end])
        {
          cerr << "Error reading getconf output: " << dimension_name << " size is not numerical." << endl;
          continue;
        }
        cache_info[string(dimension_name)] = atoi(buf + str_pos);
        for (int i = 0; i < 128; ++i) { dimension_name[i] = '\0'; }
    }
    // critical stride = ((total size / (line size * assoc)) * line size);
    int l1_dcache_critical_stride = 0;
    int l1_icache_critical_stride = 0;
    if (cache_info["LEVEL1_DCACHE_ASSOC"])
    {
      l1_dcache_critical_stride = (cache_info["LEVEL1_DCACHE_SIZE"] / cache_info["LEVEL1_DCACHE_ASSOC"]);
    }
    else
    {
      //set some variable indicating that an empirical test is needed
      cerr << "LEVEL1_DCACHE_ASSOC does not specify its size." << endl;
    }
    if (cache_info["LEVEL1_ICACHE_ASSOC"])
    {
      l1_icache_critical_stride = (cache_info["LEVEL1_ICACHE_SIZE"] / cache_info["LEVEL1_ICACHE_ASSOC"]);
    }
    else
    {
      //set some variable indicating that an empirical test is needed
      cerr << "LEVEL1_ICACHE_ASSOC does not specify its size." << endl;
    }
    cout << "This processor ";
    cache_info["LEVEL1_DCACHE_SIZE"]
      ? cout << "has " << cache_info["LEVEL1_DCACHE_SIZE"] << " bytes of "
      : cout << "does not have an ";
    cout << "L1 data cache." << endl;

    cout << "This processor ";
    cache_info["LEVEL1_ICACHE_SIZE"]
      ? cout << "has " << cache_info["LEVEL1_ICACHE_SIZE"] << " bytes of "
      : cout << "does not have an ";
    cout << "L1 instruction cache." << endl;

    cout << "This processor ";
    cache_info["LEVEL2_CACHE_SIZE"]
      ? cout << "has " << cache_info["LEVEL2_CACHE_SIZE"] << " bytes of "
      : cout << "does not have an ";
    cout << "L2 cache." << endl;

    cout << "This processor ";
    cache_info["LEVEL3_CACHE_SIZE"]
      ? cout << "has " << cache_info["LEVEL3_CACHE_SIZE"] << " bytes of "
      : cout << "does not have an ";
    cout << "L3 cache." << endl;

    cout << "This processor ";
    cache_info["LEVEL4_CACHE_SIZE"]
      ? cout << "has " << cache_info["LEVEL4_CACHE_SIZE"] << " bytes of "
      : cout << "does not have an ";
    cout << "L4 cache." << endl;
    if (l1_dcache_critical_stride)
    {
      cout << "l1_dcache_critical_stride: " << l1_dcache_critical_stride << endl;
    }
    else
    {
      cout << "l1_dcache_critical_stride unknown." << endl;
    }
    if (l1_icache_critical_stride)
    {
      cout << "l1_icache_critical_stride: " << l1_icache_critical_stride << endl;
    }
    else
    {
      cout << "l1_icache_critical_stride unknown." << endl;
    }
    int close_status = pclose(cmd_stream);
    if (close_status)
    {
        cerr << "Error closing stream." << endl;
    }
    return 0;
}
