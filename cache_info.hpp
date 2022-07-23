#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>

class L1d_cache;
class L1i_cache;
class L2_cache;
class L3_cache;
class L4_cache;

class Cache
{
  std::string name = "generic_cache";
  int size = -1;
  int linesize = -1;
  int assoc = -1;
  int critical_stride = -1;
  public:
  Cache(std::string name, int size, int linesize, int assoc)
    : name(name), size(size), linesize(linesize), assoc(assoc)
  {
    if (size <= 0)
    {
      std::cerr << "This processor does not have an " << name << " cache" << std::endl << std::endl;
    }
    else if (assoc <= 0)
    {
      std::cerr << "This processor does not specify the associativity of its " << name << " cache." << std::endl;
    }
    else if (assoc <= 0)
    {
      std::cerr << "This processor does not specify linesize of its " << name << " cache." << std::endl;
      critical_stride = size / assoc;
      std::cout << "The critical stride of the " << name << " cache is " << critical_stride << " bytes." << std::endl;
    }
    else
    {
      critical_stride = size / assoc;
      std::cout << "The critical stride of the " << name << " cache is " << critical_stride << " bytes." << std::endl;
    }
  }
  const int get_linesize() const { return linesize; }
  const int get_assoc() const { return assoc; }
  const int get_size() const { return size; }
  const int get_critical_stride() const { return critical_stride; }
  void set_linesize(int l) { linesize = l; }
  void set_assoc(int a) { assoc = a; }
  void set_critical_stride(int cs) { critical_stride = cs; }
};

class L1_cache : public Cache
{
    public:
    L1_cache(std::string name, int size, int linesize, int assoc)
        : Cache(name, size, linesize, assoc)
    {

    }
    virtual int empirical_assoc_test() = 0; // these are interesting from the perspective of the class parser!
    virtual int empirical_stride_test() = 0;
};

