#include "class_parser.hpp"

size_t FileCollection::get_alignment(std::string alignment_string)
{
  if (alignment_string.substr(0, 5) == "class")
  {
    alignment_string = alignment_string.substr(6);
  }
  else if (alignment_string.substr(0, 6) == "struct")
  {
    alignment_string = alignment_string.substr(7);
  }

  size_t string_end = alignment_string.length() - 1;
  while (string_end > 0)
  {
    if (alignment_string[string_end] == '>')
    {
      break;
    }
    if (alignment_string[string_end] == '*')
    {
      return alignof(void*);
    }
    --string_end;
  }

  size_t class_string_end = 0;
  while ( class_string_end < alignment_string.length() &&
          isalnum(alignment_string[class_string_end]) ||
          alignment_string[class_string_end] == '_' ||
          alignment_string[class_string_end] == ':' ||
          isspace(alignment_string[class_string_end]) ||
          alignment_string[class_string_end] == '*')
  {
    ++class_string_end;
    if (class_string_end > 0 && alignment_string[class_string_end - 1] == '*')
    {
      break;
    }
  }

  std::string class_string = alignment_string.substr(0, class_string_end);

  if (class_string.substr(0, 10) == "std::array")
  {
    return get_alignment(class_string.substr(11));
  }

  for (int i = 0; i < udtypes.size(); ++i) // check for alignment of user type
  {
    if (class_string.find(udtypes[i].get_name()) != std::string::npos)
    {
      return std::min(udtypes[i].get_total_size(), 8ul);
    }
  }

  if (class_string.find("char16_t") != std::string::npos)
  {
    return alignof(char16_t);
  }
  else if (class_string.find("char32_t") != std::string::npos)
  {
    return alignof(char32_t);
  }
  else if (class_string.find("64") != std::string::npos)
  {
    return 8;
  }
  else if (class_string.find("32") != std::string::npos)
  {
    return 4;
  }
  else if (class_string.find("16") != std::string::npos)
  {
    return 2;
  }
  else if (class_string.find("8") != std::string::npos)
  {
    return 1;
  }
  else if (class_string.find("bool") != std::string::npos)
  {
    return alignof(bool);
  }
  else if (class_string.find("long long") != std::string::npos)
  {
    return alignof(long long);
  }
  else if (class_string.find("long double") != std::string::npos)
  {
    return alignof(long double);
  }
  else if (class_string.find("double") != std::string::npos)
  {
    return alignof(double);
  }
  else if (class_string.find("float") != std::string::npos)
  {
    return alignof(float);
  }
  else if (class_string.find("long") != std::string::npos)
  {
    return alignof(long);
  }
  else if (class_string.find("short") != std::string::npos)
  {
    return alignof(short);
  }
  else if (class_string.find("int") != std::string::npos)
  {
    return alignof(int);
  }
  else if (class_string.find("wchar") != std::string::npos)
  {
    return alignof(wchar_t);
  }
  else if (class_string.find("char") != std::string::npos)
  {
    return alignof(char);
  }
  return 8;
}

int FileCollection::detect_types(std::bitset<8>& flags)
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
    std::vector<UDType_new> type_info;
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
  
  std::ifstream exist_test("./temp_files/gdbinfocmdtmp.txt");
  if (exist_test.good())
  {
    exist_test.close();
    system("rm ./temp_files/gdbinfocmdtmp.txt");
  }
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
    std::regex file_line_regex("[0-9]+:\\s*([a-zA-Z_][a-zA-Z0-9_<>]*);");
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
  exist_test.open("./temp_files/gdbtypecmdtmp.txt");
  if (exist_test.good())
  {
    exist_test.close();
    system("rm ./temp_files/gdbtypecmdtmp.txt");
  }
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

  std::regex type_out_class_regex("/\\*\\s*offset\\s*\\|\\s*size\\s*\\*/\\s*type\\s*=\\s*(class |struct )?([a-zA-Z_][a-zA-Z0-9_<>]*)\\s*.*\\{"); // use the fact that .* does not include newlines
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

    std::regex member_regex("/\\*\\s*[0-9]+([^;])*;"); // here it is annoying that .* does not include newlines
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
      int align_end = name_start;
      --align_end;
      while (isspace(member_string[align_end])) { --align_end; }
      ++align_end;
      std::string align_string = member_string.substr(align_start, align_end - align_start);
      size_t member_align = 0;
      if (!is_array)
      {
        member_align = std::min(member_size, 8ul);
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

    classes_file = classes_file.substr(pos + 1);
  }

  for (int i = 0; i < udtypes.size(); ++i) // can only get alignments when all udtype sizes are known
  {
    for (int j = 0; j < udtypes[i].member_variables.size(); ++j)
    {
      if (udtypes[i].member_variables[j].alignment == 0)
      {
        udtypes[i].member_variables[j].alignment = get_alignment(udtypes[i].member_variables[j].align_string);
      }
    }
  }

  /*
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
      std::cout << "Alignment type: " << udtypes[i].member_variables[j].align_string << std::endl;
      std::cout << "Alignment: " << udtypes[i].member_variables[j].alignment << std::endl;
    } 
  }
  */

  return 0;
}

