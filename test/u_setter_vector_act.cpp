#include <gtest/gtest.h>

#include <cmd_args.h>
#include <memory>
#include <string>
#include <vector>

using namespace std;

int setter_vector_int_value = 2;
int setter_vector_int_value_fail = 0;

string setter_vector_test_value = "dummy 1";

struct unit_setter_vector_settings {
	vector < string >	mult_string_value;
	vector < int >		mult_int_value;
};

TEST(unit_setter_vector_act, exec_mult_happy_string) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < vector < string > unit_setter_vector_settings::* > >(
		&unit_setter_vector_settings::mult_string_value,
        setter_vector_test_value
	);
    
    auto settings = unit_setter_vector_settings {};

    opt->exec(settings);

	EXPECT_EQ(
		setter_vector_test_value,
		settings.mult_string_value [0]
	);
}

TEST(unit_setter_vector_act, exec_mult_happy_cast) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < vector < int > unit_setter_vector_settings::* > >(
		&unit_setter_vector_settings::mult_int_value,
        to_string (setter_vector_int_value)
	);

	// clean up settings
	unit_setter_vector_settings settings = {};

	opt->exec(settings);

	EXPECT_EQ(
		setter_vector_int_value,
		settings.mult_int_value [0]
	);
}

TEST(unit_setter_vector_act, exec_mult_fail_cast) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < vector < int > unit_setter_vector_settings::* > >(
		&unit_setter_vector_settings::mult_int_value,
        "not an int"
    );

	// clean up settings
	unit_setter_vector_settings settings = {};

	opt->exec(settings);

	EXPECT_EQ(
		setter_vector_int_value_fail,
		settings.mult_int_value [0]
	);
}
