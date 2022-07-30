#include "class_parser.hpp"

//getalign
  // if it contains a <
    // if the first isntance of 'array' is before the first instance of <
      // move past the first instance of <
        // and then call the function on the structure again
    // otherwise do the normal thing just on the stuff up to the first <
      // will be something like vector or map or whatever
  // if no <
    // do the normal thing of finding char or int or whatever

int FileCollection::detect_types()
{
  std::vector<std::string> type_names;
  for (int i = 0; i < file_names.size(); ++i)
  {
    std::ifstream file_stream(file_names[i]);
    if (!file_stream.is_open())
    {
      std::cout << "Failed to open " << file_names[i] << "!" << std::endl;
      return 1;
    }
    char c;
    std::string full_file;
    c = file_stream.get();
    while (!file_stream.eof())
    {
      full_file += c;
      c = file_stream.get();
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
      class_info = class_match.str();
      int name_start = 0;
      while (isspace(class_info[name_start])) { name_start++; } // get to 'struct'/'class'
      while (!isspace(class_info[name_start])) { name_start++; } // get to end of 'struct'/'class'
      while (isspace(class_info[name_start])) { name_start++; } // get to start of class name
      int name_end = name_start;
      while (!isspace(class_info[name_end]) && class_info[name_end] != '{' && class_info[name_end] != ':') { name_end++; }
      std::string type_name = class_info.substr(name_start, name_end - name_start);
      type_names.push_back(type_name);
      file_remaining = class_match.suffix();
    }
    file_stream.close();
  }
  system("rm ./temp_files/gdbinfocmdtmp.txt");
  system("touch ./temp_files/gdbinfocmdtmp.txt");
  std::ofstream gdb_info_cmd_stream("./temp_files/gdbinfocmdtmp.txt");
  std::string gdb_info_cmd;
  for (int i = 0; i < type_names.size(); ++i)
  {
    gdb_info_cmd += "info types ^" + type_names[i] + "$\n";
    gdb_info_cmd += "info types ^" + type_names[i] + "<.*>$\n";
  }
  for (int i = 0; i < gdb_info_cmd.length(); ++i)
  {
    gdb_info_cmd_stream.put(gdb_info_cmd[i]);
  }
  gdb_info_cmd_stream.close();
  std::string gdbinfocmd = "gdb -x ./temp_files/gdbinfocmdtmp.txt -batch " + file_names[file_names.size() - 1] + " > ./temp_files/gdbinfoouttmp.txt";
  system(gdbinfocmd.c_str());
  std::vector<std::string> types_full;
  std::ifstream full_types_file("./temp_files/gdbinfoouttmp.txt");
  if (!full_types_file.is_open())
  {
    std::cout << "Failed to open ./temp_files/gdbinfoouttmp.txt" << std::endl;
    return 1;
  }
  std::string line;
  getline(full_types_file, line);
  while (!full_types_file.eof())
  {
    std::regex file_line_regex("[0-9]+:\\s*([a-zA-Z_][a-zA-Z0-9_]*);");
    std::smatch file_line_match;
    std::string file_line_str;
    if (regex_match(line, file_line_match, file_line_regex))
    {
      int name_start = 0;
      while (isdigit(line[name_start])) { ++name_start; }
      ++name_start;
      while (isspace(line[name_start])) { ++name_start; }
      int name_end = name_start;
      while (line[name_end] != ';') { ++name_end; }
      std::string type_name_full = line.substr(name_start, name_end - name_start);
      types_full.push_back(type_name_full);
    }
    getline(full_types_file, line);
  }
  full_types_file.close();
  system("rm ./temp_files/gdbtypecmdtmp.txt");
  system("touch ./temp_files/gdbtypecmdtmp.txt");
  std::ofstream gdb_type_cmd_stream("./temp_files/gdbtypecmdtmp.txt");
  std::string gdb_type_cmd;
  for (int i = 0; i < types_full.size(); ++i)
  {
    gdb_type_cmd += "ptype /o " + types_full[i] + "\n";
  }
  for (int i = 0; i < gdb_type_cmd.length(); ++i)
  {
    gdb_type_cmd_stream.put(gdb_type_cmd[i]);
  }
  gdb_type_cmd_stream.close();
  std::string gdbtypecmd = "gdb -x ./temp_files/gdbtypecmdtmp.txt -batch " + file_names[file_names.size() - 1] + " > ./temp_files/gdbtypeouttmp.txt";
  system(gdbtypecmd.c_str());
  std::ifstream type_out_stream("./temp_files/gdbtypeouttmp.txt");
  if (!type_out_stream.is_open())
  {
    std::cout << "Failed to open ./temp_files/gdbtypeouttmp.txt" << std::endl;
    return 1;
  }
  std::string classes_file;
  while (!type_out_stream.eof())
  {
    classes_file += type_out_stream.get();
  }

  std::regex type_out_class_regex("/\\*\\s*offset\\s*\\|\\s*size\\s*\\*/\\s*type\\s*=\\s*(class |struct )?([a-zA-Z_][a-zA-Z0-9_]*)\\s*(\\s*:\\s*(public|protected|private)\\s*([a-zA-Z_][a-zA-Z0-9_]*)\\s*)?\\{");
  std::smatch type_out_class_match;

  while (std::regex_search(classes_file, type_out_class_match, type_out_class_regex))
  {
    std::string class_string = type_out_class_match.str();
    int pos = 0;
    int bracket_depth = 1;
    classes_file = type_out_class_match.suffix();
    while (bracket_depth > 0)
    {
      if (classes_file[pos] == '{') { ++bracket_depth; }
      if (bracket_depth == 1)
      {
        class_string += classes_file[pos]; // there can be some semicolons in the middle of detailed explications of classes
      }
      if (classes_file[pos] == '}') { --bracket_depth; }
      ++pos;
    }

    // find the name
    int class_find = class_string.find("class");
    int struct_find = class_string.find("struct");
    int cs_min;
    if (class_find == std::string::npos && struct_find != std::string::npos)
    {
      cs_min = struct_find;
    }
    else if (class_find != std::string::npos && struct_find == std::string::npos)
    {
      cs_min = class_find;
    }
    else
    {
      cs_min = std::min(class_find, struct_find);
    }

    // cs_min is now at the start of 'class'/'struct'
    int name_start = cs_min;
    while (!isspace(class_string[name_start])) { ++name_start; }
    ++name_start;
    int name_end = name_start;
    while (!isspace(class_string[name_end])) { ++name_end; }
    std::string udt_name = class_string.substr(name_start, name_end - name_start);

    std::vector<Member> member_variables;

    std::regex member_regex("/\\*\\s*[0-9]+([^;])*;");
    std::smatch member_match;
    std::string member_string;
    while (regex_search(class_string, member_match, member_regex))
    {
      // get name
      member_string = member_match.str();
      int name_end = member_string.length() - 1;
      int name_start = name_end - 1;
      bool is_array = false;
      while (isalnum(member_string[name_start]) || member_string[name_start] == '_' || member_string[name_start] == '[' || member_string[name_start] == ']')
      {
        if (member_string[name_start] == '[' || member_string[name_start] == ']')
        {
          is_array = true;
        }
        --name_start;
      }
      ++name_start;
      std::string member_name = member_string.substr(name_start, name_end - name_start);

      // get size
      std::string size_string;
      int size_start = 2;
      while (isspace(member_string[size_start])) { ++size_start; }
      while (isdigit(member_string[size_start])) { ++size_start; }
      while (isspace(member_string[size_start])) { ++size_start; }
      ++size_start;
      while (isspace(member_string[size_start])) { ++size_start; }
      int size_end = size_start;
      while (isdigit(member_string[size_end])) { ++size_end; }
      size_t member_size = stoull(member_string.substr(size_start, size_end - size_start));

      // get align...
      int align_start = size_end;
      while (!isalpha(member_string[align_start])) { ++align_start; }
      std::string align_string = member_string.substr(align_start, name_start - align_start);
      size_t member_align;
      if (!is_array)
      {
        member_align = member_size;
      }
      else
      {
        member_align = 0; //get_alignment(align_string);
      }

      Member m({member_name, member_size, align_string, member_align});

      member_variables.push_back(m);

      class_string = member_match.suffix();
    }

    int total_size_end = class_string.length() - 1;
    while (!isdigit(class_string[total_size_end])) { --total_size_end; }
    int total_size_start = total_size_end;
    ++total_size_end;
    while (isdigit(class_string[total_size_start])) { --total_size_start; }
    ++total_size_start;
    int total_class_size = stoi(class_string.substr(total_size_start, total_size_end - total_size_start));

    UDType_new ud(udt_name, member_variables, total_class_size);

    udtypes.push_back(ud);

    // alignment calculation
      // if it begins with class std::array or just std::array, jump beyond that and recursively call the function

    classes_file = classes_file.substr(pos + 1);
  }

  for (int i = 0; i < udtypes.size(); ++i)
  {
    std::cout << "=== CLASS ===" << std::endl;
    std::cout << "Name: " << udtypes[i].get_name() << std::endl;
    std::cout << "Total size: " << udtypes[i].get_total_size() << std::endl;
    for (int j = 0; j < udtypes[i].member_variables.size(); ++j)
    {
      std::cout << "== MEMBER ==" << std::endl;
      std::cout << "Name: " << udtypes[i].member_variables[j].name << std::endl;
      std::cout << "Size: " << udtypes[i].member_variables[j].size << std::endl;
      // type also
      std::cout << "Alignment: " << udtypes[i].member_variables[j].alignment << std::endl;
    } 
  }

  return 0;
}

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
    /*
    do
    {
        int size = calculate_size(proposed_types_list, udtype_sizes);
        if (size < min_size)
        {
            min_size = size;
            new_ordering = proposed_types_list;
        }
    } while(std::next_permutation(proposed_types_list.begin(), proposed_types_list.end()));
    */
    std::sort ( begin(proposed_types_list),
                end(proposed_types_list), 
                [](variable_info const& first, variable_info const& second)
                {
                  return get<1>(first) > get<1>(second);
                }
              );
    min_size = calculate_size(proposed_types_list, udtype_sizes);
    new_ordering = proposed_types_list;
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
    
    return user_defined_types;
}