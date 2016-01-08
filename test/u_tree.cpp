#include <gtest/gtest.h>

#include <cmd_args.h>
#include <memory>
#include <string>

using namespace std;

const int unit_tree_node_count = 10;

class dummy_node : public command_line::_act_t {
public:

	int index;

	dummy_node ( int i ) : index (i) {}
	virtual ~dummy_node() {}
};

class unit_tree : public ::testing::Test {
public:

	command_line::_tree_t tree;

	unit_tree() {
		// initialization
	}

	virtual ~unit_tree() {
		// destruction
	}

protected:

	virtual void SetUp() override {
		using namespace command_line;
		tree = command_line::_tree_t();

		for (int i = 0; i < unit_tree_node_count; ++i) {
			tree.add_node(_range_t(), new dummy_node(i));
		}
	}

	virtual void TearDown() override {}

};

TEST_F(unit_tree, operator_index) {
	using namespace command_line;

	EXPECT_EQ(unit_tree_node_count, tree.size());

	for (int i = 0; i < unit_tree_node_count; ++i) {
		auto dummy = dynamic_cast <dummy_node *> (tree[i].action.get ());

		EXPECT_EQ(i, dummy->index) << "Dummy node indexex differ at index " << i;
	}
}

TEST_F(unit_tree, sequence_index) {
	using namespace command_line;

	EXPECT_EQ(unit_tree_node_count, tree.size());

	for (int i = 0; i < unit_tree_node_count; ++i) {
		EXPECT_EQ(i - 1, tree[i].sequence_index) << "Dummy node indexex differ at index " << i;
	}
}

TEST_F(unit_tree, size) {
	using namespace command_line;

	EXPECT_EQ(unit_tree_node_count, tree.size());
}

TEST_F(unit_tree, parent_node_default_value ) {
	using namespace command_line;

	tree = _tree_t();
	
	auto state = tree.state();
	EXPECT_EQ(_tree_t::default_parent_node, state.parent_node);
}

TEST_F(unit_tree, state) {
	using namespace command_line;
		 
	auto state = tree.state();
	EXPECT_EQ(unit_tree_node_count - 1, state.parent_node);
	EXPECT_EQ(unit_tree_node_count, state.tree_size);

	tree = _tree_t();

	auto empty_state = tree.state();
	EXPECT_EQ(-1, empty_state.parent_node);
	EXPECT_EQ(0, empty_state.tree_size);
}

TEST_F(unit_tree, restore) {
	using namespace command_line;

	tree = _tree_t();

	int half_node_count = unit_tree_node_count / 2;
	int full_node_count = half_node_count * 2;

	for (int i = 0; i < half_node_count; ++i) {
		tree.add_node(_range_t(), new dummy_node(i));
	}

	auto half_state = tree.state();

	for (int i = 0; i < half_node_count; ++i) {
		tree.add_node(_range_t(), new dummy_node(half_node_count + i));
	}

	auto final_state = tree.state();

	EXPECT_EQ(full_node_count, final_state.tree_size);
	EXPECT_EQ(full_node_count - 1, final_state.parent_node);

	tree.restore(half_state);

	auto restore_state = tree.state();

	EXPECT_EQ(half_node_count, restore_state.tree_size);
	EXPECT_EQ(half_node_count - 1, restore_state.parent_node);
}

TEST_F(unit_tree, backtrack) {
	using namespace command_line;

	tree = _tree_t();

	int half_node_count = unit_tree_node_count / 2;
	int full_node_count = half_node_count * 2;

	for (int i = 0; i < half_node_count; ++i) {
		tree.add_node(_range_t(), new dummy_node(i));
	}

	auto half_state = tree.state();

	for (int i = 0; i < half_node_count; ++i) {
		tree.add_node(_range_t(), new dummy_node(half_node_count + i));
	}

	auto final_state = tree.state();

	EXPECT_EQ(full_node_count, final_state.tree_size);
	EXPECT_EQ(full_node_count - 1, final_state.parent_node);

	tree.backtrack(half_state);

	auto backtrack_state = tree.state();

	EXPECT_EQ(full_node_count, backtrack_state.tree_size);
	EXPECT_EQ(half_node_count - 1, backtrack_state.parent_node);
}