class L1d_cache : public L1_cache
{
    public:
    L1d_cache(int size, int linesize, int assoc)
        : L1_cache("L1d", size, linesize, assoc)
    {
        
    }
    int empirical_assoc_test()
    {
        /*  
            assesses data cache associativity empirically
        */
        std::cout << "Assessing data-cache associativity empirically" << std::flush;
        std::string cmd_compile = "g++ -g ./associativity_test_d.cpp -o ./associativity_test_d";
        std::string cmd_no_arg = "perf stat -x , --append -o assoctmpd.txt -e L1-dcache-load-misses -r 1000 ./associativity_test_d ";
        std::string cmd_rm_assoctmp = "rm ./assoctmpd.txt";
        int ret_compile = system(cmd_compile.c_str());
        if (ret_compile)
        {
            std::cout << "Compilation failed!" << std::endl;
        }
        /*  
            varying the number of regions that compete for the cache
        */
        int ret_rm_assoctmp = system(cmd_rm_assoctmp.c_str());
        for (int i = 2; i <= 16; i += 2)
        {
            std::cout << "." << std::flush;
            std::string cmd_full_65536 = cmd_no_arg + std::to_string(i) + " 65536";
            std::string cmd_full_16 = cmd_no_arg + std::to_string(i) + " 16";
            FILE* cmd_stream;
            std::string echo = "echo \"#" + std::to_string(i) + "\\n\" >> assoctmpd.txt";
            system(echo.c_str());
            cmd_stream = popen(cmd_full_65536.c_str(), "r"); // these POSIX functions will only work on Linux systems
            pclose(cmd_stream);
            cmd_stream = popen(cmd_full_16.c_str(), "r");
            pclose(cmd_stream);
        }
        std::cout << std::endl << std::flush;
        std::ifstream assoc_file;
        assoc_file.open("./assoctmpd.txt");
        std::string atstr;
        int assoc = -1;
        int assoc_arr_65536[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        int assoc_arr_16[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        double assoc_arr_65536_stddev[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        double assoc_arr_16_stddev[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        int i = 0;
        while (getline(assoc_file, atstr) && i < 16)
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
            size_t end = atstr.find("%"), start = end - 1;
            if (i % 2 == 0)
            {
                assoc_arr_65536[i / 2] = stoi(atstr.substr(0, found));
                while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
                start++;
                assoc_arr_65536_stddev[i / 2] = stod(atstr.substr(start, end - start));
            }
            else
            {
                assoc_arr_16[i / 2] = stoi(atstr.substr(0, found));
                while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
                start++;
                assoc_arr_16_stddev[i / 2] = stod(atstr.substr(start, end - start));
            }
            i++;
        }
        double max_stddev = 0.0;
        int max_stddev_index = -1;
        int first_spike = 9;
        for (int i = 0; i < 8; i++)
        {
            if (assoc_arr_65536_stddev[i] > max_stddev)
            {
                max_stddev = assoc_arr_65536_stddev[i];
                max_stddev_index = i;
            }
            if (assoc_arr_65536[7 - i] > (assoc_arr_16[7 - i] * 2))
            {
                first_spike = 7 - i;
            }
        }
        /*  
            the best estinate is given by taking the minimum
            of the biggest standard deviation and
            the first big spike in number of cache misses
        */
        return (std::min(max_stddev_index, first_spike) + 1) * 2;
    }
    int empirical_stride_test()
    {
        /*  
            assesses data cache critical stride empirically
        */
        std::cout << "Assessing data-cache critical stride empirically" << std::flush;
        std::string cmd_compile = "g++ -g ./critical_stride_test_d.cpp -o ./critical_stride_test_d";
        std::string cmd_no_arg = "perf stat -x , --append -o cstmpd.txt -e L1-dcache-loads,L1-dcache-load-misses,duration_time -r 1000 ./critical_stride_test_d ";
        std::string cmd_rm_assoctmp = "rm ./cstmpd.txt";
        int ret_compile = system(cmd_compile.c_str());
        if (ret_compile)
        {
            std::cout << "Compilation failed!" << std::endl;
        }
        /*  
            varying the alignment to force the data onto
        */
        int ret_rm_assoctmp = system(cmd_rm_assoctmp.c_str());
        for (int i = 64; i <= 65536; i *= 2)
        {
            std::cout << "." << std::flush;
            std::string cmd_full = cmd_no_arg + std::to_string(i);
            FILE* cmd_stream;
            cmd_stream = popen(cmd_full.c_str(), "r");
            pclose(cmd_stream);
        }
        std::cout << std::endl << std::flush;
        std::ifstream assoc_file;
        assoc_file.open("./cstmpd.txt");
        std::string atstr;
        int assoc = -1;
        int assoc_arr_cacheloads[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        double assoc_arr_cacheloads_stddev[11] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        int assoc_arr_cacheloadmisses[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        double assoc_arr_cacheloadmisses_stddev[11] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        int assoc_arr_ns[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        double assoc_arr_ns_stddev[11] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        int i = 0;
        size_t end = atstr.find("%"), start = end - 1;
        while (getline(assoc_file, atstr) && i < 33)
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
            if (i % 3 == 0)
            {
                assoc_arr_cacheloads[i / 3] = stoi(atstr.substr(0, found));
                size_t end = atstr.find("%"), start = end - 1;
                while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
                start++;
                assoc_arr_cacheloads_stddev[i / 3] = stod(atstr.substr(start, end - start));
            }
            else if (i % 3 == 1)
            {
                assoc_arr_cacheloadmisses[i / 3] = stoi(atstr.substr(0, found));
                size_t end = atstr.find("%"), start = end - 1;
                while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
                start++;
                assoc_arr_cacheloadmisses_stddev[i / 3] = stod(atstr.substr(start, end - start));
            }
            else if (i % 3 == 2)
            {
                assoc_arr_ns[i / 3] = stoi(atstr.substr(0, found));
                size_t end = atstr.find("%"), start = end - 1;
                while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
                start++;
                assoc_arr_ns_stddev[i / 3] = stod(atstr.substr(start, end - start));
            }
            i++;
        }
        double max_cachemiss_stddev = 0;
        int max_cachemiss_stddev_index = -1;
        for (int i = 0; i < 11; i++)
        {
            if (assoc_arr_cacheloadmisses_stddev[i] > max_cachemiss_stddev)
            {
                max_cachemiss_stddev = assoc_arr_cacheloadmisses_stddev[i];
                max_cachemiss_stddev_index = i;
            }
        }
        /*  
            the biggest cache miss standard deviation
            is found on a quarter of the critical stride
        */
        return pow(2, (max_cachemiss_stddev_index + 2) + 6);
    }
};

class L1i_cache : public L1_cache
{
    public:
    L1i_cache(int size, int linesize, int assoc)
        : L1_cache("L1i", size, linesize, assoc)
    {

    }
    int empirical_assoc_test()
    {
        /*  
            assesses instruction cache associativity empirically
        */
        std::cout << "Assessing instruction-cache associativity empirically" << std::flush;
        std::string cmd_compile_65536 = "g++ -g -falign-functions=65536 ./associativity_test_i.cpp -o ./associativity_test_i_65536 ";
        std::string cmd_compile_noalign = "g++ -g ./associativity_test_i.cpp -o ./associativity_test_i_noalign ";
        std::string cmd_no_arg_65536 = "perf stat -x , --append -o assoctmpi.txt -e L1-icache-load-misses -r 1000 ./associativity_test_i_65536 ";
        std::string cmd_no_arg_noalign = "perf stat -x , --append -o assoctmpi.txt -e L1-icache-load-misses -r 1000 ./associativity_test_i_noalign ";
        std::string cmd_rm_assoctmp = "rm ./assoctmpi.txt";
        int ret_compile = system(cmd_compile_65536.c_str());
        if (ret_compile)
        {
            std::cout << "First compilation failed!" << std::endl;
        }
        ret_compile = system(cmd_compile_noalign.c_str());
        if (ret_compile)
        {
            std::cout << "Second compilation failed!" << std::endl;
        }
        int ret_rm_assoctmp = system(cmd_rm_assoctmp.c_str());
        /*  
            varying the number of functions competing for the cache
        */
        for (int i = 2; i <= 16; i += 2)
        {
            std::cout << "." << std::flush;
            std::string cmd_full_65536 = cmd_no_arg_65536 + std::to_string(i);
            std::string cmd_full_noalign = cmd_no_arg_noalign + std::to_string(i);
            FILE* cmd_stream;
            std::string echo = "echo \"#" + std::to_string(i) + "\\n\" >> assoctmpi.txt";
            system(echo.c_str());
            cmd_stream = popen(cmd_full_65536.c_str(), "r");
            pclose(cmd_stream);
            cmd_stream = popen(cmd_full_noalign.c_str(), "r");
            pclose(cmd_stream);
        }
        std::cout << std::endl << std::flush;
        std::ifstream assoc_file;
        assoc_file.open("./assoctmpi.txt");
        std::string atstr;
        int assoc = -1;
        int assoc_arr_65536[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        int assoc_arr_noalign[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        double assoc_arr_65536_stddev[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        double assoc_arr_noalign_stddev[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        int i = 0;
        while (getline(assoc_file, atstr) && i < 16)
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
            size_t end = atstr.find("%"), start = end - 1;
            if (i % 2 == 0)
            {
                assoc_arr_65536[i / 2] = stoi(atstr.substr(0, found));
                while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
                start++;
                assoc_arr_65536_stddev[i / 2] = stod(atstr.substr(start, end - start));
            }
            else
            {
                assoc_arr_noalign[i / 2] = stoi(atstr.substr(0, found));
                while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
                start++;
                assoc_arr_noalign_stddev[i / 2] = stod(atstr.substr(start, end - start));
            }
            i++;
        }
        double max_stddev = 0.0;
        int max_stddev_index = -1;
        int first_spike = 9;
        for (int i = 0; i < 8; i++)
        {
            if (assoc_arr_65536_stddev[i] > max_stddev)
            {
                max_stddev = assoc_arr_65536_stddev[i];
                max_stddev_index = i;
            }
            if (assoc_arr_65536[7 - i] > (assoc_arr_noalign[7 - i] * 2))
            {
                first_spike = 7 - i;
            }
        }
        /*  
            similarly to the data cache,
            the best estimate is the minimum
            of the biggest standard deviation
            and the first spike in number of cache misses
        */
        return (std::min(max_stddev_index, first_spike) + 1) * 2;
    }
    int empirical_stride_test()
    {
        /*  
            assesses instruction cache critical stride empirically
        */
        std::cout << "Assessing instruction-cache critical stride empirically" << std::flush;
        std::string cmd_compile_1 = "g++ -g -falign-functions=";
        std::string cmd_compile_2 = " ./critical_stride_test_i.cpp -o ./critical_stride_test_i_";
        std::string cmd_no_arg = "perf stat -x , --append -o cstmpi.txt -e L1-icache-load-misses -r 1000 ./critical_stride_test_i_";
        std::string cmd_rm_assoctmp = "rm ./cstmpi.txt";
        for (int i = 64; i <= 65536; i *= 2)
        {
            std::string cmd_compile_full = cmd_compile_1 + std::to_string(i) + cmd_compile_2 + std::to_string(i);
            int ret_compile = system(cmd_compile_full.c_str());
            if (ret_compile)
            {
                std::cout << "Compilation failed!" << std::endl;
            }
        }
        /*  
            varying the alignment to force the functions onto
        */
        int ret_rm_assoctmp = system(cmd_rm_assoctmp.c_str());
        for (int i = 64; i <= 65536; i *= 2)
        {
            std::cout << "." << std::flush;
            std::string cmd_full = cmd_no_arg + std::to_string(i);
            FILE* cmd_stream;
            cmd_stream = popen(cmd_full.c_str(), "r");
            pclose(cmd_stream);
        }
        std::cout << std::endl << std::flush;
        std::ifstream assoc_file;
        assoc_file.open("./cstmpi.txt");
        std::string atstr;
        int assoc = -1;
        int assoc_arr_cacheloadmisses[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        double assoc_arr_cacheloadmisses_stddev[11] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        int i = 0;
        size_t end = atstr.find("%"), start = end - 1;
        while (getline(assoc_file, atstr) && i < 11)
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
            assoc_arr_cacheloadmisses[i] = stoi(atstr.substr(0, found));
            size_t end = atstr.find("%"), start = end - 1;
            while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
            start++;
            assoc_arr_cacheloadmisses_stddev[i] = stod(atstr.substr(start, end - start));
            i++;
        }
        int last_jump = -1;
        for (int i = 0; i < 11; i++)
        {
            if (i >= 1 && (assoc_arr_cacheloadmisses[i] > (assoc_arr_cacheloadmisses[i - 1] * 2)))
            {
                last_jump = i;
            }
        }
        /*  
            the best estimate is the last big jump 
            in number of cache misses
        */
        return pow(2, last_jump + 6);
    }
};

class L2_cache : public Cache
{
    public:
    L2_cache(int size, int linesize, int assoc)
        : Cache("L2", size, linesize, assoc)
    {

    }
};

class L3_cache : public Cache
{
    public:
    L3_cache(int size, int linesize, int assoc)
        : Cache("L3", size, linesize, assoc)
    {

    }
};

class L4_cache : public Cache
{
    public:
    L4_cache(int size, int linesize, int assoc)
        : Cache("L4", size, linesize, assoc)
    {

    }
};

struct Processor
{
    L1d_cache* l1d = nullptr;
    L1i_cache* l1i = nullptr;
    L2_cache* l2 = nullptr;
    L3_cache* l3 = nullptr;
    L4_cache* l4 = nullptr;
    ~Processor()
    {
      if (l1d) { delete l1d; }
      if (l1i) { delete l1i; }
      if (l2) { delete l2; }
      if (l3) { delete l3; }
      if (l4) { delete l4; }
    };
};