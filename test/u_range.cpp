#include <gtest/gtest.h>
#include <cmd_args.h>

//class unit_range_t : public ::testing::Test {
//public:
//
//	unit_range_t() {
//		// initialization
//	}
//
//	virtual ~unit_range_t() {
//		// destruction
//	}
//
//protected:
//	virtual void SetUp() override {
//
//	}
//
//	virtual void TearDown() override {
//
//	}
//};

const char * range_test_values[] = {
	"dummy 1",
	"dummy 2",
	"dummy 3"
};

TEST (unit_range_t, finished_false) {
	using namespace command_line;

	_range_t range = {
		std::begin(range_test_values),
		std::end(range_test_values)
	};

	EXPECT_EQ(false, range.finished());
}

TEST (unit_range_t, finished_true) {
	using namespace command_line;

	_range_t range = {
		std::end(range_test_values),
		std::end(range_test_values)
	};

	EXPECT_EQ(true, range.finished());
}

TEST(unit_range_t, consume) {
	using namespace command_line;

	_range_t range = {
		std::begin(range_test_values),
		std::end(range_test_values)
	};

	auto starting_it_value = range.it;
	
	auto consume_value = range.consume();
	
	auto after_it_value = range.it;

	EXPECT_EQ(*starting_it_value, consume_value) 
		<< "Consumed returned value should be the value of the current iterator value before stepping forward";

	EXPECT_EQ(after_it_value, std::begin (range_test_values) + 1)
		<< "_range_t.consume () should stepp the iterator forward one position";
}

TEST(unit_range_t, instersect_ab) {
	using namespace command_line;

	_range_t range_a = {
		std::begin (range_test_values),
		std::end(range_test_values)
	};

	_range_t range_b = {
		std::end(range_test_values) - 1,
		std::end(range_test_values)
	};

	auto m = _range_t::intersect(range_a, range_b);

	EXPECT_EQ( range_a.it, m.it );
	EXPECT_EQ(range_b.it, m.end );
}

TEST(unit_range_t, instersect_ba) {
	using namespace command_line;

	_range_t range_a = {
		std::begin(range_test_values),
		std::end(range_test_values)
	};

	_range_t range_b = {
		std::end(range_test_values) - 1,
		std::end(range_test_values)
	};

	auto m = _range_t::intersect(range_b, range_a);

	EXPECT_EQ(range_a.it, m.it);
	EXPECT_EQ(range_b.it, m.end);
}