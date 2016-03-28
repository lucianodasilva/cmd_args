#include <gtest/gtest.h>
#include <cmd_args.h>

#include <memory>
#include <string>
#include <vector>

vector <string > option_test_values = {
	"dummy 1",
	"dummy 2",
	"dummy 3"
};

struct unit_option_settings {
	bool dummy;
};

TEST(unit_option_act, exec_happy) {
	using namespace command_line;

    auto opt = make_shared < _option_act_t < bool unit_option_settings::* > >(
		&unit_option_settings::dummy,
        ""
	);

	// clean up settings
    _error_cxt_t error = {};
	unit_option_settings settings = {};
	
	opt->exec(error, settings);

	EXPECT_TRUE(settings.dummy) << "settings variable should have been set to true";
}
