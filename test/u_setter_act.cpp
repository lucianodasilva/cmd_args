#include <gtest/gtest.h>

#include <cmd_args.h>
#include <memory>
#include <string>
#include <vector>

using namespace std;

const int setter_int_value = 2;

string setter_test_value = "dummy 1";

struct unit_setter_settings {
	string string_value;
	int int_value;
};

TEST(unit_setter_act, exec_happy_string) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < string unit_setter_settings::* > >(
		&unit_setter_settings::string_value,
        setter_test_value
	);

	// clean up settings
	unit_setter_settings settings = {};

	opt->exec(settings);

	EXPECT_STREQ(
		setter_test_value.c_str (),
		settings.string_value.c_str ()
	) << "Setter property content should match the first value of the setter_test_values";
}

TEST(unit_setter_act, exec_cast_happy) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < int unit_setter_settings::* > >(
		&unit_setter_settings::int_value,
        to_string (setter_int_value)
	);

	// clean up settings
	unit_setter_settings settings = {};

	opt->exec(settings);

	EXPECT_EQ(setter_int_value, settings.int_value)
		<< "casted value did not match expected value";
}

TEST(unit_setter_act, exec_cast_fail) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < int unit_setter_settings::* > >(
		&unit_setter_settings::int_value,
        "not an int"
    );

	// clean up settings
	unit_setter_settings settings = {};

	opt->exec(settings);

	EXPECT_EQ(0, settings.int_value)
		<< "cast fail should not change original value";
}