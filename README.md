# Overview

This tool identifies potential sources of L1-data and -instruction cache inefficiency.

To use the tool, invoke ./latency_tool with a list of options and filepaths as command line arguments:

    ./latency_tool [--options ...] [source_code_files ...] [binary_file]

The last file listed will be interpreted as a binary (which must be compiled with debugging symbols, e.g., the gcc flag -g), and any other files listed will be interpreted as the C++ source code files from which the binary was compiled.

Unless guided to to otherwise (see the options available below), the tool will use your processor's cache info to identify:
- inefficiently ordered user-defined classes and structs
- inefficiently sized user-defined classes and structs
- inefficiently sized functions
- groups of functions which both coexecute and compete for cache space

## Options available to the user

### -c/-\-cache-info-only

With this option, the tool will analyse your processor's caches and print out information about their sizes, linesizes, associativities, and critical stride. If the system does not provide these values directly, they will be analysed using empirical tests (which may take a couple of minutes). This option is incompatible with the -m/-manual-cache flag.

### -m=/-\-manual-cache=\<size\>:\<associativity\>:\<linesize\>::\<size\>:\<associativity\>:\<linesize\>:: ...

With this option, the tool will run using the cache dimensions you specify manually. The caches should be specified in the order (1) L1 data (2) L1 instruction (3) L2 (4) L3 (5) L4. Any missing caches will be ignored. You can also choose "--manual-cache=default", which will set the L1 data and instruction caches each to 32768 bytes large, with 8-way associativities and 64-byte linesizes. The manual cache option is incompatible with the -c/--cache-info-only flag.

### -n/-\-no-empirical-tests

With this option, the tool will not run any empirical tests of cache dimensions. If the system does not provide any necessary values, associativities will default to 8, linesizes to 64, and critical strides to 4096.

### -l=/-\-coexecution-level=\<level\>

With this option, the user can decide how many levels of coexecution indirection constitutes coexecution. For example, given functions A(), B(), C(), and D() where A calls B, B calls C, and C calls D, A and B would always be considered to coexecute, A and C would be considered to coexecute only with at least 1 level of indirection, and A and D would be considered to coexecute only with at least 2 levels of indirection. High levels of coexecution indirection may cause long analysis times.

### -t=/-\-competition-threshold=\<threshold\>

With this option, the user can decide the threshold of overlap in cache space that constitutes competition. For example, with a threshold of 200 bytes, any two functions that only overlap in cache space by less than 200 bytes would not be considered as competing with each other. Low competition thresholds may cause long analysis times.

### -r=/-\-ranking-length=\<length\>

With this option, the user can decide how many of the most problematic groups of functions are printed after analysis.

### -a/-\-all-functions

With this option, the tool will include all functions found in the binary in its analysis, as opposed to the default behaviour of just using user-defined functions and functions in user-defined types. If the binary is not very small or the competition threshold is low, this can cause (extremely) long analysis times.

### -k/-\-keep-temporary-files

With this option, the tool will not automatically delete the temp files it used for its analysis when it terminates.

### -e/-\-use-existing-cache-temp-files

With this option, the tool will attempt to find previous empirical cache test temp files and use the results from those rather than run the tests again.

## Setup

To set up the tool, simply:

1. clone this repository
2. cd into it
3. run 'make'