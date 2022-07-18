#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <map>
#include <unistd.h>
#include <fstream>
#include <cmath>
#include <regex>

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

struct UDType
{
  string name;
  using variable_info = tuple<string, size_t>;
  vector<variable_info> types;
};

size_t get_type_size(string type)
{
  size_t size = 0;
  if (type.find("*") != string::npos)
  {
    return sizeof(void*);
  }
  else if (type.find("char16") != string::npos)
  {
    return sizeof(char16_t);
  }
  else if (type.find("char32") != string::npos)
  {
    return sizeof(char32_t);
  }
  else if (type.find("64") != string::npos)
  {
    return 8;
  }
  else if (type.find("32") != string::npos)
  {
    return 4;
  }
  else if (type.find("16") != string::npos)
  {
    return 2;
  }
  else if (type.find("8") != string::npos)
  {
    return 1;
  }
  else if (type.find("bool") != string::npos)
  {
    return sizeof(bool);
  }
  else if (type.find("long long") != string::npos)
  {
    return sizeof(long long);
  }
  else if (type.find("long double") != string::npos)
  {
    return sizeof(long double);
  }
  else if (type.find("double") != string::npos)
  {
    return sizeof(double);
  }
  else if (type.find("float") != string::npos)
  {
    return sizeof(float);
  }
  else if (type.find("long") != string::npos)
  {
    return sizeof(long);
  }
  else if (type.find("short") != string::npos)
  {
    return sizeof(short);
  }
  else if (type.find("int") != string::npos)
  {
    return sizeof(int);
  }
  else if (type.find("wchar") != string::npos)
  {
    return sizeof(wchar_t);
  }
  else if (type.find("char") != string::npos)
  {
    return sizeof(char);
  }
}

int associativity_test_i()
{
  char buf[65536];
  string cmd_compile = "g++ -g -falign-functions=65536 ./associativity_test_i.cpp -o ./associativity_test_i";
  string cmd_no_arg = "perf stat -x , --append -o assoctmpi.txt -e L1-icache-load-misses -r 1000 ./associativity_test_i ";
  string cmd_rm_assoctmp = "rm ./assoctmpi.txt";
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
    cout << "running i=" << i << endl;
    cmd_stream = popen(cmd_full.c_str(), "r");
    pclose(cmd_stream);
  }
  ifstream assoc_file;
  assoc_file.open("./assoctmpi.txt");
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
      //int ret_rm_assoctmp = system(cmd_rm_assoctmp.c_str());
      return (i * 2);
    }
    i++;
  }
}

int associativity_test_d()
{
  char buf[65536];
  string cmd_compile = "g++ -g ./associativity_test_d.cpp -o ./associativity_test_d";
  string cmd_no_arg = "perf stat -x , --append -o assoctmpd.txt -e L1-dcache-load-misses -r 1000 ./associativity_test_d ";
  string cmd_rm_assoctmp = "rm ./assoctmpd.txt";
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
    cout << "running i=" << i << endl;
    cmd_stream = popen(cmd_full.c_str(), "r");
    pclose(cmd_stream);
  }
  ifstream assoc_file;
  assoc_file.open("./assoctmpd.txt");
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
      //int ret_rm_assoctmp = system(cmd_rm_assoctmp.c_str());
      return (i * 2);
    }
    i++;
  }
}

