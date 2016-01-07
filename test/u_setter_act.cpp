#include <gtest/gtest.h>
#include <cmd_args.h>

#include <memory>

const char * setter_test_values[] = {
	"dummy 1",
	"dummy 2",
	"dummy 3"
};

// implement tests for the following cases
// 1º - set happy path values
// 2º - range > 1
// 3º - empty range
// 4º - string conversion failure