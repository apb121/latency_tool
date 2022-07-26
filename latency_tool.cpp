#include <unistd.h>
#include <iostream>
#include <bit>
#include <bitset>

#define SOURCE_CODE_ONLY 7
#define BINARY_ONLY 6
#define CACHE_INFO_ONLY 5
#define MANUAL_CACHE 4
#define NO_EMPIRICAL 3
#define KEEP_TEMP 2
#define EXISTING_TEMP_FILES 1

#include "cache_info.hpp"
#include "class_parser.hpp"
#include "user_options.hpp"
#include "binary_analyser.hpp"

/*
    normal usage:

            ./latency_tool <binary_file> <source_files>

    flags:
            -s, --source-code-only {7}
            -b, --binary-only {6} (?) (make sure to look at all functions with binary only)
            -c, --cache-info-only {5}
            -m=..., --manual-cache=<size>:<associativity>:<linesize>::... {4}
            -n, --no-empirical-tests (default critical stride = 4096?, default assocativity = 8?) {3}
            -k, --keep-temporary-files (make sure to add checks for if files exist before deleting them) {2}
            -l= --coexecution-level= (default = ? maybe 1)
            -t= --competition-threshold= (default = ? maybe 64, or maybe 16)
            -r= --ranking-length= (default = 10)
            -e, --use-existing-temp-files (automatically switches on -k flag) {1}
*/

void cleanup_temp_files(UserOptions& uo)
{
  if (!uo.flags.test(2))
  {
    std::string cmd_rm_tmp = "rm ./temp_files/*";
    system(cmd_rm_tmp.c_str());
  }
}

