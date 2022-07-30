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
      if (coex_level < 0 || coex_level > 100)
      {
        std::cerr << "Invalid coexecution level." << std::endl;
        return 1;
      }
      coex = coex_level;
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
      if (comp_level < 1 || comp_level > 100000)
      {
        std::cerr << "Invalid competition thredhold." << std::endl;
        return 1;
      }
      comp = comp_level;
    }
    else if (strncmp(argv[i], "-e", 2) == 0 || strncmp(argv[i], "--use", 3) == 0)
    {

      flags.set(1, true);
      flags.set(2, true);
    }
    else if (strncmp(argv[i], "-r", 2) == 0 || strncmp(argv[i], "--ran", 3) == 0)
    {
      std::regex r_flag_regex("((-r=)|(--ranking-length=))([0-9]+)");
      if (!regex_match(argv[i], r_flag_regex))
      {
        std::cerr << "Invalid --ranking-length flag format." << std::endl;
        return 1;
      }
      std::regex rank_length_regex("[0-9]+");
      std::smatch rank_match;
      std::string flag(argv[i]);
      regex_search(flag, rank_match, rank_length_regex);
      int rank_len = stoi(rank_match.str());
      if (rank_len < 1)
      {
        std::cerr << "Invalid ranking length." << std::endl;
        return 1;
      }
      ranking_length = rank_len;
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

int UserOptions::run_file_setup()
{
  std::ifstream f("./temp_files");
  if (!f.good())
  {
    system("mkdir temp_files");
  }
  f.close();

  if (file_names.size() == 0)
  {
    std::cout << "No files specified!" << std::endl;
    return 1;
  }

  for (size_t i = 0; i < file_names.size(); ++i)
  {
    std::ifstream f(file_names[i]);
    if (!f.good())
    {
      std::cerr << "The file \"" << file_names[i] << "\" does not exist" << std::endl;
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

  // doesn't need to be fatal
  // use each of these checks separately below

  if (proc.l1d->get_size() <= 0 || proc.l1i->get_size() <= 0)
  {
    std::cout << "It has not been possible to gauge the size of one of the L1 caches. At present, this is a fatal error." << std::endl;
    return 1;
  }

  if (flags.test(NO_EMPIRICAL) && ( proc.l1d->get_assoc() <= 0 ||
                                    proc.l1d->get_critical_stride() <= 0 ||
                                    proc.l1d->get_linesize() <= 0 ||
                                    proc.l1i->get_assoc() <= 0 ||
                                    proc.l1i->get_critical_stride() <= 0 ||
                                    proc.l1i->get_linesize() <= 0 ))
  {
    std::cout << "The following cache dimensions are unknown, but you have chosen not to execute empirical tests:" << std::endl;
    if (proc.l1d->get_assoc() <= 0) { std::cout << "-- L1 data cache associativity" << std::endl; }
    if (proc.l1d->get_linesize() <= 0) { std::cout << "-- L1 data cache linesize" << std::endl; }
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
      std::cout << "The associativity and critical stride of the L1 data cache are both unknown." << std::endl;
      std::cout << "They will now be tested empirically. This may take up to a minute." << std::endl;

      suggested_values[0] = proc.l1d->empirical_assoc_test(flags);
      if (suggested_values[0] <= 0)
      {
        std::cerr << "Error assessing L1 data-cache associativity." << std::endl;
        std::cerr << "Exiting." << std::endl;
        return 1;
      }
      std::cout << "Suggested L1 data-cache associativity: " << suggested_values[0] << std::endl;
      suggested_values[1] = proc.l1d->empirical_stride_test(flags);
      if (suggested_values[1] <= 0)
      {
        std::cerr << "Error assessing L1 data-cache critical stride." << std::endl;
        std::cerr << "Exiting." << std::endl;
        return 1;
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
      std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

      suggested_values[0] = proc.l1d->empirical_assoc_test(flags);
      if (suggested_values[0] <= 0)
      {
        std::cerr << "Error assessing L1 data-cache associativity." << std::endl;
        std::cerr << "Exiting." << std::endl;
        return 1;
      }
      std::cout << "Suggested L1 data-cache associativity: " << suggested_values[0] << std::endl;

      std::cout << std::endl << "Setting L1 data-cache associavtivity to " << suggested_values[0] << std::endl << std::endl;
      proc.l1d->set_assoc(suggested_values[0]);
    }
    else if (proc.l1d->get_critical_stride() <= 0)
    {
      std::cout << "The critical stride of the L1 data cache is unknown." << std::endl;
      std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

      suggested_values[1] = proc.l1d->empirical_stride_test(flags);
      if (suggested_values[1] <= 0)
      {
        std::cerr << "Error assessing L1 data-cache critical stride." << std::endl;
        std::cerr << "Exiting." << std::endl;
        return 1;
      }
      std::cout << "Suggested L1 data-cache critical stride: " << suggested_values[1] << std::endl;

      std::cout << std::endl << "Setting L1 data-cache critical stride to " << suggested_values[1] << std::endl << std::endl;
      proc.l1d->set_critical_stride(suggested_values[1]);
    }
    if (proc.l1i->get_assoc() <= 0 && proc.l1i->get_critical_stride() <= 0)
    {
      std::cout << "The associativity and critical stride of the L1 instruction-cache are both unknown." << std::endl;
      std::cout << "They will now be tested empirically. This may take up to a minute." << std::endl;

      suggested_values[2] = proc.l1i->empirical_assoc_test(flags);
      if (suggested_values[2] <= 0)
      {
        std::cerr << "Error assessing L1 instruction-cache associativity." << std::endl;
        std::cerr << "Exiting." << std::endl;
        return 1;
      }
      std::cout << "Suggested L1 instruction-cache associativity: " << suggested_values[2] << std::endl;
      suggested_values[3] = proc.l1i->empirical_stride_test(flags);
      if (suggested_values[3] <= 0)
      {
        std::cerr << "Error assessing L1 instruction-cache critical stride." << std::endl;
        std::cerr << "Exiting." << std::endl;
        return 1;
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
      std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

      suggested_values[2] = proc.l1i->empirical_assoc_test(flags);
      if (suggested_values[2] <= 0)
      {
        std::cerr << "Error assessing L1 instruction-cache associativity." << std::endl;
        std::cerr << "Exiting." << std::endl;
        return 1;
      }
      std::cout << "Suggested instruction-cache associativity: " << suggested_values[2] << std::endl;

      std::cout << std::endl << "Setting L1 instruction-cache associavtivity to " << suggested_values[2] << std::endl << std::endl;
      proc.l1i->set_assoc(suggested_values[2]);
    }
    else if (proc.l1i->get_critical_stride() <= 0)
    {
      std::cout << "The critical stride of the L1 instruction-cache is unknown." << std::endl;
      std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

      suggested_values[3] = proc.l1i->empirical_stride_test(flags);
      if (suggested_values[3] <= 0)
      {
        std::cerr << "Error assessing L1 instruction-cache critical stride." << std::endl;
        std::cerr << "Exiting." << std::endl;
        return 1;
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
  if (flags.test(SOURCE_CODE_ONLY))
  {
    std::cout << std::endl << "===========================================================" << std::endl << std::endl;
    std::cout << "SOURCE CODE ANALYSIS" << std::endl;
    std::cout << std::endl << "===========================================================" << std::endl << std::endl;
    std::cout << "This tool analyses your source code files to discover cache-inefficient data layouts in user-defined types." << std::endl;
    std::cout << "Any user-defined types which can be optimised by reordering its data members will be reported, along with a suggestion for a new layout." << std::endl << std::endl;
    std::cout << "Additionally, this tool will identify any user-defined types where the Least Common Multiple of the size of the type and the critical stride of your processor is small." << std::endl;
    std::cout << "If objects of these types are stored in contiguous memory (such as an array or a vector), the same members of the class in different objects may compete for space in the cache." << std::endl;
    std::cout << "If access patterns involve accessing the same data members across multiple contiguous objects, this may create latency problems." << std::endl << std::endl;
    std::cout << "Analysing your source code..." << std::endl << std::flush;

    fc.populate_file_names(file_names);

    fc.detect_types();

    fc.suggest_optimisations(proc.l1d->get_critical_stride());
  }
  else
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
    }

    fc.populate_file_names(file_names);

    fc.detect_types();

    bool optimised = fc.suggest_optimisations(proc.l1d->get_critical_stride());

    if (optimised)
    {
      std::cout << std::endl << "===== Suggestions =====" << std::endl << std::endl;
      std::cout << "It is recommended, unless there is a specific reason to retain an inefficient ordering, that any reorderings identified above are implemented." << std::endl;
      std::cout << "If the overall size of a type may cause problems in the context of access patterns and the critical stride of the data cache, attempting to make the data structure smaller (perhaps by using different data types as sub-members or by using highly compact types like bitfields) is the ideal solution." << std::endl;
      std::cout << "Failing that, it may (counterintuitively) be worth trying to make the object slightly larger to reduce the chance of desired accesses evicting useful data from the cache by increasing the Least Common Multiple of the object size and the critical stride." << std::endl << std::endl;
    }
    else
    {
      std::cout << "No data ordering or type-size inefficiencies have been detected!" << std::endl;
    }
    
    std::cout << std::endl << "===========================================================" << std::endl << std::endl;
    std::cout << "BINARY ANALYSIS" << std::endl;
    std::cout << std::endl << "===========================================================" << std::endl << std::endl;
    std::cout << "Groups of functions that call each other and compete for the same portion of the cache may cause latency problems." << std::endl;
    std::cout << "If the number of functions in the group is above the associativity of the instruction cache ("<< proc.l1d->get_assoc() << " on this processor), these functions will definitely evict each other from the cache." << std::endl;
    std::cout << "If the number of functions in the group is equal to or slighly below the associativity of the instruction cache, there is a significant probability that the functions will evict each other from the cache. The uncertainty (standard deviation) of this probability can also be a source of jitter at runtime." << std::endl;
    std::cout << "The larger the region of cache memory that the functions compete for, the more code needs to be fetched from lower level caches the next time the evicted function needs to be executed, and the more significant the latency penalties are." << std::endl << std::endl;
    std::cout << "Analysing your binary..." << std::endl << std::endl;

    Binary* bin = new Binary(file_names[file_names.size() - 1], fc.get_user_types());
    int bin_ret = 0;
    bin_ret = bin->get_functions(*this);
    if (bin_ret != 0)
    {
      std::cout << std::endl << "Error reading functions from " << file_names[file_names.size() - 1] << "." << std::endl;
      std::cout << "Exiting." << std::endl;
      return 1;
    }
    bin_ret = bin->populate_competition_vectors(*this);
    if (bin_ret != 0)
    {
      std::cout << std::endl << "Error populating competition vectors from " << file_names[file_names.size() - 1] << "." << std::endl;
      std::cout << "Exiting." << std::endl;
      return 1;
    }
    bin_ret = bin->populate_coexecution_vectors(*this);
    if (bin_ret != 0)
    {
      std::cout << std::endl << "Error populating coexecution vectors from " << file_names[file_names.size() - 1] << "." << std::endl;
      std::cout << "Exiting." << std::endl;
      return 1;
    }
    bin_ret = bin->find_problem_function_groups(*this);
    if (bin_ret != 0)
    {
      std::cout << std::endl << "Error finding problem function groups." << std::endl;
      std::cout << "Exiting." << std::endl;
      return 1;
    }
    delete bin;
    std::cout << std::endl;
  }
  return 0;
}

UserOptions::~UserOptions()
{
  if (!flags.test(2))
  {
    std::string cmd_rm_tmp = "rm ./temp_files/*";
    system(cmd_rm_tmp.c_str());
  }
}