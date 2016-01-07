#include <gtest/gtest.h>
#include <cmd_args.h>

int main(int arg_c, char ** argv) {
	::testing::InitGoogleTest(&arg_c, argv);
	return RUN_ALL_TESTS();
}