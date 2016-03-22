#include <gtest/gtest.h>

#include <cmd_args.h>
#include <memory>
#include <string>
#include <vector>

using namespace std;

vector < int > setter_vector_int_values = {
	2,
	3
};

vector < int > setter_vector_int_values_fail = {
	2,
	3
};

vector < string > setter_vector_test_values = {
	"dummy 1",
	"2",
	"3",
	"dummy 4"
};

struct unit_setter_vector_settings {
	vector < string >	mult_string_value;
	vector < int >		mult_int_value;
};

namespace unit_setter_vector_details {
	template <class C>
	constexpr auto size(const C& c) -> decltype(c.size()) {
		return c.size();
	}

	template <class T, std::size_t N>
	constexpr std::size_t size(const T(&array)[N]) noexcept {
		return N;
	}
}

#define EXPECT_VECTOR_EQ(expected, actual) \
ASSERT_EQ(unit_setter_vector_details::size (expected), unit_setter_vector_details::size (actual)) << "Vectors expected and actual are of unequal length"; \
for (decltype (unit_setter_vector_details::size (expected)) i = 0; i < unit_setter_vector_details::size (expected); ++i) { \
	EXPECT_EQ(expected[i], actual[i]) << "Vectors expected and actual differ at index " << i; \
}																			 

TEST(unit_setter_vector_act, exec_mult_happy_string) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < vector < string > unit_setter_vector_settings::* > >(
		&unit_setter_vector_settings::mult_string_value
	);

    auto range = _solution_range_t::from_source (setter_vector_test_values, 0);

	// clean up settings
	unit_setter_vector_settings settings = {};

	opt->exec(range, settings);

	EXPECT_VECTOR_EQ(
		setter_vector_test_values,
		settings.mult_string_value
	);
}

TEST(unit_setter_vector_act, exec_mult_happy_cast) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < vector < int > unit_setter_vector_settings::* > >(
		&unit_setter_vector_settings::mult_int_value
	);

	auto range = _solution_range_t {
		std::cbegin(setter_vector_test_values) + 1,
		std::cbegin(setter_vector_test_values) + 3
	};

	// clean up settings
	unit_setter_vector_settings settings = {};

	opt->exec(range, settings);

	EXPECT_VECTOR_EQ(
		setter_vector_int_values,
		settings.mult_int_value
	);
}

TEST(unit_setter_vector_act, exec_mult_fail_cast) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < vector < int > unit_setter_vector_settings::* > >(
		&unit_setter_vector_settings::mult_int_value
		);

	auto range = _solution_range_t {
		std::begin(setter_vector_test_values) + 1,
		std::end(setter_vector_test_values)
	};

	// clean up settings
	unit_setter_vector_settings settings = {};

	opt->exec(range, settings);

	EXPECT_VECTOR_EQ(
		setter_vector_int_values_fail,
		settings.mult_int_value
	);
}

TEST(unit_setter_vector_act, exec_empty_range) {
	using namespace command_line;

    auto opt = make_shared < _setter_act_t < vector < string > unit_setter_vector_settings::* > >(
		&unit_setter_vector_settings::mult_string_value
	);

	auto range = _solution_range_t {
		std::end(setter_vector_test_values),
		std::end(setter_vector_test_values)
	};

	// clean up settings
	unit_setter_vector_settings settings = {};

	opt->exec(range, settings);

	EXPECT_TRUE (settings.mult_string_value.empty ()) 
		<< "empty range should not change any values";
}