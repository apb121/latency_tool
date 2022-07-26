#include <iostream>
#include <fstream>
#include <regex>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <list>
#include <forward_list>
#include <deque>
#include <vector>
#include <cmath>
#include <algorithm>

#include "class_parser.hpp"

size_t get_type_size(std::string type, std::string array_match, std::map<std::string, size_t> udtype_sizes)
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
    else if (type.find("bitset") != std::string::npos)
    {
      return sizeof(std::bitset<8>);
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
    else if (type.find("vector") != std::string::npos)
    {
      std::regex vector_bool_regex("vector *< *bool *>");
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
  else if (type.find("string") != std::string::npos)
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
  else
  {
    for (auto& i : udtype_sizes)
    {
      if (type.find(i.first) != std::string::npos)
      {
        size = i.second;
      }
    }
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
        size = get_type_size(type_match_str, array_match_str, udtype_sizes);
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

void UDType::detect_variables(std::map<std::string, size_t> udtype_sizes)
{
    std::string class_info_copy;
    size_t pos = 0;
    while (class_info[pos] != '{') { pos++; }
    pos++;
    int curly_bracket_depth = 1;
    int round_bracket_depth = 0;
    bool in_double = false, in_single = false;

    /* discard any code embedded in curly or round brackets. this cannot contain data members */

    while (pos < class_info.length() && curly_bracket_depth > 0)
    {
      if (class_info[pos] == '"' && class_info[pos - 1] != '\'' && !in_single)
      {
        in_double = !in_double;
      }
      if (class_info[pos] == '\'' && class_info[pos - 1] != '\'' && !in_double)
      {
        in_single = !in_single;
      }
      if (class_info[pos] == '{' && !in_double && !in_single)
      {
        curly_bracket_depth++;
      }
      if (class_info[pos] == ')' && !in_double && !in_single)
      {
        round_bracket_depth--;
      }
      if (curly_bracket_depth == 1 && round_bracket_depth == 0)
      {
        class_info_copy += class_info[pos];
      }
      if (class_info[pos] == '}' && !in_double && !in_single)
      {
        curly_bracket_depth--;
      }
      if (class_info[pos] == '(' && !in_double && !in_single)
      {
        round_bracket_depth++;
      }
      pos++;
    }

    /*  
        find variable declarations.
        note: the virtual keyword is present
        because it is easier to catch here and discard later
        than to exclude in the regex
    */

    /* new types should go in the middle */

    std::string variable_regex_start = "(?:(?:virtual *|";
    std::string variable_regex_end = "auto *|static *|const *|unsigned *|signed *|register *|volatile *|void *\\* *|bitset *<.*> *|array *<.*> *|std::vector *<.*> *|deque *<.*> *|forward_list *<.*> *|list *<.*> *|stack *<.*> *|queue *<.*> *|priority_queue *<.*> *|set *<.*> *|multiset *<.*> *|map *<.*> *|multimap *<.*> *|unordered_set *<.*> *|unordered_multiset *<.*> *|unordered_map *<.*> *|unordered_multimap *<.*> *|size_t *|std::string *|short *|long *|char *|wchar_t *|char8_t *|char16_t *|int *|float *|double *| bool *|complex *)+)[\\*]*(?: +\\*?\\*? *)( *const *)?([a-zA-Z_][a-zA-Z0-9_]*) *(([{;,=])|(((\\[ *[0-9]* *\\])+)))";

    for (auto& i : udtype_sizes)
    {
      variable_regex_start += (i.first + " *|");
    }

    std::regex variable_declaration(variable_regex_start + variable_regex_end);
    std::smatch variable_match;
    while (regex_search(class_info_copy, variable_match, variable_declaration))
    {
        std::string declaration = variable_match.str();
        int end = declaration.length() - 1;

        /*  
            identify square bracket array length indicators.
            note: does not yet work with var[] = {..., ..., ..., etc.}
        */

        std::regex array_length("(\\[ *[0-9]* *\\] *)+");
        std::smatch array_match;
        std::string array_match_str = "";
        std::string declaration_remaining = declaration;
        while (regex_search(declaration_remaining, array_match, array_length))
        {
            /*
                check if square brackets are inside template
                e.g., vector<char[5]> vs vector<char>[5]
            */
            if (array_match.suffix().str().find(">") == std::string::npos)
            {
                array_match_str = array_match.str();
                end = end - array_match.str().length();
            }
            declaration_remaining = array_match.suffix();
        }
        /*  
            extract the variable name
        */
        while (!isalnum(declaration[end]) && declaration[end] != '_') { end--; }
        int name_begin = end;
        while (isalnum(declaration[name_begin]) || (declaration[name_begin] == '_')) { name_begin--; }
        name_begin++;
        std::string variable_name = declaration.substr(name_begin, (end - name_begin) + 1);
        /*  
            extract the variable type
        */
        int start = 0;
        while (isspace(declaration[start])) { start++; }
        std::string variable_type = declaration.substr(start, name_begin - start);
        /*  
            check for any arrays inside template parameters
        */
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
            /*  
                find the type inside the template parameter
            */
            int type_begin = 1;
            while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
            int type_end = array_regex_recurse_match_str.length() - (array_match_str.length() + 1);
            while (isspace(array_regex_recurse_match_str[type_begin])) { type_begin++; }
            std::string type_match_str = array_regex_recurse_match_str.substr(type_begin, (type_end - type_begin));
            variable_type_alignment = type_match_str;
        }
        /*  
            static data members are not relevant to size.
            dealing with auto seems intractable right now
        */
        if (variable_type.find("static") == std::string::npos)
        {
            if (variable_type.find("auto") == std::string::npos)
            {
                size_t variable_size = get_type_size(variable_type, array_match_str, udtype_sizes);
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

size_t UDType::calculate_size(std::map<std::string, size_t> udtype_sizes)
{
    types_list.clear();

    detect_variables(udtype_sizes);

    size_t curr_align = 0;

    for (int i = 0; i < (int) types_list.size() - 1; ++i)
    {
        curr_align += std::get<1>(types_list[i]);
        std::string align_by = std::get<3>(types_list[i + 1]);
        int align_target = ((get_type_size(align_by, "", udtype_sizes) <= 8) ? get_type_size(align_by, "", udtype_sizes) : 8);
        if (align_target == 0)
        {
            return 0;
        }
        while (curr_align % align_target != 0) { curr_align++; }
    }
    if (types_list.size() > 0)
    {
      curr_align += std::get<1>(types_list[types_list.size() - 1]);
    }
    while (curr_align % 8 != 0) { curr_align++; }
    int size = curr_align;
    if (has_virtual)
    {
      if (parent_class)
      {
        if (!parent_class->has_virtual)
        {
          size += 8;
        }
      }
      else
      {
        size += 8;
      }
    }
    if (is_child)
    {
      int parent_size = this->get_parent()->calculate_size(udtype_sizes);
      if (parent_size == 0)
      {
        size = 0;
      }
      else
      {
        size += parent_size;
      }
    }
    return (total_size = size);
}

size_t UDType::calculate_size(std::vector<variable_info> proposed_types_list, std::map<std::string, size_t> udtype_sizes)
{
    size_t curr_align = 0;

    for (int i = 0; i < (int) proposed_types_list.size() - 1; ++i)
    {
        curr_align += std::get<1>(proposed_types_list[i]);
        std::string align_by = std::get<3>(proposed_types_list[i + 1]);
        int align_target = ((get_type_size(align_by, "", udtype_sizes) <= 8) ? get_type_size(align_by, "", udtype_sizes) : 8);
        if (align_target == 0)
        {
            return 0;
        }
        while (curr_align % align_target != 0) { curr_align++; }
    }
    if (proposed_types_list.size() > 0)
    {
      curr_align += std::get<1>(proposed_types_list[proposed_types_list.size() - 1]);
    }
    while (curr_align % 8 != 0) { curr_align++; }
    int size = curr_align;
    if (has_virtual)
    {
      if (parent_class)
      {
        if (!parent_class->has_virtual)
        {
          size += 8;
        }
      }
      else
      {
        size += 8;
      }
    }
    if (is_child)
    {
      int parent_size = this->get_parent()->calculate_size(udtype_sizes);
      if (parent_size == 0)
      {
        size = 0;
      }
      else
      {
        size += parent_size;
      }
    }
    return size;
}

bool UDType::suggest_optimisations(std::map<std::string, size_t> udtype_sizes, int critical_stride)
{
    bool optimised = false;
    int current_size = calculate_size(udtype_sizes);
    if (current_size == 0)
    {
      std::cerr << name << " contains unknown an type, cannot suggest optimised orderings" << std::endl;
      return false;
    }
    else if (types_list.size() == 0)
    {
      return false;
    }
    std::vector<variable_info> new_ordering = types_list;
    std::vector<variable_info> proposed_types_list = types_list;
    int curr_size = calculate_size(udtype_sizes);
    int min_size = curr_size;
    do
    {
        int size = calculate_size(proposed_types_list, udtype_sizes);
        if (size < min_size)
        {
            min_size = size;
            new_ordering = proposed_types_list;
        }
    } while(std::next_permutation(proposed_types_list.begin(), proposed_types_list.end()));
    if (min_size < curr_size)
    {
        optimised = true;
        std::cout << std::endl << "=== An inefficient data member ordering has been detected ===" << std::endl << std::endl;
        std::cout << "Typename: " << name << std::endl << std::endl;
        std::cout << "Current ordering:" << std::endl << std::endl;
        for (size_t j = 0; j < types_list.size(); ++j)
        {
            std::cout << "type: " << std::get<2>(types_list[j]) << "; ";
            std::cout << "name: " << std::get<0>(types_list[j]) << "; ";
            std::cout << "size: " << std::get<1>(types_list[j]) << "; ";
            std::cout << std::endl;
        }

        std::cout << std::endl << "Current size: " << current_size << std::endl << std::endl;

        std::cout << "Proposed ordering:" << std::endl << std::endl;
        for (size_t j = 0; j < new_ordering.size(); ++j)
        {
            std::cout << "type: " << std::get<2>(new_ordering[j]) << "; ";
            std::cout << "name: " << std::get<0>(new_ordering[j]) << "; ";
            std::cout << "size: " << std::get<1>(new_ordering[j]) << "; ";
            std::cout << std::endl;
        }
        std::cout << std::endl << "New size: " << min_size << std::endl << std::endl;
        std::cout << "This ordering saves " << current_size - min_size << " bytes." << std::endl << std::endl;
    }
    int lcm = std::lcm(curr_size, critical_stride);
    if ((lcm / curr_size) <= 16)
    {
      optimised = true;
      std::cout << std::endl << "=== An inefficient class size has been detected ===" << std::endl << std::endl;
      std::cout << "Typename: " << name << std::endl << std::endl;
      std::cout << "This type has a total size of " << curr_size << " bytes, whose Least Common Multiple with your processor's L1 data-cache critical stride of " << critical_stride << " bytes is " << lcm << "." << std::endl;
      std::cout << "This means that data members " << lcm / curr_size << " objects apart in contiguous memory will compete with each other for cache space." << std::endl << std::endl;
    }
    return optimised;
}

/* ============================= FILE ============================= */

std::vector<UDType> File::detect_types()
{
    std::ifstream class_file;
    class_file.open(file_name);
    char c;
    std::string full_file;
    c = class_file.get();
    while (!class_file.eof())
    {
      full_file += c;
      c = class_file.get();
    }
    std::vector<UDType> type_info;
    /*  
        detect classes and structs.
        this regex detects the *start* of a class/struct
        because regexes (as finite-state-machine equivalent formal languages)
        are not able to count bracket recursions,
        the end of the class is found separately below
    */
    std::regex class_regex("(class|struct)\\s*([a-zA-Z_][a-zA-Z0-9_]*)\\s*(:\\s*(public|protected|private)\\s*([a-zA-Z_][a-zA-Z0-9_]*)\\s*)?\\{");
    std::smatch class_match;
    std::string file_remaining = full_file;
    std::string class_info;
    while (regex_search(file_remaining, class_match, class_regex))
    {
      bool is_child = false;
      std::string parent_name = "";
      class_info = class_match.str();
      std::regex parent_regex("(\\s*(public|protected|private)\\s*([a-zA-Z_][a-zA-Z0-9_]*)\\s*)");
      std::smatch parent_match;
      std::string parent_str;
      regex_search(class_info, parent_match, parent_regex);
      parent_str = parent_match.str();
      if (parent_str.length() > 0)
      { 
        is_child = true;
        int ppp_start = 0;
        while (isspace(parent_str[ppp_start])) { ++ppp_start; }
        while (!isspace(parent_str[ppp_start])) { ++ppp_start; }
        while (isspace(parent_str[ppp_start])) { ++ppp_start; }
        size_t ppp_end = ppp_start;
        while (!isspace(parent_str[ppp_end])) { ++ppp_end; }
        parent_name = parent_str.substr(ppp_start, ppp_end - ppp_start);
      }
      file_remaining = class_match.suffix();
      size_t pos = 0;
      int curly_bracket_depth = 1;
      bool in_double = false;
      bool in_single = false;
      while (pos < file_remaining.length() && curly_bracket_depth > 0)
      {
        if (file_remaining[pos] == '"' && file_remaining[pos - 1] != '\'' && !in_single)
        {
          in_double = !in_double;
        }
        if (file_remaining[pos] == '\'' && file_remaining[pos - 1] != '\'' && !in_double)
        {
          in_single = !in_single;
        }
        if (file_remaining[pos] == '{' && !in_double && !in_single)
        {
          curly_bracket_depth++;
        }
        if (file_remaining[pos] == '}' && !in_double && !in_single)
        {
          curly_bracket_depth--;
        }
        class_info += file_remaining[pos];
        pos++;
      }
      while (pos < file_remaining.length() && file_remaining[pos] != ';')
      {
        class_info += file_remaining[pos];
        pos++;
      }
      if (file_remaining[pos] == ';')
      {
        /*  
            here a class/struct has been properly detected
        */
        class_info += file_remaining[pos];
        pos++;
        int name_start = 0, name_end = 0;
        while (isspace(class_info[name_start])) { name_start++; }
        name_end = name_start;
        while (!isspace(class_info[name_end])) { name_end++; }
        name_start = name_end;
        while (isspace(class_info[name_start])) { name_start++; }
        name_end = name_start;
        while (!isspace(class_info[name_end]) && class_info[name_end] != '{' && class_info[name_end] != ':') { name_end++; }
        UDType new_ud_type(class_info.substr(name_start, name_end - name_start), class_info);
        size_t virtual_loc = class_info.find("virtual");

        /* virtual functions require a field of size 8 bytes to point to the class's vtable */
        
        if (virtual_loc != std::string::npos && isspace(class_info[virtual_loc - 1]) && isspace(class_info[virtual_loc + 7]))
        {
          new_ud_type.has_virtual = true;
        }
        if (is_child)
        {
          new_ud_type.is_child = true;
          new_ud_type.parent_name = parent_name;
        }
        file_remaining = file_remaining.substr(pos);
        user_defined_types.push_back(new_ud_type);
      }
      else
      {
        break;
      }
    }
    return user_defined_types;
}