#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <map>

int main()
{
    using namespace std;
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
        int str_pos = 0;
        while (!isspace(buf[str_pos]) && buf[str_pos]) { str_pos++; }
        strncpy(dimension_name, buf, str_pos);
        while (isspace(buf[str_pos]) && buf[str_pos]) { str_pos++; }
        dimension_size = atoi(buf + str_pos);
        cache_info[string(dimension_name)] = dimension_size;
        for (int i = 0; i < 128; ++i) {dimension_name[i] = '\0';}
    }
    /*
    for (auto i : cache_info)
    {
        cout << "Dimension: " << i.first << endl << "Size: " << i.second << endl;
    }
    */
    // critical stride = ((total size / (line size * assoc)) * line size)
    int l1_dcache_critical_stride = (cache_info["LEVEL1_DCACHE_SIZE"] / cache_info["LEVEL1_DCACHE_ASSOC"]);
    int l1_icache_critical_stride = (cache_info["LEVEL1_ICACHE_SIZE"] / cache_info["LEVEL1_ICACHE_ASSOC"]);
    cout << "l1_dcache_critical_stride: " << l1_dcache_critical_stride << endl;
    cout << "l1_icache_critical_stride: " << l1_icache_critical_stride << endl;
    int close_status = pclose(cmd_stream);
    if (close_status)
    {
        cerr << "Error closing stream." << endl;
    }
    return 0;
}