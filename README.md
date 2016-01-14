# cmd_args
Simple single header command line arguments parser:

Stable Branch (master): 
[![Build Status](https://travis-ci.org/lucianodasilva/cmd_args.svg?branch=master)](https://travis-ci.org/lucianodasilva/cmd_args) 

Development Branch (dev): [![Build Status](https://travis-ci.org/lucianodasilva/cmd_args.svg?branch=dev)](https://travis-ci.org/lucianodasilva/cmd_args)

# Getting Started

Define a structure to hold the arguments you wish to be available
```c++
struct arg_t {
  bool show_version;
  bool show_usage;
  vector < string > files;
};
``` 

Declare the grammar for the command line usages you wish to be made available using the following operators:

 "-" - zero or one of
 "+" - one or more of
 "*" - zero or more of
 ">" - sequence
 "|" - alternation
 
And the following scanners:
 
 "any" - any content
 "option" - represents a boolean. Enabled if the option is found
 "key" - represents a value preceded by a key ( --output="file.x" )
 
Use them toggether in "usages" to build expressions and trigger callbacks given successfully parsed expressions:

auto cmd_line_exp = usage ( option ("-h", "--help") ) [&help_callback] |
                    usage ( option ("-v", "--version") ) [&version_callback] |
                    usage ( (*any)[&arg_t::files] ) [&process_files];

Please check the example file provided for further details and a running example of usage.
