#include <gtest/gtest.h>
#include <cmd_args.h>

#include <memory>

const char * option_test_values[] = {
	"dummy 1",
	"dummy 2",
	"dummy 3"
};

struct unit_option_settings {
	bool dummy;
};

TEST(unit_option_act, exec_happy) {
	using namespace command_line;

	auto opt = make_shared < _option_act_t < unit_option_settings > >(
		&unit_option_settings::dummy
	);

	auto range = _range_t{
		std::begin(option_test_values),
		std::end(option_test_values)
	};

	// clean up settings
	unit_option_settings settings = {};
	
	opt->exec(range, settings);

	EXPECT_TRUE(settings.dummy) << "settings variable should have been set to true";
}

TEST(unit_option_act, exec_finished_range) {
	using namespace command_line;

	auto opt = make_shared < _option_act_t < unit_option_settings > >(
		&unit_option_settings::dummy
	);

	auto range = _range_t{
		std::end(option_test_values),
		std::end(option_test_values)
	};

	// clean up settings
	unit_option_settings settings = {};

	opt->exec(range, settings);

	EXPECT_FALSE(settings.dummy) << "settings variable should have canceled this operation's execution";
}