size_t UDType_new::calculate_size()
{
    size_t curr_align = 0;
    for (int i = 0; i < (int) member_variables.size() - 1; ++i)
    {
        curr_align += member_variables[i].size;
        int align_target = member_variables[i + 1].alignment;
        if (align_target == 0)
        {
            std::cout << "Alignment 0 error" << std::endl;
            return 1;
        }
        while (curr_align % align_target != 0) { curr_align++; }
    }
    if (member_variables.size() > 0)
    {
      curr_align += member_variables[member_variables.size() - 1].alignment;
    }
    while (curr_align % 8 != 0) { curr_align++; }
    int size = curr_align;
    return size;
}

size_t UDType_new::calculate_size(std::vector<Member> proposed_types_list)
{
    size_t curr_align = 0;
    for (int i = 0; i < (int) proposed_types_list.size() - 1; ++i)
    {
        curr_align += proposed_types_list[i].size;
        int align_target = proposed_types_list[i + 1].alignment;
        if (align_target == 0)
        {
            std::cout << "Alignment 0 error" << std::endl;
            return 0;
        }
        while (curr_align % align_target != 0) { curr_align++; }
    }
    if (proposed_types_list.size() > 0)
    {
      curr_align += proposed_types_list[proposed_types_list.size() - 1].alignment;
    }
    while (curr_align % 8 != 0) { curr_align++; }
    int size = curr_align;
    return size;
}

bool UDType_new::suggest_optimisation(int critical_stride)
{
  bool optimised = false;
  int curr_size = calculate_size();
  if (curr_size == 0)
  {
    return false;
  }
  else if (member_variables.size() == 0)
  {
    return false;
  }
  std::vector<Member> new_ordering = member_variables;
  std::vector<Member> proposed_types_list = member_variables;
  int min_size = curr_size;
  std::sort ( begin(proposed_types_list),
              end(proposed_types_list), 
              [](Member const& first, Member const& second)
              {
                return first.size > second.size;
              }
            );
  min_size = calculate_size(proposed_types_list);
  new_ordering = proposed_types_list;
  if (min_size < curr_size)
  {
      optimised = true;
      std::cout << std::endl << "=== An inefficient data member ordering has been detected ===" << std::endl << std::endl;
      std::cout << "Typename: " << name << std::endl << std::endl;
      std::cout << "Current ordering:" << std::endl << std::endl;
      for (size_t j = 0; j < member_variables.size(); ++j)
      {
          std::cout << member_variables[j].name << ";" << std::endl;
      }

      std::cout << std::endl << "Current size: " << curr_size << std::endl << std::endl;

      std::cout << "Proposed ordering:" << std::endl << std::endl;
      for (size_t j = 0; j < new_ordering.size(); ++j)
      {
          std::cout << new_ordering[j].name << ";" << std::endl;
      }
      std::cout << std::endl << "New size: " << min_size << std::endl << std::endl;
      std::cout << "This ordering saves " << curr_size - min_size << " bytes." << std::endl << std::endl;
  }
  int lcm = std::lcm(total_size, critical_stride);
  if ((lcm / total_size) <= 16)
  {
    optimised = true;
    std::cout << std::endl << "=== An inefficient class size has been detected ===" << std::endl << std::endl;
    std::cout << "Typename: " << name << std::endl << std::endl;
    std::cout << "This type has a total size of " << total_size << " bytes, whose Least Common Multiple with your processor's L1 data-cache critical stride of " << critical_stride << " bytes is " << lcm << "." << std::endl;
    std::cout << "This means that data members " << lcm / curr_size << " objects apart in contiguous memory will compete with each other for cache space." << std::endl << std::endl;
  }
  return optimised;
}

bool FileCollection::suggest_optimisations(int critical_stride)
{
  bool optimised = false;
  for (int i = 0; i < udtypes.size(); ++i)
  {
    if (udtypes[i].suggest_optimisation(critical_stride))
    {
      optimised = true;
    }
  }
  return optimised;
}