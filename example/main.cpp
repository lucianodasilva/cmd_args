#include <cmd_args.h>

#include <string>
#include <vector>
#include <iostream>

using namespace std;
//using namespace cmd_args;

struct demo {
    
    bool show_version;
    bool show_help;

	string dude_name;
    
    vector < string > source_files;
    
};

void show_help (demo & d) {
    std::cout << "demo show help" << std::endl;
}

void show_version(demo & d) {
    std::cout << "demo show version" << std::endl;
}

void process (demo & d) {
    std::cout << "demo process files" << std::endl;
}

int main(int arg_c, char * arg_v[]) {

	using namespace command_line;

	//const char * args[] = { "garbage", "ficheiro1", "ficheiro2", "ficheiro3" };
	//const char * args[] = { "garbage", "file1", "file2", "file3" };

	const char * args[] = { "yada", "-v" };
	
	auto expression = key("-x") > key("-Y") > (*key("-z"));

	auto cmd_expr =
		usage( option("-v", "--version") )[&show_version] |
		usage( option("-h", "--help") )[&show_help] |
		usage( (+any)[&demo::source_files] ) [&process];
	;
	
	parse < demo > (cmd_expr, 4, args);


	
	//auto val = command_line::value(&demo::xxx, .0F);

	/*
	// define an instance of the command line parser
	parser < arguments_t > command_line;

	// set its available arguments and options
	command_line
		// option ( < member field address >, < list of command optiones >, < description > )
		+ option(&arguments_t::show_version, { "-v", "--version" }, "shows the version of the application")
		+ option(&arguments_t::show_usage, { "-h, --help" }, "shows this usage page")
		// argument ( < member field address >, < list of command optiones >, < description >, < is optional >, < default value > )
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
	*/
	return 0;
}
