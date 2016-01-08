#include <gtest/gtest.h>

#include <cmd_args.h>
#include <memory>
#include <string>

using namespace std;

struct unit_seq_dummy {
	bool value;

	inline bool eval(command_line::_cxt_t & cxt) const {
		return value;
	}
};

TEST(unit_seq_op, eval_tt) {
	auto seq = command_line::operator > (
		unit_seq_dummy { true },
		unit_seq_dummy { true }
	);

	command_line::_cxt_t cxt;

	EXPECT_TRUE(seq.eval(cxt));
}

TEST(unit_seq_op, eval_tf) {
	auto seq = command_line::operator > (
		unit_seq_dummy{ true },
		unit_seq_dummy{ false }
	);

	command_line::_cxt_t cxt;

	EXPECT_FALSE(seq.eval(cxt));
}

TEST(unit_seq_op, eval_ft) {
	auto seq = command_line::operator > (
		unit_seq_dummy{ false },
		unit_seq_dummy{ true }
	);

	command_line::_cxt_t cxt;

	EXPECT_FALSE(seq.eval(cxt));
}

TEST(unit_seq_op, eval_ff) {
	auto seq = command_line::operator > (
		unit_seq_dummy{ false },
		unit_seq_dummy{ false }
	);

	command_line::_cxt_t cxt;

	EXPECT_FALSE(seq.eval(cxt));
}