#include <iostream>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <map>
#include <unistd.h>
#include <fstream>
#include <cmath>
#include <regex>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <queue>
#include <deque>
#include <list>
#include <forward_list>
#include <string>
#include <algorithm>

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
  int i = 4;
};

int associativity_test_i()
{
  char buf[65536];
  string cmd_compile_65536 = "g++ -g -falign-functions=65536 ./associativity_test_i.cpp -o ./associativity_test_i_65536 ";
  string cmd_compile_noalign = "g++ -g ./associativity_test_i.cpp -o ./associativity_test_i_noalign ";
  string cmd_no_arg_65536 = "perf stat -x , --append -o assoctmpi.txt -e L1-icache-load-misses -r 1000 ./associativity_test_i_65536 ";
  string cmd_no_arg_noalign = "perf stat -x , --append -o assoctmpi.txt -e L1-icache-load-misses -r 1000 ./associativity_test_i_noalign ";
  string cmd_rm_assoctmp = "rm ./assoctmpi.txt";
  int ret_compile = system(cmd_compile_65536.c_str());
  if (ret_compile)
  {
    cout << "First compilation failed!" << endl;
  }
  ret_compile = system(cmd_compile_noalign.c_str());
  if (ret_compile)
  {
    cout << "Second compilation failed!" << endl;
  }
  int ret_rm_assoctmp = system(cmd_rm_assoctmp.c_str());
  
  for (int i = 2; i <= 16; i += 2)
  {
    string cmd_full_65536 = cmd_no_arg_65536 + to_string(i);
    string cmd_full_noalign = cmd_no_arg_noalign + to_string(i);
    FILE* cmd_stream;
    cout << "(65536) running i=" << i << endl;
    string echo = "echo \"#" + to_string(i) + "\\n\" >> assoctmpi.txt";
    system(echo.c_str());
    cmd_stream = popen(cmd_full_65536.c_str(), "r");
    pclose(cmd_stream);
    cout << "(noalign) running i=" << i << endl;
    cmd_stream = popen(cmd_full_noalign.c_str(), "r");
    pclose(cmd_stream);
  }
  ifstream assoc_file;
  assoc_file.open("./assoctmpi.txt");
  string atstr;
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
      cout << "stoi: " << stoi(atstr.substr(0, found)) << endl;
      assoc_arr_65536[i / 2] = stoi(atstr.substr(0, found));
      while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
      start++;
      cout << "stod: " << stod(atstr.substr(start, end - start)) << endl;
      assoc_arr_65536_stddev[i / 2] = stod(atstr.substr(start, end - start));
    }
    else
    {
      cout << "stoi: " << stoi(atstr.substr(0, found)) << endl;
      assoc_arr_noalign[i / 2] = stoi(atstr.substr(0, found));
      while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
      start++;
      cout << "stod: " << stod(atstr.substr(start, end - start)) << endl;
      assoc_arr_noalign_stddev[i / 2] = stod(atstr.substr(start, end - start));
    }
    i++;
  }
  /*
  int diff, total_diff, avg_diff, total;
  for (int i = 0; i < 8; i++)
  {
    diff = abs(assoc_arr_65536[i] - assoc_arr_noalign[i]);
    total_diff += diff;
  }
  avg_diff = total_diff / 8;
  */
  double max_stddev = 0.0;
  int max_stddev_index = -1;
  int first_spike = 9;
  for (int i = 0; i < 8; i++)
  {
    //cout << "assoc stddev [i] = " << assoc_arr_65536_stddev[i] << endl;
    if (assoc_arr_65536_stddev[i] > max_stddev)
    {
      max_stddev = assoc_arr_65536_stddev[i];
      max_stddev_index = i;
    }
    if (assoc_arr_65536[7 - i] > (assoc_arr_noalign[7 - i] * 2))
    {
      first_spike = 7 - i;
    }
    /*
    if (assoc_arr_65536[i] > assoc_arr_noalign[i] * 2/* || assoc_arr_65536_stddev[i] > assoc_arr_noalign_stddev[i] * 2)
    {
      return (i + 1) * 2;
    }
    */
  }
  return (min(max_stddev_index, first_spike) + 1) * 2;
  //__builtin_unreachable(); //not good
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
    string cmd_full_65536 = cmd_no_arg + to_string(i) + " 65536";
    string cmd_full_16 = cmd_no_arg + to_string(i) + " 16";
    FILE* cmd_stream;
    string echo = "echo \"#" + to_string(i) + "\\n\" >> assoctmpd.txt";
    system(echo.c_str());
    cout << "(65536) running i=" << i << endl;
    cmd_stream = popen(cmd_full_65536.c_str(), "r");
    pclose(cmd_stream);
    cout << "(16) running i=" << i << endl;
    cmd_stream = popen(cmd_full_16.c_str(), "r");
    pclose(cmd_stream);
  }
  ifstream assoc_file;
  assoc_file.open("./assoctmpd.txt");
  string atstr;
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
      cout << "stoi: " << stoi(atstr.substr(0, found)) << endl;
      assoc_arr_65536[i / 2] = stoi(atstr.substr(0, found));
      while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
      start++;
      cout << "stod: " << stod(atstr.substr(start, end - start)) << endl;
      assoc_arr_65536_stddev[i / 2] = stod(atstr.substr(start, end - start));
    }
    else
    {
      cout << "stoi: " << stoi(atstr.substr(0, found)) << endl;
      assoc_arr_16[i / 2] = stoi(atstr.substr(0, found));
      while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
      start++;
      cout << "stod: " << stod(atstr.substr(start, end - start)) << endl;
      assoc_arr_16_stddev[i / 2] = stod(atstr.substr(start, end - start));
    }
    i++;
  }
  /*
  int diff, total_diff, avg_diff, total;
  for (int i = 0; i < 8; i++)
  {
    diff = abs(assoc_arr_65536[i] - assoc_arr_16[i]);
    total_diff += diff;
  }
  avg_diff = total_diff / 8;
  */
  double max_stddev = 0.0;
  int max_stddev_index = -1;
  int first_spike = 9;
  for (int i = 0; i < 8; i++)
  {
    if (assoc_arr_65536_stddev[i] > max_stddev)
    {
      //cout << "assoc stddev [i] = " << assoc_arr_65536_stddev[i] << endl;
      max_stddev = assoc_arr_65536_stddev[i];
      max_stddev_index = i;
    }
    if (assoc_arr_65536[7 - i] > (assoc_arr_16[7 - i] * 2))
    {
      first_spike = 7 - i;
    }
    /*
    if (assoc_arr_65536[i] > assoc_arr_noalign[i] * 2/* || assoc_arr_65536_stddev[i] > assoc_arr_noalign_stddev[i] * 2)
    {
      return (i + 1) * 2;
    }
    */
  }
  return (min(max_stddev_index, first_spike) + 1) * 2;
  //__builtin_unreachable(); //not good
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
      cout << "=== " + to_string(pow(2, (i / 3) + 6)) + " ===" << endl; 
      cout << "stoi: " << stoi(atstr.substr(0, found)) << endl;
      assoc_arr_cacheloads[i / 3] = stoi(atstr.substr(0, found));
      size_t end = atstr.find("%"), start = end - 1;
      while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
      start++;
      cout << "stod: " << stod(atstr.substr(start, end - start)) << endl;
      assoc_arr_cacheloads_stddev[i / 3] = stod(atstr.substr(start, end - start));
    }
    else if (i % 3 == 1)
    {
      cout << "stoi: " << stoi(atstr.substr(0, found)) << endl;
      assoc_arr_cacheloadmisses[i / 3] = stoi(atstr.substr(0, found));
      size_t end = atstr.find("%"), start = end - 1;
      while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
      start++;
      cout << "stod: " << stod(atstr.substr(start, end - start)) << endl;
      assoc_arr_cacheloadmisses_stddev[i / 3] = stod(atstr.substr(start, end - start));
    }
    else if (i % 3 == 2)
    {
      cout << "stoi: " << stoi(atstr.substr(0, found)) << endl;
      assoc_arr_ns[i / 3] = stoi(atstr.substr(0, found));
      size_t end = atstr.find("%"), start = end - 1;
      while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
      start++;
      cout << "stod: " << stod(atstr.substr(start, end - start)) << endl;
      assoc_arr_ns_stddev[i / 3] = stod(atstr.substr(start, end - start));
    }
    /*
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
    */
    i++;
  }
  double max_cachemiss_stddev = 0;
  int max_cachemiss_stddev_index = -1;
  for (int i = 0; i < 11; i++)
  {
    cout << ": " << assoc_arr_cacheloadmisses_stddev[i] << endl;
    if (assoc_arr_cacheloadmisses_stddev[i] > max_cachemiss_stddev)
    {
      max_cachemiss_stddev = assoc_arr_cacheloadmisses_stddev[i];
      max_cachemiss_stddev_index = i;
    }
  }
  cout << max_cachemiss_stddev_index << endl;
  return pow(2, (max_cachemiss_stddev_index + 2) + 6);
}

