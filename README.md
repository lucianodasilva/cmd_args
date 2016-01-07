MASTER: [![Build Status](https://travis-ci.org/lucianodasilva/cmd_args.svg?branch=master)](https://travis-ci.org/lucianodasilva/cmd_args) Dev: [![Build Status](https://travis-ci.org/lucianodasilva/cmd_args.svg?branch=dev)](https://travis-ci.org/lucianodasilva/cmd_args)

# cmd_args
Simple single header command line arguments parser:

# Getting Started

Just define a structure to hold the arguments you wish to be available
```c++
struct arg_t {
  bool show_version;
  bool show_usage
};
``` 

Declare the available switches to the parser in the parser:
```c++
parser < arg_t > command_parser;

command_parser
  + option (&arg_t::show_version, {"-v"}, "show version")
  + option (&arg_t::show_usage, {"-h", "--help"}, "show this usage page");
```

And parse the command line arguments from main:
```c++
arg_t args_inst;
command_parser.parse (arg_c, arg_v, args_inst);
```

Please check the example file provided for further details and a running example of usage.
