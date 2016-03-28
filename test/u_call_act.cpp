#include <gtest/gtest.h>

#include <cmd_args.h>
#include <memory>
#include <string>

using namespace std;

struct unit_call_settings {
	bool called;
};

void unit_call_act_callback(unit_call_settings & settings) {
	settings.called = true;
}

TEST(unit_call_act, exec_callback) {
	using namespace command_line;

    auto opt = make_shared < _call_act_t < void (*)(unit_call_settings&) > >(&unit_call_act_callback, "");
    
    _error_cxt_t error = {};
	unit_call_settings settings = {};

    opt->exec(error, settings);

	EXPECT_TRUE(settings.called);
}

TEST(unit_call_act, exec_null_callback) {
	using namespace command_line;

	auto opt = make_shared < _call_act_t < void (*)(unit_call_settings&) > >(nullptr, "");
	_error_cxt_t error = {};
    unit_call_settings settings = {};

	opt->exec(error, settings);

	EXPECT_FALSE(settings.called);
}