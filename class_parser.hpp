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
#include <tuple>
#include <fstream>

using variable_info = std::tuple<std::string, size_t, std::string, std::string>;

// variable_name, variable_size, variable_type, alignment_type
size_t get_type_size(std::string type, std::string array_match);

struct UDType
{
    std::string name;
    std::string class_info;
    std::vector<variable_info> types_list;
    bool has_auto = false;
    size_t total_size;
    public:
    UDType(std::string name, std::string class_info)
        : name(name), class_info(class_info)
    {

    }
    void detect_variables()
    {
        std::string class_info_copy = class_info;
        std::regex variable_declaration("(?:(?:auto *|static *|const *|unsigned *|signed *|register *|volatile *|void *|array *<.*> *|std::vector *<.*> *|deque *<.*> *|forward_list *<.*> *|list *<.*> *|stack *<.*> *|queue *<.*> *|priority_queue *<.*> *|set *<.*> *|multiset *<.*> *|map *<.*> *|multimap *<.*> *|unordered_set *<.*> *|unordered_multiset *<.*> *|unordered_map *<.*> *|unordered_multimap *<.*> *|size_t *|std::string *|short *|long *|char *|wchar_t *|char8_t *|char16_t *|int *|float *|double *| bool *|complex *)+)[\\*]*(?: +\\*?\\*? *)( *const *)?([a-zA-Z_][a-zA-Z0-9_]*) *(([{;,=)])|(((\\[ *[0-9]* *\\])+)))");
        std::smatch variable_match;
        while (regex_search(class_info_copy, variable_match, variable_declaration))
        {
            std::string declaration = variable_match.str();
            int end = declaration.length() - 1;
            std::regex array_length("(\\[ *[0-9]* *\\] *)+");
            std::smatch array_match;
            std::string array_match_str = "";
            std::string declaration_remaining = declaration;
            while (regex_search(declaration_remaining, array_match, array_length))
            {
                if (array_match.suffix().str().find(">") == std::string::npos)
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
            std::string variable_name = declaration.substr(name_begin, (end - name_begin) + 1);
            int start = 0;
            while (isspace(declaration[start])) { start++; }
            std::string variable_type = declaration.substr(start, name_begin - start);
            std::regex array_regex_recurse("< *.* *,");
            std::smatch array_regex_recurse_match;
            std::string variable_type_alignment = variable_type;
            if (regex_search(variable_type_alignment, array_regex_recurse_match, array_regex_recurse))
            {
                std::string array_regex_recurse_match_str = array_regex_recurse_match.str();
                std::regex array_length("(\\[ *[0-9]* *\\] *)+");
                std::smatch array_match;
                std::string array_match_str;
                if (regex_search(array_regex_recurse_match_str, array_match, array_length))
                {
                    array_match_str = array_match.str();
                }
                int type_begin = 1;
                while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
                int type_end = array_regex_recurse_match_str.length() - (array_match_str.length() + 1);
                while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
                std::string type_match_str = array_regex_recurse_match_str.substr(type_begin, (type_end - type_begin));
                variable_type_alignment = type_match_str;
            }
            if (variable_type.find("static") == std::string::npos)
            {
                if (variable_type.find("auto") == std::string::npos)
                {
                    size_t variable_size = get_type_size(variable_type, array_match_str);
                    types_list.push_back({variable_name, variable_size, variable_type, variable_type_alignment});
                }
                else
                {
                    has_auto = true;
                    std::cout << "Sorry, this programme can't deal with the 'auto' keyword at the moment!" << std::endl;
                }
            }
            class_info_copy = variable_match.suffix();
        }
    }
    size_t calculate_size()
    {
        size_t curr_align = 0;
        for (int i = 0; i < types_list.size() - 1; ++i)
        {
            curr_align += std::get<1>(types_list[i]);
            std::string align_by = std::get<3>(types_list[i + 1]);
            int align_target = ((get_type_size(align_by, "") <= 8) ? get_type_size(align_by, "") : 8);
            if (align_target == 0)
            {
                exit(1);
            }
            while (curr_align % align_target != 0) { curr_align++; }
        }
        curr_align += std::get<1>(types_list[types_list.size() - 1]);
        while (curr_align % 8 != 0) { curr_align++; }
        return (total_size = curr_align);
    }
    size_t calculate_size(std::vector<variable_info> proposed_types_list)
    {
        size_t curr_align = 0;
        for (int i = 0; i < proposed_types_list.size() - 1; ++i)
        {
            curr_align += std::get<1>(proposed_types_list[i]);
            std::string align_by = std::get<3>(proposed_types_list[i + 1]);
            int align_target = ((get_type_size(align_by, "") <= 8) ? get_type_size(align_by, "") : 8);
            if (align_target == 0)
            {
                exit(1);
            }
            while (curr_align % align_target != 0) { curr_align++; }
        }
        curr_align += std::get<1>(proposed_types_list[proposed_types_list.size() - 1]);
        while (curr_align % 8 != 0) { curr_align++; }
        return (total_size = curr_align);
    }
    std::vector<variable_info> suggest_optimised_ordering()
    {
        std::vector<variable_info> new_ordering = types_list;
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
        return new_ordering;
    }
};

class File
{
    std::string file_name;
    std::vector<UDType> user_defined_types;
    public:
    File(std::string file_name)
        : file_name(file_name)
    {

    }
    void detect_types()
    {
        std::ifstream class_file;
        class_file.open(file_name);
        char buf[512];
        std::vector<UDType> type_info;
        std::regex class_declaration("");
        std::smatch class_match;
        while (class_file >> buf)
        {
            if (strcmp(buf, "struct") == 0 || strcmp(buf, "class") == 0)
            {
                class_file >> buf;
                std::string type_name(buf);
                std::string class_info;
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
                UDType new_ud_type(type_name, class_info);
                new_ud_type.detect_variables();
                user_defined_types.push_back(new_ud_type);
            }
        }
    }
    void suggest_optimised_orderings()
    {
        for (int i = 0; i < user_defined_types.size(); ++i)
        {
            std::cout << " ================== " << std::endl << std::endl;
            std::cout << user_defined_types[i].name << ": " << std::endl;
            std::cout << "Current ordering:" << std::endl;
            for (int j = 0; j < user_defined_types[i].types_list.size(); ++j)
            {
                std::cout << "type: " << std::get<2>(user_defined_types[i].types_list[j]) << "; ";
                std::cout << "name: " << std::get<0>(user_defined_types[i].types_list[j]) << "; ";
                std::cout << "size: " << std::get<1>(user_defined_types[i].types_list[j]) << "; ";
                std::cout << std::endl;
            }

            int current_size = user_defined_types[i].calculate_size();
            std::cout << "Total size: " << current_size << std::endl;
            std::cout << std::endl;
            std::cout << "Calculating optimal ordering..." << std::endl;
            std::cout << std::endl;
            
            std::vector<variable_info> proposed_types_list = user_defined_types[i].suggest_optimised_ordering();
            int optimised_size = user_defined_types[i].calculate_size(proposed_types_list);

            if (optimised_size == current_size)
            {
                std::cout << "This type is already optimally ordered for size!" << std::endl;
                return;
            }

            std::cout << "Proposed ordering:" << std::endl;
            for (int j = 0; j < proposed_types_list.size(); ++j)
            {
                std::cout << "type: " << std::get<2>(proposed_types_list[j]) << "; ";
                std::cout << "name: " << std::get<0>(proposed_types_list[j]) << "; ";
                std::cout << "size: " << std::get<1>(proposed_types_list[j]) << "; ";
                std::cout << std::endl;
            }
            std::cout << "Total size: " << optimised_size << std::endl;
            std::cout << "This ordering saves " << current_size - optimised_size << " bytes." << std::endl;
            std::cout << std::endl;
        }
    }
};

size_t get_type_size(std::string type, std::string array_match)
{
  size_t size = 0;
  if (type.find("<") != std::string::npos)
  {
    std::regex ptr_outside_template("< *.* *> *.*\\*");
    std::smatch ptr_outside_template_match;
    if (regex_search(type, ptr_outside_template_match, ptr_outside_template))
    {
      size = sizeof(void*);
    }
    else if (type.find("unordered") != std::string::npos)
    {
      size = sizeof(std::unordered_set<void*>);
    }
    else if (type.find("set") != std::string::npos)
    {
      size = sizeof(std::set<void*>);
    }
    else if (type.find("map") != std::string::npos)
    {
      size = sizeof(std::map<void*, void*>);
    }
    else if (type.find("priority_queue") != std::string::npos)
    {
      size = sizeof(std::priority_queue<void*>);
    }
    else if (type.find("queue") != std::string::npos)
    {
      size = sizeof(std::queue<void*>);
    }
    else if (type.find("stack") != std::string::npos)
    {
      size = sizeof(std::stack<void*>);
    }
    else if (type.find("forward_list") != std::string::npos)
    {
      size = sizeof(std::forward_list<void*>);
    }
    else if (type.find("list") != std::string::npos)
    {
      size = sizeof(std::list<void*>);
    }
    else if (type.find("deque") != std::string::npos)
    {
      size = sizeof(std::deque<void*>);
    }
    else if (type.find("std::vector") != std::string::npos)
    {
      std::regex vector_bool_regex("std::vector *< *bool *>");
      std::smatch vector_bool_match;
      if (regex_search(type, vector_bool_match, vector_bool_regex))
      {
        size = sizeof(std::vector<bool>);
      }
      else
      {
        size = sizeof(std::vector<void*>);
      }
    }
  }
  else if (type.find("std::string") != std::string::npos)
  {
    size = sizeof(std::string);
  }
  else if (type.find("*") != std::string::npos)
  {
    size = sizeof(void*);
  }
  else if (type.find("size_t") != std::string::npos)
  {
    size = sizeof(size_t);
  }
  else if (type.find("char16") != std::string::npos)
  {
    size = sizeof(char16_t);
  }
  else if (type.find("char32") != std::string::npos)
  {
    size = sizeof(char32_t);
  }
  else if (type.find("64") != std::string::npos)
  {
    size = 8;
  }
  else if (type.find("32") != std::string::npos)
  {
    size = 4;
  }
  else if (type.find("16") != std::string::npos)
  {
    size = 2;
  }
  else if (type.find("8") != std::string::npos)
  {
    size = 1;
  }
  else if (type.find("bool") != std::string::npos)
  {
    size = sizeof(bool);
  }
  else if (type.find("long long") != std::string::npos)
  {
    size = sizeof(long long);
  }
  else if (type.find("long double") != std::string::npos)
  {
    size = sizeof(long double);
  }
  else if (type.find("double") != std::string::npos)
  {
    size = sizeof(double);
  }
  else if (type.find("float") != std::string::npos)
  {
    size = sizeof(float);
  }
  else if (type.find("long") != std::string::npos)
  {
    size = sizeof(long);
  }
  else if (type.find("short") != std::string::npos)
  {
    size = sizeof(short);
  }
  else if (type.find("int") != std::string::npos)
  {
    size = sizeof(int);
  }
  else if (type.find("wchar") != std::string::npos)
  {
    size = sizeof(wchar_t);
  }
  else if (type.find("char") != std::string::npos)
  {
    size = sizeof(char);
  }
  if (type.find("array") != std::string::npos)
  {
    std::regex array_regex("array *< *.* *, *[0-9]+ *>");
    std::smatch array_regex_match;
    if (regex_search(type, array_regex_match, array_regex))
    {
      std::string array_len_str = array_regex_match.str();
      int end = array_len_str.length() - 1;
      end--;
      while (isspace(array_len_str[end])) { end--; }
      int begin = end;
      while (isdigit(array_len_str[begin])) { begin--; }
      begin++;
      std::regex array_regex_recurse("< *.* *,");
      std::smatch array_regex_recurse_match;
      if (regex_search(type, array_regex_recurse_match, array_regex_recurse))
      {
        std::string array_regex_recurse_match_str = array_regex_recurse_match.str();
        std::regex array_length("(\\[ *[0-9]* *\\] *)+");
        std::smatch array_match;
        std::string array_match_str;
        if (regex_search(array_regex_recurse_match_str, array_match, array_length))
        {
          array_match_str = array_match.str();
        }
        int type_begin = 1;
        while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
        int type_end = array_regex_recurse_match_str.length() - (array_match_str.length() + 1);
        while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
        std::string type_match_str = array_regex_recurse_match_str.substr(type_begin, (type_end - type_begin));
        size = get_type_size(type_match_str, array_match_str);
      }
      size *= stoi(array_len_str.substr(begin, end - begin + 1));
    }
  }
  if (array_match.length() > 0)
  {
    std::regex array_spec("\\[ *[0-9]* *\\]");
    std::smatch sub_array_match;
    while (regex_search(array_match, sub_array_match, array_spec))
    {
      std::string match = sub_array_match.str();
      int start = 0, end = match.length() - 1;
      while (isspace(match[start]) || match[start] == '[') { start++; }
      while (isspace(match[end]) || match[end] == ']') { end--; }
      size *= stoi(match.substr(start, (end - start) + 1));
      array_match = sub_array_match.suffix();
    }
  }
  return size;
}