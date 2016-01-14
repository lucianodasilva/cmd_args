#include <cmd_args.h>

#include <string>
#include <vector>
#include <iostream>

using namespace std;
//using namespace cmd_args;

struct demo {

	bool inverse_order;

    vector < string > source_files;
	string output;
    
};

void show_help (demo & d) {
    std::cout << "example show help" << std::endl;
}

void show_version(demo & d) {
    std::cout << "example show version" << std::endl;
}

void process (demo & d) {
    std::cout << "example process files" << std::endl;

	for (auto & f : d.source_files) {
		std::cout << f << endl;
	}

	std::cout << "output: " << d.output << std::endl;
	std::cout << "inverse: " << d.inverse_order << std::endl;
}

int main(int arg_c, char * arg_v[]) {

	using namespace command_line;

	// define the main usage expression separately for readability reasons
	auto main_exp =
		(-option("-i")[&demo::inverse_order]) >	// zero or one -i followed by
		key("-o", "--output")[&demo::output] >  // a mandatory -o or --output value
		(*any)[&demo::source_files]				// into an unknown amount of anything
	;

	auto cmd_line_exp =
		usage(option("-v", "--version"))[&show_version] |
		usage(option("-h", "--help"))[&show_help] |
		usage(main_exp)[&process];

	if (!parse < demo >(cmd_line_exp, arg_c, arg_v)) {
		std::cerr << "unexpected command line arguments" << std::endl;
	}

	return 0;
}
