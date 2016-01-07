#include <gtest/gtest.h>
#include <cmd_args.h>

#include <memory>

const char * setter_test_values[] = {
	"dummy 1",
	"dummy 2",
	"dummy 3"
};

// implement tests for the following cases
// 1� - set happy path values
// 2� - range > 1
// 3� - empty range
// 4� - string conversion failure