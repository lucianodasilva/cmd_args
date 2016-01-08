#include <gtest/gtest.h>

#include <cmd_args.h>
#include <memory>
#include <string>

using namespace std;

class unit_alt_dummy_node : public command_line::_act_t {};

struct unit_alt_dummy_true {
	bool value;

	inline bool eval(command_line::_cxt_t & cxt) const {
		cxt.solution_tree.add_node(cxt.range, new unit_alt_dummy_node());
		return true;
	}
};

struct unit_alt_dummy_false {
	bool value;

	inline bool eval(command_line::_cxt_t & cxt) const {
		return false;
	}
};

//TEST(unit_seq_op, eval_tt) {
//	auto seq = command_line::operator | (
//		unit_alt_dummy_true{},
//		unit_alt_dummy_true{}
//	);
//
//	command_line::_cxt_t cxt;
//
//	EXPECT_TRUE(seq.eval(cxt));
//
//	// evaluate solution tree
//}
//
//TEST(unit_seq_op, eval_tf) {
//	auto seq = command_line::operator > (
//		unit_seq_dummy{ true },
//		unit_seq_dummy{ false }
//	);
//
//	command_line::_cxt_t cxt;
//
//	EXPECT_FALSE(seq.eval(cxt));
//}
//
//TEST(unit_seq_op, eval_ft) {
//	auto seq = command_line::operator > (
//		unit_seq_dummy{ false },
//		unit_seq_dummy{ true }
//	);
//
//	command_line::_cxt_t cxt;
//
//	EXPECT_FALSE(seq.eval(cxt));
//}
//
//TEST(unit_seq_op, eval_ff) {
//	auto seq = command_line::operator > (
//		unit_seq_dummy{ false },
//		unit_seq_dummy{ false }
//	);
//
//	command_line::_cxt_t cxt;
//
//	EXPECT_FALSE(seq.eval(cxt));
//}