int critical_stride_test_i()
{
  char buf[65536];
  string cmd_compile_1 = "g++ -g -falign-functions=";
  string cmd_compile_2 = " ./critical_stride_test_i.cpp -o ./critical_stride_test_i_";
  string cmd_no_arg = "perf stat -x , --append -o cstmpi.txt -e L1-icache-load-misses -r 1000 ./critical_stride_test_i_";
  string cmd_rm_assoctmp = "rm ./cstmpi.txt";
  for (int i = 64; i <= 65536; i *= 2)
  {
    string cmd_compile_full = cmd_compile_1 + to_string(i) + cmd_compile_2 + to_string(i);
    int ret_compile = system(cmd_compile_full.c_str());
    if (ret_compile)
    {
      cout << "Compilation failed!" << endl;
    }
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
  assoc_file.open("./cstmpi.txt");
  string atstr;
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
    cout << "=== " + to_string(pow(2, i + 6)) + " ===" << endl; 
    cout << "stoi: " << stoi(atstr.substr(0, found)) << endl;
    assoc_arr_cacheloadmisses[i] = stoi(atstr.substr(0, found));
    size_t end = atstr.find("%"), start = end - 1;
    while (isdigit(atstr[start]) || atstr[start] == '.') { start--; }
    start++;
    cout << "stod: " << stod(atstr.substr(start, end - start)) << endl;
    assoc_arr_cacheloadmisses_stddev[i] = stod(atstr.substr(start, end - start));
    /*
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
    */
    i++;
  }
  /*
  double max_cachemiss_stddev = 0;
  int max_cachemiss_stddev_index = -1;
  for (int i = 0; i < 11; i++)
  {
    cout << ": " << assoc_arr_cacheloadmisses_stddev[i] << endl;
    if (assoc_arr_cacheloadmisses_stddev[i] > max_cachemiss_stddev)
    {
      max_cachemiss_stddev = assoc_arr_cacheloadmisses_stddev[i];
      max_cachemiss_stddev_index = i;
    }
  }
  cout << max_cachemiss_stddev_index << endl;
  return pow(2, max_cachemiss_stddev_index + 6);
  */
  int last_jump = -1;
  for (int i = 0; i < 11; i++)
  {
    cout << ": " << assoc_arr_cacheloadmisses[i] << endl;
    if (i >= 1 && (assoc_arr_cacheloadmisses[i] > (assoc_arr_cacheloadmisses[i - 1] * 2)))
    {
      last_jump = i;
    }
  }
  cout << last_jump << endl;
  return pow(2, last_jump + 6);
}


