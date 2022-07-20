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

#include "cache.hpp"

using namespace std;

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

  //detect_types();

  //cout << "(compiler) Cache: " << sizeof(Cache) << endl;

  //cout << "(compiler) string_struct: " << sizeof(string_struct) << endl;

  //cout << "(compiler) UDType: " << sizeof(UDType) << endl;
  return 0;
}
