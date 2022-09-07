#include "user_options.hpp"

int UserOptions::check_requirements()
{
  int ret = 0;
  int err = 0;
  if (!flags.test(CACHE_INFO_ONLY))
  {
    if (file_names.size() > 1)
    {
      ret = system("gdb --version >> ./temp_files/debugtmp.txt");
      if (ret)
      {
        std::cout << "You need gdb to use this tool with those options." << std::endl;
        err = 1;
      }
    }
    ret = system("nm --version >> ./temp_files/debugtmp.txt");
    if (ret)
    {
      std::cout << "You need nm to use this tool." << std::endl;
      err = 1;
    }
    ret = system("objdump --version >> ./temp_files/debugtmp.txt");
    if (ret)
    {
      std::cout << "You need objdump to use this tool." << std::endl;
      err = 1;
    }
  }
  if (!flags.test(NO_EMPIRICAL))
  {
    ret = system("g++ --version >> ./temp_files/debugtmp.txt");
    if (ret)
    {
      std::cout << "You need g++ to use this tool with those options." << std::endl;
      err = 1;
    }
    ret = system("perf --version >> ./temp_files/debugtmp.txt");
    if (ret)
    {
      std::cout << "You need perf to use this tool with those options." << std::endl;
      err = 1;
    }
  }
  return err;
}