struct string_struct
{
  string s = "hello";
};

size_t get_type_size(string type, string array_match)
{
  size_t size = 0;
  regex ptr_outside_template("< *.* *> *.*\\*");
  smatch ptr_outside_template_match;
  if (type.find("<") != string::npos)
  {
    if (regex_search(type, ptr_outside_template_match, ptr_outside_template))
    {
      size = sizeof(void*);
    }
    else if (type.find("unordered") != string::npos)
    {
      size = sizeof(unordered_set<void*>);
    }
    else if (type.find("set") != string::npos)
    {
      size = sizeof(set<void*>);
    }
    else if (type.find("map") != string::npos)
    {
      size = sizeof(map<void*, void*>);
    }
    else if (type.find("priority_queue") != string::npos)
    {
      size = sizeof(priority_queue<void*>);
    }
    else if (type.find("queue") != string::npos)
    {
      size = sizeof(queue<void*>);
    }
    else if (type.find("stack") != string::npos)
    {
      size = sizeof(stack<void*>);
    }
    else if (type.find("forward_list") != string::npos)
    {
      size = sizeof(forward_list<void*>);
    }
    else if (type.find("list") != string::npos)
    {
      size = sizeof(list<void*>);
    }
    else if (type.find("deque") != string::npos)
    {
      size = sizeof(deque<void*>);
    }
    else if (type.find("vector") != string::npos)
    {
      regex vector_bool_regex("vector *< *bool *>");
      smatch vector_bool_match;
      if (regex_search(type, vector_bool_match, vector_bool_regex))
      {
        size = sizeof(vector<bool>);
      }
      else
      {
        size = sizeof(vector<void*>);
      }
    }
  }
  else if (type.find("string") != string::npos)
  {
    size = sizeof(string);
  }
  else if (type.find("*") != string::npos)
  {
    size = sizeof(void*);
  }
  else if (type.find("size_t") != string::npos)
  {
    size = sizeof(size_t);
  }
  else if (type.find("char16") != string::npos)
  {
    size = sizeof(char16_t);
  }
  else if (type.find("char32") != string::npos)
  {
    size = sizeof(char32_t);
  }
  else if (type.find("64") != string::npos)
  {
    size = 8;
  }
  else if (type.find("32") != string::npos)
  {
    size = 4;
  }
  else if (type.find("16") != string::npos)
  {
    size = 2;
  }
  else if (type.find("8") != string::npos)
  {
    size = 1;
  }
  else if (type.find("bool") != string::npos)
  {
    size = sizeof(bool);
  }
  else if (type.find("long long") != string::npos)
  {
    size = sizeof(long long);
  }
  else if (type.find("long double") != string::npos)
  {
    size = sizeof(long double);
  }
  else if (type.find("double") != string::npos)
  {
    size = sizeof(double);
  }
  else if (type.find("float") != string::npos)
  {
    size = sizeof(float);
  }
  else if (type.find("long") != string::npos)
  {
    size = sizeof(long);
  }
  else if (type.find("short") != string::npos)
  {
    size = sizeof(short);
  }
  else if (type.find("int") != string::npos)
  {
    size = sizeof(int);
  }
  else if (type.find("wchar") != string::npos)
  {
    size = sizeof(wchar_t);
  }
  else if (type.find("char") != string::npos)
  {
    size = sizeof(char);
  }
  if (type.find("array") != string::npos)
  {
    regex array_regex("array *< *.* *, *[0-9]+ *>");
    smatch array_regex_match;
    if (regex_search(type, array_regex_match, array_regex))
    {
      string array_len_str = array_regex_match.str();
      int end = array_len_str.length() - 1;
      end--;
      while (isspace(array_len_str[end])) { end--; }
      int begin = end;
      while (isdigit(array_len_str[begin])) { begin--; }
      begin++;
      regex array_regex_recurse("< *.* *,");
      smatch array_regex_recurse_match;
      if (regex_search(type, array_regex_recurse_match, array_regex_recurse))
      {
        string array_regex_recurse_match_str = array_regex_recurse_match.str();
        regex array_length("(\\[ *[0-9]* *\\] *)+");
        smatch array_match;
        string array_match_str;
        if (regex_search(array_regex_recurse_match_str, array_match, array_length))
        {
          array_match_str = array_match.str();
        }
        int type_begin = 1;
        while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
        int type_end = array_regex_recurse_match_str.length() - (array_match_str.length() + 1);
        while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
        string type_match_str = array_regex_recurse_match_str.substr(type_begin, (type_end - type_begin));
        size = get_type_size(type_match_str, array_match_str);
      }
      size *= stoi(array_len_str.substr(begin, end - begin + 1));
    }
  }
  if (array_match.length() > 0)
  {
    regex array_spec("\\[ *[0-9]* *\\]");
    smatch sub_array_match;
    while (regex_search(array_match, sub_array_match, array_spec))
    {
      string match = sub_array_match.str();
      int start = 0, end = match.length() - 1;
      while (isspace(match[start]) || match[start] == '[') { start++; }
      while (isspace(match[end]) || match[end] == ']') { end--; }
      size *= stoi(match.substr(start, (end - start) + 1));
      array_match = sub_array_match.suffix();
    }
  }
  return size;
}