int main(int argc, char** argv)
{
    using namespace std;
    UserOptions uo;
    int parse_ret = uo.parse_flags(argc, argv);
    if (parse_ret) { return 1; }

    std::ifstream f("./temp_files");
    if (!f.good())
    {
      system("mkdir temp_files");
    }
    f.close();

    if (uo.file_names.size() == 0)
    {
      std::cout << "No files specified!" << std::endl;
      return 1;
    }

    for (size_t i = 0; i < uo.file_names.size(); ++i)
    {
      std::ifstream f(uo.file_names[i]);
      if (!f.good())
      {
        std::cerr << "The file \"" << uo.file_names[i] << "\" does not exist" << std::endl;
        return 1;
      }
    }

    // changes these bitflag integers to # define s

    if (!uo.flags.test(MANUAL_CACHE))
    {
      std::cout << std::endl << "===========================================================" << std::endl << std::endl;
      std::cout << "CACHE ANALYSIS" << std::endl;
      std::cout << std::endl << "===========================================================" << std::endl << std::endl;
      uo.proc.l1d = new L1d_cache(
        sysconf(_SC_LEVEL1_DCACHE_SIZE),
        sysconf(_SC_LEVEL1_DCACHE_LINESIZE),
        sysconf(_SC_LEVEL1_DCACHE_ASSOC)
      );
      uo.proc.l1i = new L1i_cache(
        sysconf(_SC_LEVEL1_ICACHE_SIZE),
        sysconf(_SC_LEVEL1_ICACHE_LINESIZE),
        sysconf(_SC_LEVEL1_ICACHE_ASSOC)
      );
      uo.proc.l2 = new L2_cache(
        sysconf(_SC_LEVEL2_CACHE_SIZE),
        sysconf(_SC_LEVEL2_CACHE_LINESIZE),
        sysconf(_SC_LEVEL2_CACHE_ASSOC)
      );
      uo.proc.l3 = new L3_cache(
        sysconf(_SC_LEVEL3_CACHE_SIZE),
        sysconf(_SC_LEVEL3_CACHE_LINESIZE),
        sysconf(_SC_LEVEL3_CACHE_ASSOC)
      );
      uo.proc.l4 = new L4_cache(
        sysconf(_SC_LEVEL4_CACHE_SIZE),
        sysconf(_SC_LEVEL4_CACHE_LINESIZE),
        sysconf(_SC_LEVEL4_CACHE_ASSOC)
      );
    }

    if (uo.proc.l1d->get_size() <= 0 || uo.proc.l1i->get_size() <= 0)
    {
      std::cout << "It has not been possible to gauge the size of one of the L1 caches. At present, this is a fatal error." << std::endl;
      return 1;
    }

    if (  uo.flags.test(NO_EMPIRICAL) &&
          (
            uo.proc.l1d->get_assoc() <= 0 ||
            uo.proc.l1d->get_critical_stride() <= 0 ||
            uo.proc.l1d->get_linesize() <= 0 ||
            uo.proc.l1i->get_assoc() <= 0 ||
            uo.proc.l1i->get_critical_stride() <= 0 ||
            uo.proc.l1i->get_linesize() <= 0
          )
      )
    {
      std::cout << "The following cache dimensions are unknown, but you have chosen not to execute empirical tests:" << std::endl;
      if (uo.proc.l1d->get_assoc() <= 0) { std::cout << "-- L1 data cache associativity" << std::endl; }
      if (uo.proc.l1d->get_linesize() <= 0) { std::cout << "-- L1 data cache linesize" << std::endl; }
      if (uo.proc.l1i->get_assoc() <= 0) { std::cout << "-- L1 instruction cache associativity" << std::endl; }
      if (uo.proc.l1i->get_linesize() <= 0) { std::cout << "-- L1 instruction cache linesize" << std::endl; }
      std::cout << "Associativities will default to 8, linesizes to 64, and critical strides to 4096.";
      std::cout << " This can produce results that are inaccurate for your processor, but the suggestions may still prove useful." << std::endl;
      if (uo.proc.l1d->get_assoc() <= 0) { uo.proc.l1d->set_assoc(8); }
      if (uo.proc.l1i->get_assoc() <= 0) { uo.proc.l1i->set_assoc(8); }
      if (uo.proc.l1d->get_linesize() <= 0) { uo.proc.l1d->set_linesize(64); }
      if (uo.proc.l1i->get_linesize() <= 0) { uo.proc.l1i->set_linesize(64); }
      if (uo.proc.l1d->get_critical_stride() <= 0) { uo.proc.l1d->set_critical_stride(4096); }
      if (uo.proc.l1i->get_critical_stride() <= 0) { uo.proc.l1i->set_critical_stride(4096); }
    }
    else if (!(uo.flags.test(NO_EMPIRICAL)))
    {
      int suggested_values[4] = {0, 0, 0, 0};
      if (uo.proc.l1d->get_assoc() <= 0 && uo.proc.l1d->get_critical_stride() <= 0)
      {
        std::cout << "The associativity and critical stride of the L1 data cache are both unknown." << std::endl;
        std::cout << "They will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[0] = uo.proc.l1d->empirical_assoc_test(uo.flags);
        if (suggested_values[0] <= 0)
        {
          std::cerr << "Error assessing L1 data-cache associativity." << std::endl;
          std::cerr << "Exiting." << std::endl;
          cleanup_temp_files(uo);
          return 1;
        }
        std::cout << "Suggested L1 data-cache associativity: " << suggested_values[0] << std::endl;
        suggested_values[1] = uo.proc.l1d->empirical_stride_test(uo.flags);
        if (suggested_values[1] <= 0)
        {
          std::cerr << "Error assessing L1 data-cache critical stride." << std::endl;
          std::cerr << "Exiting." << std::endl;
          cleanup_temp_files(uo);
          return 1;
        }
        std::cout << "Suggested L1 data-cache critical stride: " << suggested_values[1] << std::endl;

        std::cout << std::endl << "Setting L1 data-cache associativity to " << suggested_values[0] << std::endl;
        uo.proc.l1d->set_assoc(suggested_values[0]);
        int pos_critical_stride = uo.proc.l1d->get_size() / uo.proc.l1d->get_assoc();

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
          std::cout << std::endl << "Setting L1 data-cache critical stride to " << min(pos_critical_stride, suggested_values[1]) << std::endl << std::endl;
          uo.proc.l1d->set_critical_stride(min(pos_critical_stride, suggested_values[1]));
        }
      }
      else if (uo.proc.l1d->get_assoc() <= 0)
      {
        std::cout << "The associativity of the L1 data cache is unknown." << std::endl;
        std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[0] = uo.proc.l1d->empirical_assoc_test(uo.flags);
        if (suggested_values[0] <= 0)
        {
          std::cerr << "Error assessing L1 data-cache associativity." << std::endl;
          std::cerr << "Exiting." << std::endl;
          cleanup_temp_files(uo);
          return 1;
        }
        std::cout << "Suggested L1 data-cache associativity: " << suggested_values[0] << std::endl;

        std::cout << std::endl << "Setting L1 data-cache associavtivity to " << suggested_values[0] << std::endl << std::endl;
        uo.proc.l1d->set_assoc(suggested_values[0]);
      }
      else if (uo.proc.l1d->get_critical_stride() <= 0)
      {
        std::cout << "The critical stride of the L1 data cache is unknown." << std::endl;
        std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[1] = uo.proc.l1d->empirical_stride_test(uo.flags);
        if (suggested_values[1] <= 0)
        {
          std::cerr << "Error assessing L1 data-cache critical stride." << std::endl;
          std::cerr << "Exiting." << std::endl;
          cleanup_temp_files(uo);
          return 1;
        }
        std::cout << "Suggested L1 data-cache critical stride: " << suggested_values[1] << std::endl;

        std::cout << std::endl << "Setting L1 data-cache critical stride to " << suggested_values[1] << std::endl << std::endl;
        uo.proc.l1d->set_critical_stride(suggested_values[1]);
      }
      if (uo.proc.l1i->get_assoc() <= 0 && uo.proc.l1i->get_critical_stride() <= 0)
      {
        std::cout << "The associativity and critical stride of the L1 instruction-cache are both unknown." << std::endl;
        std::cout << "They will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[2] = uo.proc.l1i->empirical_assoc_test(uo.flags);
        if (suggested_values[2] <= 0)
        {
          std::cerr << "Error assessing L1 instruction-cache associativity." << std::endl;
          std::cerr << "Exiting." << std::endl;
          cleanup_temp_files(uo);
          return 1;
        }
        std::cout << "Suggested L1 instruction-cache associativity: " << suggested_values[2] << std::endl;
        suggested_values[3] = uo.proc.l1i->empirical_stride_test(uo.flags);
        if (suggested_values[3] <= 0)
        {
          std::cerr << "Error assessing L1 instruction-cache critical stride." << std::endl;
          std::cerr << "Exiting." << std::endl;
          cleanup_temp_files(uo);
          return 1;
        }
        std::cout << "Suggested L1 instruction-cache critical stride: " << suggested_values[3] << std::endl;

        std::cout << std::endl << "Setting L1 instruction-cache associativity to " << suggested_values[2] << std::endl;
        uo.proc.l1i->set_assoc(suggested_values[2]);
        int pos_critical_stride = uo.proc.l1i->get_size() / uo.proc.l1i->get_assoc();

        if (  ((pos_critical_stride) & (pos_critical_stride - 1) == 0) && pos_critical_stride != 0 &&
              ((suggested_values[3]) & (suggested_values[3] - 1) != 0))
        {
          std::cout << std::endl << "Setting L1 instruction-cache critical stride to " << pos_critical_stride << std::endl << std::endl;  
          uo.proc.l1i->set_critical_stride(pos_critical_stride);
        }
        else if ( ((pos_critical_stride) & (pos_critical_stride - 1) != 0) &&
                  ((suggested_values[3]) & (suggested_values[3] - 1) == 0) && suggested_values[3] != 0)
        {
          std::cout << std::endl << "Setting L1 instruction-cache critical stride to " << suggested_values[3] << std::endl << std::endl;
          uo.proc.l1i->set_critical_stride(suggested_values[3]);
        }
        else
        {
          std::cout << std::endl << "Setting L1 instruction-cache critical stride to " << min(pos_critical_stride, suggested_values[3]) << std::endl << std::endl;
          uo.proc.l1i->set_critical_stride(min(pos_critical_stride, suggested_values[3]));
        }
      }
      else if (uo.proc.l1i->get_assoc() <= 0)
      {
        std::cout << "The associativity of the L1 instruction-cache is unknown." << std::endl;
        std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[2] = uo.proc.l1i->empirical_assoc_test(uo.flags);
        if (suggested_values[2] <= 0)
        {
          std::cerr << "Error assessing L1 instruction-cache associativity." << std::endl;
          std::cerr << "Exiting." << std::endl;
          cleanup_temp_files(uo);
          return 1;
        }
        std::cout << "Suggested instruction-cache associativity: " << suggested_values[2] << std::endl;

        std::cout << std::endl << "Setting L1 instruction-cache associavtivity to " << suggested_values[2] << std::endl << std::endl;
        uo.proc.l1i->set_assoc(suggested_values[2]);
      }
      else if (uo.proc.l1i->get_critical_stride() <= 0)
      {
        std::cout << "The critical stride of the L1 instruction-cache is unknown." << std::endl;
        std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[3] = uo.proc.l1i->empirical_stride_test(uo.flags);
        if (suggested_values[3] <= 0)
        {
          std::cerr << "Error assessing L1 instruction-cache critical stride." << std::endl;
          std::cerr << "Exiting." << std::endl;
          cleanup_temp_files(uo);
          return 1;
        }
        std::cout << "Suggested instruction-cache critical stride: " << suggested_values[3] << std::endl;

        std::cout << std::endl << "Setting L1 instruction-cache critical stride to " << suggested_values[3] << std::endl << std::endl;
        uo.proc.l1i->set_assoc(suggested_values[3]);
      }
    }

    if (uo.flags.test(SOURCE_CODE_ONLY))
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

      for (size_t i = 0; i < uo.file_names.size(); ++i)
      {
        uo.files.push_back(uo.file_names[i]);
      }
      for (size_t i = 0; i < uo.files.size(); ++i)
      {
        vector<UDType> types = uo.files[i].detect_types();
        for (size_t j = 0; j < types.size(); ++j)
        {
          uo.udtype_sizes[types[j].get_name()] = 0;
        }
        uo.all_user_types.insert(uo.all_user_types.end(), types.begin(), types.end());
      }
      
      for (size_t i = 0; i < uo.all_user_types.size(); ++i)
      {
        if (uo.all_user_types[i].is_child == true)
        {
          for (size_t j = 0; j < uo.all_user_types.size(); ++j)
          {
            if (j == i) { continue; }
            if (uo.all_user_types[i].get_parent_name() == uo.all_user_types[j].get_name())
            {
              uo.all_user_types[i].parent_class = &(uo.all_user_types[j]);
            }
          }
        }
      }

      int num_unknown = 0, prev_unknown = 1;
      while (num_unknown != prev_unknown)
      {
        prev_unknown = num_unknown;
        num_unknown = 0;
        for (size_t i = 0; i < uo.all_user_types.size(); ++i)
        {
          int size = uo.all_user_types[i].calculate_size(uo.udtype_sizes);
          if (size <= 0)
          {
            ++num_unknown;
          }
          else
          {
            uo.udtype_sizes[uo.all_user_types[i].get_name()] = size;
          }
        }
      }
      for (size_t i = 0; i < uo.all_user_types.size(); ++i)
      {
        std::cout << "Analysing class: " << uo.all_user_types[i].get_name() << std::endl;
        uo.all_user_types[i].suggest_optimisations(uo.udtype_sizes, uo.proc.l1d->get_critical_stride());
      }
    }
    else
    {
      if (uo.file_names.size() > 1)
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
      for (size_t i = 0; i < uo.file_names.size(); ++i)
      {
        uo.files.push_back(uo.file_names[i]);
      }
      for (size_t i = 0; i < uo.files.size(); ++i)
      {
        vector<UDType> types = uo.files[i].detect_types();
        for (size_t j = 0; j < types.size(); ++j)
        {
          uo.udtype_sizes[types[j].get_name()] = 0;
        }
        uo.all_user_types.insert(uo.all_user_types.end(), types.begin(), types.end());
      }
      
      for (size_t i = 0; i < uo.all_user_types.size(); ++i)
      {
        if (uo.all_user_types[i].is_child == true)
        {
          for (size_t j = 0; j < uo.all_user_types.size(); ++j)
          {
            if (j == i) { continue; }
            if (uo.all_user_types[i].get_parent_name() == uo.all_user_types[j].get_name())
            {
              uo.all_user_types[i].parent_class = &(uo.all_user_types[j]);
            }
          }
        }
      }

      int num_unknown = 0, prev_unknown = 1;
      while (num_unknown != prev_unknown)
      {
        prev_unknown = num_unknown;
        num_unknown = 0;
        for (size_t i = 0; i < uo.all_user_types.size(); ++i)
        {
          int size = uo.all_user_types[i].calculate_size(uo.udtype_sizes);
          if (size <= 0)
          {
            ++num_unknown;
          }
          else
          {
            uo.udtype_sizes[uo.all_user_types[i].get_name()] = size;
          }
        }
      }

      bool optimised = false;
      for (size_t i = 0; i < uo.all_user_types.size(); ++i)
      {
        if (uo.all_user_types[i].suggest_optimisations(uo.udtype_sizes, uo.proc.l1d->get_critical_stride()))
        {
          optimised = true;
        }
      }

      if (optimised)
      {
        std::cout << std::endl << "===== Suggestions =====" << std::endl << std::endl;
        std::cout << "It is recommended, unless there is a specific reason to retain an inefficient ordering, that any reorderings identified above are implemented." << std::endl;
        std::cout << "If the overall size of a type may cause problems in the context of access patterns and the critical stride of the data cache, attempting to make the data structure smaller (perhaps by using different data types as sub-members or by using highly compact types like bitfields) is the ideal solution." << std::endl;
        std::cout << "Failing that, it may (counterintuitively) be worth trying to make the object slightly larger to reduce the chance of desired accesses evicting useful data from the cache by increasing the Least Common Multiple of the object size and the critical stride." << std::endl << std::endl;
      }
      
      std::cout << std::endl << "===========================================================" << std::endl << std::endl;
      std::cout << "BINARY ANALYSIS" << std::endl;
      std::cout << std::endl << "===========================================================" << std::endl << std::endl;
      std::cout << "Groups of functions that call each other and compete for the same portion of the cache (such as those identified above) may cause latency problems." << std::endl;
      std::cout << "If the number of functions in the group is above the associativity of the instruction cache ("<< uo.proc.l1d->get_assoc() << " on this processor), these functions will definitely evict each other from the cache." << std::endl;
      std::cout << "If the number of functions in the group is equal to or slighly below the associativity of the instruction cache, there is a significant probability that the functions will evict each other from the cache. The uncertainty (standard deviation) of this probability can also be a source of jitter at runtime." << std::endl;
      std::cout << "The larger the region of cache memory that the functions compete for, the more code needs to be fetched from lower level caches the next time the evicted function needs to be executed, and the more significant the latency penalties are." << std::endl << std::endl;
      std::cout << "Analysing your binary..." << std::endl << std::endl;

      Binary* bin = new Binary(uo.file_names[uo.file_names.size() - 1], uo.all_user_types);
      int bin_ret = 0;
      bin_ret = bin->get_functions(uo);
      if (bin_ret != 0)
      {
        std::cout << std::endl << "Error reading functions from " << uo.file_names[uo.file_names.size() - 1] << "." << std::endl;
        std::cout << "Exiting." << std::endl;
        cleanup_temp_files(uo);
        return 1;
      }
      bin_ret = bin->populate_competition_vectors(uo);
      if (bin_ret != 0)
      {
        std::cout << std::endl << "Error populating competition vectors from " << uo.file_names[uo.file_names.size() - 1] << "." << std::endl;
        std::cout << "Exiting." << std::endl;
        cleanup_temp_files(uo);
        return 1;
      }
      bin_ret = bin->populate_coexecution_vectors(uo);
      if (bin_ret != 0)
      {
        std::cout << std::endl << "Error populating coexecution vectors from " << uo.file_names[uo.file_names.size() - 1] << "." << std::endl;
        std::cout << "Exiting." << std::endl;
        cleanup_temp_files(uo);
        return 1;
      }
      bin_ret = bin->find_problem_function_groups(uo);
      if (bin_ret != 0)
      {
        std::cout << std::endl << "Error finding problem function groups." << std::endl;
        std::cout << "Exiting." << std::endl;
        cleanup_temp_files(uo);
        return 1;
      }
      delete bin;
      std::cout << std::endl;
    }
    cleanup_temp_files(uo);
    return 0;
}