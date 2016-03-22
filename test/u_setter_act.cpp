#include <gtest/gtest.h>

#include <cmd_args.h>
#include <memory>
#include <string>
#include <vector>

using namespace std;

const int setter_int_value = 2;

vector < string > setter_test_values = {
	"dummy 1",
	"2",
	"dummy 3"
};

struct unit_setter_settings {
	string string_value;
	int int_value;
};

TEST(unit_setter_act, exec_happy_string) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < string unit_setter_settings::* > >(
		&unit_setter_settings::string_value
	);

	auto range = _solution_range_t {
		std::begin(setter_test_values),
		std::begin(setter_test_values) + 1
	};

	// clean up settings
	unit_setter_settings settings = {};

	opt->exec(range, settings);

	EXPECT_STREQ(
		setter_test_values [0].c_str (),
		settings.string_value.c_str ()
	) << "Setter property content should match the first value of the setter_test_values";
}

TEST(unit_setter_act, exec_multiple_values) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < string unit_setter_settings::* > >(
		&unit_setter_settings::string_value
	);

	auto range = _solution_range_t {
		std::cbegin(setter_test_values),
		std::cend(setter_test_values)
	};

	// clean up settings
	unit_setter_settings settings = {};

	opt->exec(range, settings);

	EXPECT_STREQ(
		setter_test_values[0].c_str (),
		settings.string_value.c_str()
	) << "range values other than the first should be ignored";
}

TEST(unit_setter_act, exec_empty_range) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < string unit_setter_settings::* > >(
		&unit_setter_settings::string_value
		);

	auto range = _solution_range_t {
		std::cend(setter_test_values),
		std::cend(setter_test_values)
	};

	// clean up settings
	unit_setter_settings settings = {};

	opt->exec(range, settings);

	EXPECT_TRUE (settings.string_value.empty ()) 
		<< "empty range should not change any values";
}

TEST(unit_setter_act, exec_cast_happy) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < int unit_setter_settings::* > >(
		&unit_setter_settings::int_value
	);

	auto range = _solution_range_t {
		std::cbegin (setter_test_values) + 1,
		std::cbegin (setter_test_values)	+ 2
	};

	// clean up settings
	unit_setter_settings settings = {};

	opt->exec(range, settings);

	EXPECT_EQ(setter_int_value, settings.int_value)
		<< "casted value did not match expected value";
}

TEST(unit_setter_act, exec_cast_fail) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < int unit_setter_settings::* > >(
		&unit_setter_settings::int_value
		);

	auto range = _solution_range_t {
		std::cbegin(setter_test_values),
		std::cbegin(setter_test_values) + 1
	};

	// clean up settings
	unit_setter_settings settings = {};

	opt->exec(range, settings);

	EXPECT_EQ(0, settings.int_value)
		<< "cast fail should not change original value";
}