struct UDType
{
  string name;
  // variable_name, variable_size, variable_type, alignment_type
  using variable_info = tuple<string, size_t, string, string>;
  vector<variable_info> types_list;
  bool has_auto = false;
  size_t total_size;
  size_t calculate_size()
  {
    size_t curr_align = 0;
    for (int i = 0; i < types_list.size() - 1; ++i)
    {
      curr_align += get<1>(types_list[i]);
      string align_by = get<3>(types_list[i + 1]);
      int align_target = ((get_type_size(align_by, "") <= 8) ? get_type_size(align_by, "") : 8);
      if (align_target == 0)
      {
        exit(1);
      }
      while (curr_align % align_target != 0) { curr_align++; }
    }
    curr_align += get<1>(types_list[types_list.size() - 1]);
    while (curr_align % 8 != 0) { curr_align++; }
    return (total_size = curr_align);
  }
  void optimise_ordering()
  {
    vector<variable_info> new_ordering = types_list;
    int min_size = calculate_size();
    do
    {
      int size = calculate_size();
      if (size < min_size)
      {
        min_size = size;
        new_ordering = types_list;
      }
      min_size = (size < min_size ? size : min_size);
    } while(next_permutation(types_list.begin(), types_list.end()));
    types_list = new_ordering;
  }
};

