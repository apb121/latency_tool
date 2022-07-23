#include <iostream>
#include <vector>
#include <regex>
#include <string>

#include "user_options.hpp"

int UserOptions::parse_flags(int argc, char** argv)
{
  int i = 1;
  for (i = 1; i < argc; ++i)
  {
    if (strncmp(argv[i], "-s", 2) == 0 || strncmp(argv[i], "--s", 3) == 0)
    {
      if (flags.test(6))
      {
        std::cerr << "Incompatible combination of flags (--source-code-only and --binary-only)." << std::endl;
        return 1;
      }
      flags.set(7, true);
    }
    else if (strncmp(argv[i], "-b", 2) == 0 || strncmp(argv[i], "--b", 3) == 0)
    {
      if (flags.test(7))
      {
        std::cerr << "Incompatible combination of flags (--source-code-only and --binary-only)." << std::endl;
        return 1;
      }
      flags.set(6, true);
    }
    else if (strncmp(argv[i], "-c", 2) == 0 || strncmp(argv[i], "--c", 3) == 0)
    {
      if (flags.test(4))
      {
        std::cerr << "Incompatible combination of flags (--cache-info-only and --manual-cache)." << std::endl;
        return 1;
      }
      flags.set(5, true);
    }
    else if (strncmp(argv[i], "-m", 2) == 0 || strncmp(argv[i], "--m", 3) == 0)
    {
      if (flags.test(5))
      {
        std::cerr << "Incompatible combination of flags (--cache-info-only and --manual-cache)." << std::endl;
        return 1;
      }
      flags.set(4, true);
      std::regex m_flag_regex("((-m=)|(--manual-cache=))([0-9]+:[0-9]+:[0-9]+)(::[0-9]+:[0-9]+:[0-9]+)*");
      if (!regex_match(argv[i], m_flag_regex))
      {
        std::cerr << "Invalid -manual-cache flag format." << std::endl;
        return 1;
      }
      std::string flag(argv[i]);
      std::regex cache_size("[0-9]+");
      std::smatch size_match;
      int size_int[3];
      int count = 0;
      while (std::regex_search(flag, size_match, cache_size))
      {
        size_int[0] = stoi(size_match.str());
        flag = size_match.suffix();
        std::regex_search(flag, size_match, cache_size);
        size_int[1] = stoi(size_match.str());
        flag = size_match.suffix();
        std::regex_search(flag, size_match, cache_size);
        size_int[2] = stoi(size_match.str());
        flag = size_match.suffix();
        switch (count)
        {
          case 0:
            proc.l1d = new L1d_cache(size_int[0], size_int[2], size_int[1]);
            break;
          case 1:
            proc.l1i = new L1i_cache(size_int[0], size_int[2], size_int[1]);
            break;
          case 2:
            proc.l2 = new L2_cache(size_int[0], size_int[2], size_int[1]);
            break;
          case 3:
            proc.l3 = new L3_cache(size_int[0], size_int[2], size_int[1]);
            break;
          case 4:
            proc.l4 = new L4_cache(size_int[0], size_int[2], size_int[1]);
            break;
        }
        ++count;
      }
    }
    else if (strncmp(argv[i], "-n", 2) == 0 || strncmp(argv[i], "--n", 3) == 0)
    {
      flags.set(3, true);
    }
    else if (strncmp(argv[i], "-k", 2) == 0 || strncmp(argv[i], "--k", 3) == 0)
    {
      flags.set(2, true); 
    }
    else if (strncmp(argv[i], "-l", 2) == 0 || strncmp(argv[i], "--coe", 3) == 0)
    {
      std::regex l_flag_regex("((-l=)|(--coexecution-level=))([0-9]+)");
      if (!regex_match(argv[i], l_flag_regex))
      {
        std::cerr << "Invalid --coexecution-level flag format." << std::endl;
        return 1;
      }
      std::regex coex_level_regex("[0-9]+");
      std::smatch level_match;
      std::string flag(argv[i]);
      regex_search(flag, level_match, coex_level_regex);
      int coex_level = stoi(level_match.str());
      if (coex_level < 0 || coex_level > 100) // some large number, probably like 4 would do because high numbers here can be dangerous; perhaps just add a warning if its over about 3
      {
        std::cerr << "Invalid coexecution level." << std::endl;
        return 1;
      }
      coex = coex_level;
      std::cout << "Setting coex to " << coex_level << std::endl;
    }
    else if (strncmp(argv[i], "-t", 2) == 0 || strncmp(argv[i], "--com", 3) == 0)
    {
      std::regex t_flag_regex("((-t=)|(--competition-threshold=))([0-9]+)");
      if (!regex_match(argv[i], t_flag_regex))
      {
        std::cerr << "Invalid --competition-threshold flag format." << std::endl;
        return 1;
      }
      std::regex comp_level_regex("[0-9]+");
      std::smatch threshold_match;
      std::string flag(argv[i]);
      regex_search(flag, threshold_match, comp_level_regex);
      int comp_level = stoi(threshold_match.str());
      if (comp_level < 1 || comp_level > 100000) // some large number
      {
        std::cerr << "Invalid competition thredhold." << std::endl;
        return 1;
      }
      comp = comp_level;
      std::cout << "Setting comp to " << comp_level << std::endl;
    }
    else if (strncmp(argv[i], "-", 1) == 0)
    {
        std::cerr << "Unrecognised flag \"" << argv[i] << "\"" << std::endl;
        return 1;
    }
    else
    {
        break;
    }
  }
  while (i < argc)
  {
    file_names.push_back(argv[i]);
    ++i;
  }
  return 0;
}