int critical_stride_test_d()
{
  char buf[65536];
  string cmd_compile = "g++ -g ./critical_stride_test_d.cpp -o ./critical_stride_test_d";
  string cmd_no_arg = "perf stat -x , --append -o cstmpd.txt -e L1-dcache-loads,L1-dcache-load-misses,duration_time -r 1000 ./critical_stride_test_d ";
  string cmd_rm_assoctmp = "rm ./cstmpd.txt";
  int ret_compile = system(cmd_compile.c_str());
  if (ret_compile)
  {
    cout << "Compilation failed!" << endl;
  }
  int ret_rm_assoctmp = system(cmd_rm_assoctmp.c_str());
  for (int i = 64; i <= 65536; i *= 2)
  {
    string cmd_full = cmd_no_arg + to_string(i);
    FILE* cmd_stream;
    cout << "running i=" << i << endl;
    cmd_stream = popen(cmd_full.c_str(), "r");
    pclose(cmd_stream);
  }
  ifstream assoc_file;
  assoc_file.open("./cstmpd.txt");
  string atstr;
  int assoc = -1;
  int assoc_arr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int assoc_diff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
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
    int diff_max = 0;
    int diff_max_index = -1;
    if (i > 1 && i <= 7)
    {
      assoc_diff[i - 1] = (assoc_diff[i - 1] - assoc_diff[i - 2]) - (assoc_diff[i] - assoc_diff[i - 1]);
      if ((assoc_diff[i - 1]) > diff_max)
      {
        diff_max = assoc_diff[i - 1];
        diff_max_index = i - 1;
      }
    }
    i++;
  }
  return pow(2, (i - 1) + 5);
}

void detect_types()
{
  ifstream class_file;
  class_file.open("./class_test.cpp");
  char buf[512];
  vector<UDType> type_info;
  while (class_file >> buf)
  {
    if (strcmp(buf, "struct") == 0 || strcmp(buf, "class") == 0)
    {
      class_file >> buf;
      string type_name(buf);
      cout << "type name is: " << type_name << endl;
      string class_info;
      char c;
      class_file.get(c);
      while (c != '{') { class_file.get(c); }
      class_info += c;
      int bracket_depth = 1;
      while (bracket_depth > 0)
      {
        class_file.get(c);
        class_info += c;
        if (c == '{') { bracket_depth++; }
        else if (c == '}') { bracket_depth--; }
      }
      cout << "class info:" << endl;
      cout << class_info << endl;
      //regex variable_declaration("\b(?:(?:auto *|const *|unsigned *|signed *|register *|volatile *|static *|void *|short *|long *|char *|int *|float *|double *|_Bool *|complex *)+)(?: +\*?\*? *)([a-zA-Z_][a-zA-Z0-9_]*) *[\[;,=)]");
      regex variable_declaration("(?:(?:const *|unsigned *|signed *|register *|volatile *|void *|short *|long *|char *|int *|float *|double *| bool *|complex *)+)[\*]*(?: +\*?\*? *)( *const *)?([a-zA-Z_][a-zA-Z0-9_]*) *[\[;,={]");
      smatch sm;
      UDType new_ud_type;
      while (regex_search(class_info, sm, variable_declaration))
      {
        string declaration = sm.str();
        int end = declaration.length() - 1;
        while (!isalnum(declaration[end]) && declaration[end] != '_') { end--; }
        int name_begin = end;
        while (isalnum(declaration[name_begin]) || (declaration[name_begin] == '_')) { name_begin--; }
        name_begin++;
        string variable_name = declaration.substr(name_begin, (end - name_begin) + 1);
        int start = 0;
        while (isspace(declaration[start])) { start++; }
        string variable_type = declaration.substr(start, name_begin - start);
        size_t variable_size = get_type_size(variable_type);
        new_ud_type.name = type_name;
        new_ud_type.types.push_back({variable_name, variable_size});
        class_info = sm.suffix();
      }
      for (int i = 0; i < new_ud_type.types.size(); ++i)
      {
        cout << "variable size: " << get<1>(new_ud_type.types[i]) << endl;
        cout << "variable name: " << get<0>(new_ud_type.types[i]) << endl;
        cout << endl;
      }
    }
  }
}

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

  //cout << "Suggested instruction-cache associativity: " << associativity_test_i() << endl;

  //cout << "Suggested data-cache associativity: " << associativity_test_d() << endl;

  //int critical_stride = critical_stride_test_d();
  //cout << "Suggested data-cache critical stide: " << critical_stride << endl;

  detect_types();

  return 0;
}