void detect_types()
{
  ifstream class_file;
  class_file.open("./class_test.cpp");
  char buf[512];
  vector<UDType> type_info;
  regex class_declaration("");
  smatch class_match;
  while (class_file >> buf)
  {
    if (strcmp(buf, "struct") == 0 || strcmp(buf, "class") == 0)
    {
      class_file >> buf;
      string type_name(buf);
      string class_info;
      char c;
      class_file.get(c);
      while (c != '{') { class_file.get(c); }
      class_info += c;
      int curly_bracket_depth = 1;
      int round_bracket_depth = 0;
      while (curly_bracket_depth > 0)
      {
        class_file.get(c);
        if (c == '{') { curly_bracket_depth++; }
        if (c == '(') { round_bracket_depth++; }
        if (curly_bracket_depth == 1 && round_bracket_depth == 0)
        {
          class_info += c;
        }
        if (c == '}') { curly_bracket_depth--; }
        if (c == ')') { round_bracket_depth--; }
      }
      // cout << "class info:" << endl;
      // cout << class_info << endl;
      //regex variable_declaration("\b(?:(?:auto *|const *|unsigned *|signed *|register *|volatile *|static *|void *|short *|long *|char *|int *|float *|double *|_Bool *|complex *)+)(?: +\*?\*? *)([a-zA-Z_][a-zA-Z0-9_]*) *[\[;,=)]");
      //regex variable_declaration("(?:(?:auto *|static *|const *|unsigned *|signed *|register *|volatile *|void *|array *<.*> *|vector *<.*> *|deque *<.*> *|forward_list *<.*> *|list *<.*> *|stack *<.*> *|queue *<.*> *|priority_queue *<.*> *|set *<.*> *|multiset *<.*> *|map *<.*> *|multimap *<.*> *|unordered_set *<.*> *|unordered_multiset *<.*> *|unordered_map *<.*> *|unordered_multimap *<.*> *|size_t *|string *|short *|long *|char *|wchar_t *|char8_t *|char16_t *|int *|float *|double *| bool *|complex *)+)[\\*]*(?: +\\*?\\*? *)( *const *)?([a-zA-Z_][a-zA-Z0-9_]*) *(([{;,=)])|(((\\[ *[0-9]* *\\])+)))"); //^( *\\(.*\\) *(const)? *\\{.*\\})
      regex variable_declaration("(?:(?:auto *|static *|const *|unsigned *|signed *|register *|volatile *|void *|array *<.*> *|vector *<.*> *|deque *<.*> *|forward_list *<.*> *|list *<.*> *|stack *<.*> *|queue *<.*> *|priority_queue *<.*> *|set *<.*> *|multiset *<.*> *|map *<.*> *|multimap *<.*> *|unordered_set *<.*> *|unordered_multiset *<.*> *|unordered_map *<.*> *|unordered_multimap *<.*> *|size_t *|string *|short *|long *|char *|wchar_t *|char8_t *|char16_t *|int *|float *|double *| bool *|complex *)+)[\\*]*(?: +\\*?\\*? *)( *const *)?([a-zA-Z_][a-zA-Z0-9_]*) *(([{;,=)])|(((\\[ *[0-9]* *\\])+)))");
      smatch variable_match;
      UDType new_ud_type;
      while (regex_search(class_info, variable_match, variable_declaration))
      {
        string declaration = variable_match.str();
        int end = declaration.length() - 1;
        regex array_length("(\\[ *[0-9]* *\\] *)+");
        smatch array_match;
        string array_match_str = "";
        string declaration_remaining = declaration;
        while (regex_search(declaration_remaining, array_match, array_length))
        {
          if (array_match.suffix().str().find(">") == string::npos)
          {
            array_match_str = array_match.str();
            end = end - array_match.str().length();
          }
          declaration_remaining = array_match.suffix();
        }
        while (!isalnum(declaration[end]) && declaration[end] != '_') { end--; }
        int name_begin = end;
        while (isalnum(declaration[name_begin]) || (declaration[name_begin] == '_')) { name_begin--; }
        name_begin++;
        string variable_name = declaration.substr(name_begin, (end - name_begin) + 1);
        int start = 0;
        while (isspace(declaration[start])) { start++; }
        string variable_type = declaration.substr(start, name_begin - start);
        regex array_regex_recurse("< *.* *,");
        smatch array_regex_recurse_match;
        string variable_type_alignment = variable_type;
        if (regex_search(variable_type_alignment, array_regex_recurse_match, array_regex_recurse))
        {
          string array_regex_recurse_match_str = array_regex_recurse_match.str();
          regex array_length("(\\[ *[0-9]* *\\] *)+");
          smatch array_match;
          string array_match_str;
          if (regex_search(array_regex_recurse_match_str, array_match, array_length))
          {
            array_match_str = array_match.str();
          }
          int type_begin = 1;
          while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
          int type_end = array_regex_recurse_match_str.length() - (array_match_str.length() + 1);
          while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
          string type_match_str = array_regex_recurse_match_str.substr(type_begin, (type_end - type_begin));
          variable_type_alignment = type_match_str;
        }
        if (variable_type.find("static") == string::npos)
        {
          if (variable_type.find("auto") == string::npos)
          {
            size_t variable_size = get_type_size(variable_type, array_match_str);
            new_ud_type.name = type_name;
            new_ud_type.types_list.push_back({variable_name, variable_size, variable_type, variable_type_alignment});
          }
          else
          {
            new_ud_type.has_auto = true;
            cout << "Sorry, this programme can't deal with the 'auto' keyword at the moment!" << endl;
          }
        }
        class_info = variable_match.suffix();
      }
      cout << " ================== " << endl << endl;
      cout << type_name << ": " << endl;
      cout << "Old ordering:" << endl;
      for (int i = 0; i < new_ud_type.types_list.size(); ++i)
      {
        cout << "type: " << get<2>(new_ud_type.types_list[i]) << "; ";
        cout << "size: " << get<1>(new_ud_type.types_list[i]) << "; ";
        cout << "name: " << get<0>(new_ud_type.types_list[i]) << "; ";
        cout << endl;
      }
      cout << "Total size: " << new_ud_type.calculate_size() << endl;
      cout << endl;
      cout << "Calculating optimal ordering..." << endl;
      cout << endl;
      
      new_ud_type.optimise_ordering();
      cout << "New ordering:" << endl;
      for (int i = 0; i < new_ud_type.types_list.size(); ++i)
      {
        cout << "type: " << get<2>(new_ud_type.types_list[i]) << "; ";
        cout << "size: " << get<1>(new_ud_type.types_list[i]) << "; ";
        cout << "name: " << get<0>(new_ud_type.types_list[i]) << "; ";
        cout << endl;
      }
      cout << "Total size: " << new_ud_type.calculate_size() << endl;
      cout << endl;
      
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

  cout << "Suggested data-cache associativity: " << associativity_test_d() << endl;

  cout << "Suggested instruction-cache associativity: " << associativity_test_i() << endl;

  cout << "Suggested data-cache critical stride: " << critical_stride_test_d() << endl;

  cout << "Suggested instruction-cache critical stride: " << critical_stride_test_i() << endl;

  //detect_types();

  //cout << "(compiler) Cache: " << sizeof(Cache) << endl;

  //cout << "(compiler) string_struct: " << sizeof(string_struct) << endl;

  //cout << "(compiler) UDType: " << sizeof(UDType) << endl;

  return 0;
}
