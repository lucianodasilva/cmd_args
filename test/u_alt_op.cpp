#include <gtest/gtest.h>

#include <cmd_args.h>
#include <memory>
#include <string>

using namespace std;

class unit_alt_dummy_node : public command_line::_act_t {
public:
	int id;
	inline unit_alt_dummy_node(int id_v) : id(id_v) {}
};

class unit_alt_op;

struct unit_alt_dummy_true {
	unit_alt_op * test_inst;
	inline bool eval(command_line::_cxt_t & cxt) const;
};

struct unit_alt_dummy_false {
	inline bool eval(command_line::_cxt_t & cxt) const {
		return false;
	}
};

class unit_alt_op : public ::testing::Test {
private:

	int _node_id;

public:

	int consume_id() {
		return ++_node_id;
	}

	command_line::_cxt_t context;

	unit_alt_op() {
		// initialization
	}

	virtual ~unit_alt_op() {
		// destruction
	}

protected:

	virtual void SetUp() override {
		context = command_line::_cxt_t();
		_node_id = 0;
	}

	virtual void TearDown() override {}

};

inline bool unit_alt_dummy_true::eval(command_line::_cxt_t & cxt) const {
	cxt.solution_tree.add_node(cxt.range, new unit_alt_dummy_node(test_inst->consume_id ()));
	return true;
}

inline int get_node_id( const command_line::_tree_t::item_t & item) {
	auto dnode = dynamic_cast <const unit_alt_dummy_node *> (item.action.get ());

	if (dnode) {
		return dnode->id;
	} else {
		return -1;
	}
}

TEST_F(unit_alt_op, eval_tt) {
	using namespace command_line;

	auto alt = command_line::operator | (
		unit_alt_dummy_true{this},
		unit_alt_dummy_true{this}
	);

	command_line::_cxt_t cxt;

	cxt.solution_tree.add_node(_range_t {}, new unit_alt_dummy_node(this->consume_id()));

	// evaluate solution tree
	EXPECT_TRUE(alt.eval(cxt));

	EXPECT_EQ(3, cxt.solution_tree.size()) << "The solution tree should contain both solution branches!";

	EXPECT_EQ (1, get_node_id (cxt.solution_tree[0])); // dummy node
	EXPECT_EQ (2, get_node_id (cxt.solution_tree[1])); // left expression node
	EXPECT_EQ (3, get_node_id (cxt.solution_tree[2])); // right expression node

}

TEST_F(unit_alt_op, eval_tf) {
	using namespace command_line;

	auto alt = command_line::operator | (
		unit_alt_dummy_true {this},
		unit_alt_dummy_false {}
	);

	command_line::_cxt_t cxt;
	cxt.solution_tree.add_node(_range_t{}, new unit_alt_dummy_node(this->consume_id()));

	EXPECT_TRUE(alt.eval(cxt));

	EXPECT_EQ(2, cxt.solution_tree.size()) << "The solution tree should contain only the left solution branch!";

	EXPECT_EQ(1, get_node_id(cxt.solution_tree[0])); // dummy node
	EXPECT_EQ(2, get_node_id(cxt.solution_tree[1])); // left expression node

}

TEST_F(unit_alt_op, eval_ft) {
	using namespace command_line;

	auto alt = command_line::operator | (
		unit_alt_dummy_false {},
		unit_alt_dummy_true {this}
	);

	command_line::_cxt_t cxt;
	cxt.solution_tree.add_node(_range_t{}, new unit_alt_dummy_node(this->consume_id()));

	EXPECT_TRUE(alt.eval(cxt));

	EXPECT_EQ(2, cxt.solution_tree.size()) << "The solution tree should contain only the right solution branch!";

	EXPECT_EQ(1, get_node_id(cxt.solution_tree[0])); // dummy node
	EXPECT_EQ(2, get_node_id(cxt.solution_tree[1])); // right expression node
}

TEST_F(unit_alt_op, eval_ff) {
	using namespace command_line;

	auto alt = command_line::operator | (
		unit_alt_dummy_false {},
		unit_alt_dummy_false {}
	);

	command_line::_cxt_t cxt;
	cxt.solution_tree.add_node(_range_t{}, new unit_alt_dummy_node(this->consume_id()));

	EXPECT_FALSE(alt.eval(cxt));

	EXPECT_EQ(1, cxt.solution_tree.size()) << "The solution tree should contain no solution branches!";
}
