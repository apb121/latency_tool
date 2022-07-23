#include <unistd.h>
#include <iostream>

#define SOURCE_CODE_ONLY 7
#define BINARY_ONLY 6
#define CACHE_INFO_ONLY 5
#define MANUAL_CACHE 4
#define NO_EMPIRICAL 3
#define KEEP_TEMP 2

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
*/

int main(int argc, char** argv)
{
    using namespace std;
    UserOptions uo;
    int parse_ret = uo.parse_flags(argc, argv);
    if (parse_ret) { return 1; }

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
      std::cout << "Reading cache data from your system..." << std::endl << std::endl;
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
      std::cout << "It has not been possible to gauge the size of one of the L1 caches. At present this is a fatal error." << std::endl;
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

        suggested_values[0] = uo.proc.l1d->empirical_assoc_test();
        std::cout << "Suggested L1 data-cache associativity: " << suggested_values[0] << std::endl;
        suggested_values[1] = uo.proc.l1d->empirical_stride_test();
        std::cout << "Suggested L1 data-cache critical stride: " << suggested_values[1] << std::endl;

        std::cout << "Setting L1 data-cache associativity to " << suggested_values[0] << std::endl;
        uo.proc.l1d->set_assoc(suggested_values[0]);
        int pos_critical_stride = uo.proc.l1d->get_size() / uo.proc.l1d->get_assoc();
        std::cout << "Setting L1 data-cache critical stride to " << min(pos_critical_stride, suggested_values[1]) << std::endl;
        uo.proc.l1d->set_critical_stride(min(pos_critical_stride, suggested_values[1]));
      }
      else if (uo.proc.l1d->get_assoc() <= 0)
      {
        std::cout << "The associativity of the L1 data cache is unknown." << std::endl;
        std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[0] = uo.proc.l1d->empirical_assoc_test();
        std::cout << "Suggested L1 data-cache associativity: " << suggested_values[0] << std::endl;

        std::cout << "Setting L1 data-cache associavtivity to " << suggested_values[0] << std::endl;
        uo.proc.l1d->set_assoc(suggested_values[0]);
      }
      else if (uo.proc.l1d->get_critical_stride() <= 0)
      {
        std::cout << "The critical stride of the L1 data cache is unknown." << std::endl;
        std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[1] = uo.proc.l1d->empirical_stride_test();
        std::cout << "Suggested L1 data-cache critical stride: " << suggested_values[1] << std::endl;

        std::cout << "Setting L1 data-cache critical stride to " << suggested_values[1] << std::endl;
        uo.proc.l1d->set_critical_stride(suggested_values[1]);
      }
      if (uo.proc.l1i->get_assoc() <= 0 && uo.proc.l1i->get_critical_stride() <= 0)
      {
        std::cout << "The associativity and critical stride of the L1 instruction-cache are both unknown." << std::endl;
        std::cout << "They will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[2] = uo.proc.l1i->empirical_assoc_test();
        std::cout << "Suggested L1 instruction-cache associativity: " << suggested_values[2] << std::endl;
        suggested_values[3] = uo.proc.l1i->empirical_stride_test();
        std::cout << "Suggested L1 instruction-cache critical stride: " << suggested_values[3] << std::endl;

        std::cout << "Setting L1 instruction-cache associativity to " << suggested_values[2] << std::endl;
        uo.proc.l1i->set_assoc(suggested_values[2]);
        int pos_critical_stride = uo.proc.l1i->get_size() / uo.proc.l1i->get_assoc();
        std::cout << "Setting L1 instruction-cache critical stride to " << min(pos_critical_stride, suggested_values[3]) << std::endl;
        uo.proc.l1d->set_critical_stride(min(pos_critical_stride, suggested_values[3]));
      }
      else if (uo.proc.l1i->get_assoc() <= 0)
      {
        std::cout << "The associativity of the L1 instruction-cache is unknown." << std::endl;
        std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[2] = uo.proc.l1i->empirical_assoc_test();
        std::cout << "Suggested instruction-cache associativity: " << suggested_values[2] << std::endl;
      }
      else if (uo.proc.l1i->get_critical_stride() <= 0)
      {
        std::cout << "The critical stride of the L1 instruction-cache is unknown." << std::endl;
        std::cout << "It will now be tested empirically. This may take up to a minute." << std::endl;

        suggested_values[3] = uo.proc.l1i->empirical_stride_test();
        std::cout << "Suggested instruction-cache critical stride: " << suggested_values[3] << std::endl;
      }
    }

    vector<File>* files = nullptr;
    Binary* bin = nullptr;

    if (uo.flags.test(SOURCE_CODE_ONLY))
    {
      std::cout << std::endl << "===========================================================" << std::endl << std::endl;
      std::cout << "Analysing your source code..." << std::endl << std::endl;
      files = new vector<File>;
      for (size_t i = 0; i < uo.file_names.size(); ++i)
      {
        files->push_back(uo.file_names[i]);
      }
      for (size_t i = 0; i < files->size(); ++i)
      {
        (*files)[i].detect_types();
        (*files)[i].suggest_optimised_orderings();
      }
    }
    else
    {
      files = new vector<File>;
      if (uo.file_names.size() > 1)
      {
        std::cout << std::endl << "===========================================================" << std::endl << std::endl;
        std::cout << "Analysing your source code..." << std::endl << std::endl;
      }
      for (size_t i = 0; i < uo.file_names.size() - 1; ++i)
      {
        files->push_back(uo.file_names[i]);
      }
      vector<UDType> all_types;
      for (size_t i = 0; i < files->size(); ++i)
      {
        (*files)[i].detect_types();
        (*files)[i].suggest_optimised_orderings();
        vector<UDType> types = (*files)[i].get_types();
        all_types.insert(all_types.end(), types.begin(), types.end());
      }
      if (uo.file_names.size() > 1)
      {
        std::cout << std::endl;
      }
      std::cout << std::endl << "===========================================================" << std::endl << std::endl;
      std::cout << "Analysing your binary..." << std::endl << std::endl;
      bin = new Binary(uo.file_names[uo.file_names.size() - 1], all_types);
      bin->get_functions(uo);
      bin->populate_competition_vectors(uo);
      bin->populate_coexecution_vectors(uo);
      bin->find_problem_function_groups(uo);
    }
    
    return 0;
}
