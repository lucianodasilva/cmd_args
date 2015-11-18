#include "cmd_args.h"
#include <iostream>

using namespace cmd_args;

// structure type that represents the 
// information to be read
struct arguments_t {
	bool show_version;
	bool show_usage;
	int value_a;
	int value_b;
};

int main (int arg_c, char * arg_v []) {

	// define an instance of the command line parser
	parser < arguments_t > command_line;

	// set its available arguments and options
	command_line
		// option ( < member field address >, < list of command switches >, < description > )
		+ option(&arguments_t::show_version, { "-v", "--version" }, "shows the version of the application")
		+ option(&arguments_t::show_usage, { "-h, --help" }, "shows this usage page")
		// argument ( < member field address >, < list of command switches >, < description >, < is optional >, < default value > )
		+ argument(&arguments_t::value_a, { "-va", "--value_a" }, "multiplicand")
		+ argument(&arguments_t::value_b, { "-vb", "--value_b" }, "multiplier");

	arguments_t args;

	// command line parser returs a context with information about
	// the success of the parsing operation and related errors
	auto context = command_line.parse(arg_c, arg_v, args);

	// if the parsing failed
	if (context.has_failed()) {
		cout << "example failed with the following invalid arguments" << endl;
		// the user can print all errors to an ostream
		context.to_stream(cout);
		return -1;
	}

	if (args.show_version)
		cout << "example 1.0" << endl;

	// users can also print a list of options and arguments with their descriptions
	// to an ostream
	if (args.show_usage)
		command_line.usage_to_stream(cout);

	cout << "example value: " << (args.value_a * args.value_b) << endl;

	return 0;
}