int UserOptions::parse_flags(int argc, char** argv)
{
  int i = 1;
  for (; i < argc; ++i)
  {
    if (strncmp(argv[i], "-h", 2) == 0 || strncmp(argv[i], "--h", 3) == 0)
    {
      std::cout << std::endl << "===== Help =====" << std::endl << std::endl;
      std::cout << "This tool identifies potential sources of L1-data and -instruction cache inefficiency." << std::endl << std::endl;
      std::cout << "To use the tool, invoke ./latency_tool with a list of filepaths as command line arguments: \"./latency_tool [--options ...] [source_code_files ...] binary_file)\". The last file listed will be interpreted as a binary (which must be compiled with debugging symbols, e.g., the gcc flag -g), and any other files listed will be interpreted as the C++ source code files from which the binary was compiled." << std::endl << std::endl;
      std::cout << "Unless guided to to otherwise (see the options available below), the tool will use your processor's cache info to:" << std::endl;
      std::cout << "-- Identify inefficiently ordered user-defined classes and structs" << std::endl;
      std::cout << "-- Identify inefficiently sized user-defined classes and structs" << std::endl;
      std::cout << "-- Identify inefficiently sized functions" << std::endl;
      std::cout << "-- Identify groups of functions which both coexecute and compete for cache space" << std::endl << std::endl;
      std::cout << "Options available to the user are as follows: " << std::endl << std::endl;

      std::cout << "  -c/--cache-info-only" << std::endl << std::endl;
      std::cout << "    With this option, the tool will analyse your processor's caches and print out information about their sizes, linesizes, associativities, and critical stride. If the system does not provide these values directly, they will be analysed using empirical tests (which may take a couple of minutes). This option is incompatible with the -m/-manual-cache flag." << std::endl << std::endl;

      std::cout << "  -m=/--manual-cache=<size>:<associativity>:<linesize>::<size>:<associativity>:<linesize>:: ..." << std::endl << std::endl;
      std::cout << "    With this option, the tool will run using the cache dimensions you specify manually. The caches should be specified in the order (1) L1 data (2) L1 instruction (3) L2 (4) L3 (5) L4. Any missing caches will be ignored. You can also choose \"--manual-cache=default\", which will set the L1 data and instruction caches each to 32768 bytes large, with 8-way associativities and 64-byte linesizes. The manual cache option is incompatible with the -c/--cache-info-only flag." << std::endl << std::endl;

      std::cout << "  -n/--no-empirical-tests" << std::endl << std::endl;
      std::cout << "    With this option, the tool will not run any empirical tests of cache dimensions. If the system does not provide any necessary values, associativities will default to 8, linesizes to 64, and critical strides to 4096." << std::endl << std::endl;

      std::cout << "  -l=/--coexecution-level=<level>" << std::endl << std::endl;
      std::cout << "    With this option, the user can decide how many levels of coexecution indirection constitutes coexecution. For example, given functions A(), B(), C(), and D() where A calls B, B calls C, and C calls D, A and B would always be considered to coexecute, A and C would be considered to coexecute only with at least 1 level of indirection, and A and D would be considered to coexecute only with at least 2 levels of indirection. High levels of coexecution indirection may cause long analysis times." << std::endl << std::endl;

      std::cout << "  -t=/--competition-threshold=<threshold>" << std::endl << std::endl;
      std::cout << "    With this option, the user can decide the threshold of overlap in cache space that constitutes competition. For example, with a threshold of 200 bytes, any two functions that only overlap in cache space by less than 200 bytes would not be considered as competing with each other. Low competition thresholds may cause long analysis times." << std::endl << std::endl;

      std::cout << "  -r=/--ranking-length=<length>" << std::endl << std::endl;
      std::cout << "    With this option, the user can decide how many of the most problematic groups of functions are printed after analysis." << std::endl << std::endl;

      std::cout << "  -a/--all-functions" << std::endl << std::endl;
      std::cout << "    With this option, the tool will include all functions found in the binary in its analysis, as opposed to the default behaviour of just using user-defined functions and functions in user-defined types. If the binary is not very small or the competition threshold is low, this can cause (extremely) long analysis times." << std::endl << std::endl;

      std::cout << "  -k/--keep-temporary-files" << std::endl << std::endl;
      std::cout << "    With this option, the tool will not automatically delete the temp files it used for its analysis when it terminates." << std::endl << std::endl;

      std::cout << "  -e/--use-existing-cache-temp-files" << std::endl << std::endl;
      std::cout << "    With this option, the tool will attempt to find previous empirical cache test temp files and use the results from those rather than run the tests again." << std::endl << std::endl;

      return 1;
    }
    if (strncmp(argv[i], "-a", 2) == 0 || strncmp(argv[i], "--a", 3) == 0)
    {
      std::cout << std::endl << "(Warning: unless your binary file is very small or your competition threshold is very high, the --all-functions flag can cause (extremely) long execution times." << std::endl;
      flags.set(ALL_FUNCTIONS, true);
    }
    else if (strncmp(argv[i], "-c", 2) == 0 || strncmp(argv[i], "--c", 3) == 0)
    {
      if (flags.test(MANUAL_CACHE))
      {
        std::cout << "Incompatible combination of flags (--cache-info-only and --manual-cache)." << std::endl;
        return 1;
      }
      flags.set(CACHE_INFO_ONLY, true);
    }
    else if (strncmp(argv[i], "-m", 2) == 0 || strncmp(argv[i], "--m", 3) == 0)
    {
      if (flags.test(MANUAL_CACHE))
      {
        std::cout << "(Superfluous manual cache specifications ignored.)" << std::endl;
        continue;
      }
      if (flags.test(CACHE_INFO_ONLY))
      {
        std::cout << "Incompatible combination of flags (--cache-info-only and --manual-cache)." << std::endl;
        return 1;
      }
      flags.set(MANUAL_CACHE, true);
      std::regex m_flag_regex("((-m=)|(--manual-cache=))(([0-9]+:[0-9]+:[0-9]+)(::[0-9]+:[0-9]+:[0-9]+)*|default)");
      if (!regex_match(argv[i], m_flag_regex))
      {
        std::cout << "Invalid -manual-cache flag format." << std::endl;
        return 1;
      }
      std::string flag(argv[i]);
      if (flag.substr(flag.length() - 7, 7) == "default")
      {
        proc.l1d = new L1d_cache(32768, 64, 8);
        proc.l1i = new L1i_cache(32768, 64, 8);
        proc.l2 = new L2_cache(262144, 64, 4);
        proc.l3 = new L3_cache(12582912, 64, 16);
        proc.l4 = new L4_cache(0, 0, 0);
        continue;
      }
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
      flags.set(NO_EMPIRICAL, true);
    }
    else if (strncmp(argv[i], "-k", 2) == 0 || strncmp(argv[i], "--k", 3) == 0)
    {
      flags.set(KEEP_TEMP, true); 
    }
    else if (strncmp(argv[i], "-l", 2) == 0 || strncmp(argv[i], "--coe", 3) == 0)
    {
      static bool set = false;
      if (set)
      {
        std::cout << "(Superfluous coexecution level specifications ignored.)" << std::endl;
        continue;
      }
      std::regex l_flag_regex("((-l=)|(--coexecution-level=))([0-9]+)");
      if (!regex_match(argv[i], l_flag_regex))
      {
        std::cout << "Invalid --coexecution-level flag format." << std::endl;
        return 1;
      }
      std::regex coex_level_regex("[0-9]+");
      std::smatch level_match;
      std::string flag(argv[i]);
      regex_search(flag, level_match, coex_level_regex);
      int coex_level = stoi(level_match.str());
      if (coex_level < 0)
      {
        std::cout << "Invalid coexecution level." << std::endl;
        return 1;
      }
      if (coex_level > 2)
      {
        std::cout << "Warning: high coexecution levels may cause long processing times." << std::endl;
      }
      coex = coex_level;
      set = true;
    }
    else if (strncmp(argv[i], "-t", 2) == 0 || strncmp(argv[i], "--com", 3) == 0)
    {
      static bool set = false;
      if (set)
      {
        std::cout << "(Superfluous competition threshold specifications ignored.)" << std::endl;
        continue;
      }
      std::regex t_flag_regex("((-t=)|(--competition-threshold=))([0-9]+)");
      if (!regex_match(argv[i], t_flag_regex))
      {
        std::cout << "Invalid --competition-threshold flag format." << std::endl;
        return 1;
      }
      std::regex comp_level_regex("[0-9]+");
      std::smatch threshold_match;
      std::string flag(argv[i]);
      regex_search(flag, threshold_match, comp_level_regex);
      int comp_level = stoi(threshold_match.str());
      if (comp_level < 1)
      {
        std::cout << "Invalid competition thredhold." << std::endl;
        return 1;
      }
      comp = comp_level;
      set = true;
    }
    else if (strncmp(argv[i], "-e", 2) == 0 || strncmp(argv[i], "--use", 3) == 0)
    {
      flags.set(EXISTING_TEMP_FILES, true);
      flags.set(KEEP_TEMP, true);
    }
    else if (strncmp(argv[i], "-r", 2) == 0 || strncmp(argv[i], "--ran", 3) == 0)
    {
      static bool set = false;
      if (set)
      {
        std::cout << "(Superfluous ranking length specifications ignored.)" << std::endl;
        continue;
      }
      std::regex r_flag_regex("((-r=)|(--ranking-length=))([0-9]+)");
      if (!regex_match(argv[i], r_flag_regex))
      {
        std::cout << "Invalid --ranking-length flag format." << std::endl;
        return 1;
      }
      std::regex rank_length_regex("[0-9]+");
      std::smatch rank_match;
      std::string flag(argv[i]);
      regex_search(flag, rank_match, rank_length_regex);
      int rank_len = stoi(rank_match.str());
      if (rank_len < 1)
      {
        std::cout << "Invalid ranking length." << std::endl;
        return 1;
      }
      ranking_length = rank_len;
      set = true;
    }
    else if (strncmp(argv[i], "-", 1) == 0)
    {
        std::cout << "Unrecognised flag \"" << argv[i] << "\"" << std::endl;
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

int UserOptions::run_file_setup()
{
  if (file_names.size() == 0 && !flags.test(CACHE_INFO_ONLY))
  {
    std::cout << "No files specified!" << std::endl;
    std::cout << "Normal usage: ./latency_tool [--options ...] [source_code_files ...] binary_file" << std::endl;
    std::cout << "For cache info only, invoke with the -c/--cache-only option." << std::endl;
    return 1;
  }

  std::ifstream f("./temp_files");
  if (!f.good())
  {
    system("mkdir temp_files");
  }
  f.close();

  for (size_t i = 0; i < file_names.size(); ++i)
  {
    std::ifstream f(file_names[i]);
    if (!f.good())
    {
      std::cout << "The file \"" << file_names[i] << "\" does not exist" << std::endl;
      return 1;
    }
  }
  return 0;
}

int UserOptions::run_cache_setup()
{
  if (!flags.test(MANUAL_CACHE))
  {
    std::cout << std::endl << "===========================================================" << std::endl << std::endl;
    std::cout << "CACHE ANALYSIS" << std::endl;
    std::cout << std::endl << "===========================================================" << std::endl << std::endl;
    proc.l1d = new L1d_cache(
      sysconf(_SC_LEVEL1_DCACHE_SIZE),
      sysconf(_SC_LEVEL1_DCACHE_LINESIZE),
      sysconf(_SC_LEVEL1_DCACHE_ASSOC)
    );
    proc.l1i = new L1i_cache(
      sysconf(_SC_LEVEL1_ICACHE_SIZE),
      sysconf(_SC_LEVEL1_ICACHE_LINESIZE),
      sysconf(_SC_LEVEL1_ICACHE_ASSOC)
    );
    proc.l2 = new L2_cache(
      sysconf(_SC_LEVEL2_CACHE_SIZE),
      sysconf(_SC_LEVEL2_CACHE_LINESIZE),
      sysconf(_SC_LEVEL2_CACHE_ASSOC)
    );
    proc.l3 = new L3_cache(
      sysconf(_SC_LEVEL3_CACHE_SIZE),
      sysconf(_SC_LEVEL3_CACHE_LINESIZE),
      sysconf(_SC_LEVEL3_CACHE_ASSOC)
    );
    proc.l4 = new L4_cache(
      sysconf(_SC_LEVEL4_CACHE_SIZE),
      sysconf(_SC_LEVEL4_CACHE_LINESIZE),
      sysconf(_SC_LEVEL4_CACHE_ASSOC)
    );
  }

  if (flags.test(NO_EMPIRICAL) && ( proc.l1d->get_assoc() <= 0 ||
                                    proc.l1d->get_critical_stride() <= 0 ||
                                    proc.l1d->get_linesize() <= 0 ||
                                    proc.l1i->get_assoc() <= 0 ||
                                    proc.l1i->get_critical_stride() <= 0 ||
                                    proc.l1i->get_linesize() <= 0 ))
  {
    std::cout << "The following cache dimensions are unknown, but you have chosen not to execute empirical tests:" << std::endl;
    if (proc.l1d->get_size() <= 0) { std::cout << "-- L1 data cache size" << std::endl; }
    if (proc.l1d->get_assoc() <= 0) { std::cout << "-- L1 data cache associativity" << std::endl; }
    if (proc.l1d->get_linesize() <= 0) { std::cout << "-- L1 data cache linesize" << std::endl; }
    if (proc.l1i->get_size() <= 0) { std::cout << "-- L1 instruction cache size" << std::endl; }
    if (proc.l1i->get_assoc() <= 0) { std::cout << "-- L1 instruction cache associativity" << std::endl; }
    if (proc.l1i->get_linesize() <= 0) { std::cout << "-- L1 instruction cache linesize" << std::endl; }
    std::cout << "Associativities will default to 8, linesizes to 64, and critical strides to 4096.";
    std::cout << " This can produce results that are inaccurate for your processor, but the suggestions may still prove useful." << std::endl;
    if (proc.l1d->get_assoc() <= 0) { proc.l1d->set_assoc(8); }
    if (proc.l1i->get_assoc() <= 0) { proc.l1i->set_assoc(8); }
    if (proc.l1d->get_linesize() <= 0) { proc.l1d->set_linesize(64); }
    if (proc.l1i->get_linesize() <= 0) { proc.l1i->set_linesize(64); }
    if (proc.l1d->get_critical_stride() <= 0) { proc.l1d->set_critical_stride(4096); }
    if (proc.l1i->get_critical_stride() <= 0) { proc.l1i->set_critical_stride(4096); }
  }
  else if (!(flags.test(NO_EMPIRICAL)))
  {
    int suggested_values[4] = {0, 0, 0, 0};
    if (proc.l1d->get_assoc() <= 0 && proc.l1d->get_critical_stride() <= 0)
    {
      std::cout << std::endl << "The associativity and critical stride of the L1 data cache are both unknown." << std::endl;
      std::cout << "They will now be tested empirically. This may take a couple of minutes." << std::endl;

      suggested_values[0] = proc.l1d->empirical_assoc_test(flags);
      if (suggested_values[0] <= 0)
      {
        std::cout << "Error assessing L1 data-cache associativity. Defaulting to 8." << std::endl;
        suggested_values[0] = 8;
      }
      std::cout << "Suggested L1 data-cache associativity: " << suggested_values[0] << std::endl;
      suggested_values[1] = proc.l1d->empirical_stride_test(flags);
      if (suggested_values[1] <= 0)
      {
        std::cout << "Error assessing L1 data-cache critical stride. Defaulting to 4096" << std::endl;
        suggested_values[1] = 4096;
      }
      std::cout << "Suggested L1 data-cache critical stride: " << suggested_values[1] << std::endl;

      std::cout << std::endl << "Setting L1 data-cache associativity to " << suggested_values[0] << std::endl;
      proc.l1d->set_assoc(suggested_values[0]);
      int pos_critical_stride = proc.l1d->get_size() / proc.l1d->get_assoc();

      if (  ((pos_critical_stride) & (pos_critical_stride - 1) == 0) && pos_critical_stride != 0 &&
            ((suggested_values[1]) & (suggested_values[1] - 1) != 0))
      {
        std::cout << std::endl << "Setting L1 data-cache critical stride to " << pos_critical_stride << std::endl << std::endl;          
      }
      else if ( ((pos_critical_stride) & (pos_critical_stride - 1) != 0) &&
                ((suggested_values[1]) & (suggested_values[1] - 1) == 0) && suggested_values[1] != 0)
      {
        std::cout << std::endl << "Setting L1 data-cache critical stride to " << suggested_values[1] << std::endl << std::endl; 
      }
      else
      {
        std::cout << std::endl << "Setting L1 data-cache critical stride to " << std::min(pos_critical_stride, suggested_values[1]) << std::endl << std::endl;
        proc.l1d->set_critical_stride(std::min(pos_critical_stride, suggested_values[1]));
      }
    }
    else if (proc.l1d->get_assoc() <= 0)
    {
      std::cout << "The associativity of the L1 data cache is unknown." << std::endl;
      std::cout << "It will now be tested empirically. This may take a couple of minutes." << std::endl;

      suggested_values[0] = proc.l1d->empirical_assoc_test(flags);
      if (suggested_values[0] <= 0)
      {
        std::cout << "Error assessing L1 data-cache associativity. Defaulting to 8." << std::endl;
        suggested_values[0] = 8;
      }
      std::cout << "Suggested L1 data-cache associativity: " << suggested_values[0] << std::endl;

      std::cout << std::endl << "Setting L1 data-cache associavtivity to " << suggested_values[0] << std::endl << std::endl;
      proc.l1d->set_assoc(suggested_values[0]);
    }
    else if (proc.l1d->get_critical_stride() <= 0)
    {
      std::cout << "The critical stride of the L1 data cache is unknown." << std::endl;
      std::cout << "It will now be tested empirically. This may take a couple of minutes." << std::endl;

      suggested_values[1] = proc.l1d->empirical_stride_test(flags);
      if (suggested_values[1] <= 0)
      {
        std::cout << "Error assessing L1 data-cache critical stride. Defaulting to 4096" << std::endl;
        suggested_values[1] = 4096;
      }
      std::cout << "Suggested L1 data-cache critical stride: " << suggested_values[1] << std::endl;

      std::cout << std::endl << "Setting L1 data-cache critical stride to " << suggested_values[1] << std::endl << std::endl;
      proc.l1d->set_critical_stride(suggested_values[1]);
    }
    if (proc.l1i->get_assoc() <= 0 && proc.l1i->get_critical_stride() <= 0)
    {
      std::cout << "The associativity and critical stride of the L1 instruction-cache are both unknown." << std::endl;
      std::cout << "They will now be tested empirically. This may take a couple of minutes." << std::endl;

      suggested_values[2] = proc.l1i->empirical_assoc_test(flags);
      if (suggested_values[2] <= 0)
      {
        std::cout << "Error assessing L1 instruction-cache associativity. Defaulting to 8." << std::endl;
        suggested_values[2] = 8;
      }
      std::cout << "Suggested L1 instruction-cache associativity: " << suggested_values[2] << std::endl;
      suggested_values[3] = proc.l1i->empirical_stride_test(flags);
      if (suggested_values[3] <= 0)
      {
        std::cout << "Error assessing L1 instruction-cache critical stride. Defaulting to 4096." << std::endl;
        suggested_values[3] = 4096;
      }
      std::cout << "Suggested L1 instruction-cache critical stride: " << suggested_values[3] << std::endl;

      std::cout << std::endl << "Setting L1 instruction-cache associativity to " << suggested_values[2] << std::endl;
      proc.l1i->set_assoc(suggested_values[2]);
      int pos_critical_stride = proc.l1i->get_size() / proc.l1i->get_assoc();

      if (  ((pos_critical_stride) & (pos_critical_stride - 1) == 0) && pos_critical_stride != 0 &&
            ((suggested_values[3]) & (suggested_values[3] - 1) != 0))
      {
        std::cout << std::endl << "Setting L1 instruction-cache critical stride to " << pos_critical_stride << std::endl << std::endl;  
        proc.l1i->set_critical_stride(pos_critical_stride);
      }
      else if ( ((pos_critical_stride) & (pos_critical_stride - 1) != 0) &&
                ((suggested_values[3]) & (suggested_values[3] - 1) == 0) && suggested_values[3] != 0)
      {
        std::cout << std::endl << "Setting L1 instruction-cache critical stride to " << suggested_values[3] << std::endl << std::endl;
        proc.l1i->set_critical_stride(suggested_values[3]);
      }
      else
      {
        std::cout << std::endl << "Setting L1 instruction-cache critical stride to " << std::min(pos_critical_stride, suggested_values[3]) << std::endl << std::endl;
        proc.l1i->set_critical_stride(std::min(pos_critical_stride, suggested_values[3]));
      }
    }
    else if (proc.l1i->get_assoc() <= 0)
    {
      std::cout << "The associativity of the L1 instruction-cache is unknown." << std::endl;
      std::cout << "It will now be tested empirically. This may take a couple of minutes." << std::endl;

      suggested_values[2] = proc.l1i->empirical_assoc_test(flags);
      if (suggested_values[2] <= 0)
      {
        std::cout << "Error assessing L1 instruction-cache associativity. Defaulting to 8." << std::endl;
        suggested_values[2] = 8;
      }
      std::cout << "Suggested instruction-cache associativity: " << suggested_values[2] << std::endl;

      std::cout << std::endl << "Setting L1 instruction-cache associavtivity to " << suggested_values[2] << std::endl << std::endl;
      proc.l1i->set_assoc(suggested_values[2]);
    }
    else if (proc.l1i->get_critical_stride() <= 0)
    {
      std::cout << "The critical stride of the L1 instruction-cache is unknown." << std::endl;
      std::cout << "It will now be tested empirically. This may take a couple of minutes." << std::endl;

      suggested_values[3] = proc.l1i->empirical_stride_test(flags);
      if (suggested_values[3] <= 0)
      {
        std::cout << "Error assessing L1 instruction-cache critical stride. Defaulting to 4096." << std::endl;
        suggested_values[3] = 4096;
      }
      std::cout << "Suggested instruction-cache critical stride: " << suggested_values[3] << std::endl;

      std::cout << std::endl << "Setting L1 instruction-cache critical stride to " << suggested_values[3] << std::endl << std::endl;
      proc.l1i->set_assoc(suggested_values[3]);
    }
  }
  return 0;
}

int UserOptions::run_analysis()
{ 
  if (file_names.size() > 1)
  {
    std::cout << std::endl << "===========================================================" << std::endl << std::endl;
    std::cout << "SOURCE CODE ANALYSIS" << std::endl;
    std::cout << std::endl << "===========================================================" << std::endl << std::endl;
    std::cout << "This tool analyses your source code files to discover cache-inefficient data layouts in user-defined types." << std::endl;
    std::cout << "Any user-defined types which can be optimised by reordering their data members will be reported, along with suggestions for new layouts." << std::endl << std::endl;
    std::cout << "Additionally, this tool will identify any user-defined types where the Least Common Multiple of the size of the type and the critical stride of your processor is small." << std::endl;
    std::cout << "If objects of these types are stored in contiguous memory (such as an array or vector), the same data members in different objects may compete for space in the cache." << std::endl;
    std::cout << "If access patterns involve accessing the same data members across multiple contiguous objects, this may create latency problems." << std::endl << std::endl;
    std::cout << "Analysing your source code..." << std::endl << std::endl << std::flush;

    fc.populate_file_names(file_names);

    int src_ret = fc.detect_types(flags);
    if (src_ret)
    {
      std::cout << "Error detecting types." << std::endl;
      std::cout << "Exiting." << std::endl;
      return 1;
    }

    int l1d_critical_stride = 0;
    if (proc.l1d != nullptr)
    {
      l1d_critical_stride = proc.l1d->get_critical_stride();
    }
    if (l1d_critical_stride <= 0)
    {
      std::cout << std::endl << "(Warning: no L1 data cache critical stride was specified. Defaulting to 4096 bytes.)" << std::endl << std::endl;
      l1d_critical_stride = 4096;
    }
    bool optimised = fc.suggest_optimisations(l1d_critical_stride);

    if (optimised)
    {
      std::cout << std::endl << "===== Suggestions =====" << std::endl << std::endl;
      std::cout << "It is recommended, unless there is a specific reason to retain an inefficient ordering, that any reorderings identified above are implemented." << std::endl;
      std::cout << "If the overall size of a type may cause problems in the context of access patterns and the critical stride of the data cache, attempting to make the data structure smaller (perhaps by using different data types as sub-members or by using highly compact types like bitfields) is the ideal solution." << std::endl;
      std::cout << "Another mitigation strategy to consider is to avoid contiguous allocations of entire objects when individual data members are accessed in sequence across many of those objects. Instead of these 'arrays of structs', it is sometimes better to organise this data in 'structs of arrays', that is, structs which point to arrays which contain only those data members that are accessed in strided access patterns. This compacts the consecutively accessed data, significantly reducing the risk of cache conflicts." << std::endl;
      std::cout << "Failing that, it may (counterintuitively) be worth trying to make the object slightly larger to reduce the chance of desired accesses evicting useful data from the cache by increasing the Least Common Multiple of the object size and the critical stride." << std::endl << std::endl;
    }
    else
    {
      std::cout << "No data ordering or type-size inefficiencies have been detected!" << std::endl;
    }
  }
  
  std::cout << std::endl << "===========================================================" << std::endl << std::endl;
  std::cout << "BINARY ANALYSIS" << std::endl;
  std::cout << std::endl << "===========================================================" << std::endl << std::endl;
  std::cout << "Groups of functions that call each other and compete for the same portion of the cache may cause latency problems." << std::endl;
  std::cout << "If the number of functions in the group is above the associativity of the instruction cache ("<< proc.l1d->get_assoc() << " on this processor), these functions will definitely evict each other from the cache." << std::endl;
  std::cout << "If the number of functions in the group is equal to or slighly below the associativity of the instruction cache, there is a significant probability that the functions will evict each other from the cache. The uncertainty (standard deviation) of this probability can also be a source of jitter at runtime." << std::endl;
  std::cout << "The larger the region of cache memory that the functions compete for, the more code needs to be fetched from lower level caches the next time the evicted function needs to be executed, and the more significant the latency penalties are." << std::endl << std::endl;
  if (comp < 256)
  {
    std::cout << "(Warning: low competition thresholds may lead to long processing times.)" << std::endl << std::endl;
  }
  if (coex > 2)
  {
    std::cout << "(Warning: high coexecution levels may lead to long processing times.)" << std::endl << std::endl;
  }
  std::cout << "Analysing your binary..." << std::endl << std::endl;
  
  

  Binary b(file_names[file_names.size() - 1], fc.get_user_types());

  int bin_ret = 0;
  bin_ret = b.get_functions(*this);
  if (bin_ret != 0)
  {
    std::cout << std::endl << "Error reading functions from " << file_names[file_names.size() - 1] << "." << std::endl;
    std::cout << "Exiting." << std::endl;
    return 1;
  }

  bin_ret = b.populate_competition_vectors(*this);
  if (bin_ret != 0)
  {
    std::cout << std::endl << "Error populating competition vectors from " << file_names[file_names.size() - 1] << "." << std::endl;
    std::cout << "Exiting." << std::endl;
    return 1;
  }
  
  bin_ret = b.populate_coexecution_vectors(*this);
  if (bin_ret != 0)
  {
    std::cout << std::endl << "Error populating coexecution vectors from " << file_names[file_names.size() - 1] << "." << std::endl;
    std::cout << "Exiting." << std::endl;
    return 1;
  }

  bin_ret = b.find_problem_function_groups(*this);
  if (bin_ret != 0)
  {
    std::cout << std::endl << "Error finding problem function groups." << std::endl;
    std::cout << "Exiting." << std::endl;
    return 1;
  }
  std::cout << std::endl;

  return 0;
}

UserOptions::~UserOptions()
{
  if (!flags.test(KEEP_TEMP))
  {
    std::string cmd_rm_tmp = "rm ./temp_files/* > ./temp_files/debugtmp.txt";
    system(cmd_rm_tmp.c_str());
    std::string finished = "echo \"run finished " + std::to_string(std::time(0)) + "\" > ./temp_files/debugtmp.txt";
    system(finished.c_str());
  }
}