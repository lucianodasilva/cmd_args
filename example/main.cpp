//#include "cmd_args.h"
#include <iostream>
#include <functional>

//using namespace cmd_args;

// structure type that represents the 
// information to be read
//struct arguments_t {
//	bool show_version;
//	bool show_usage;
//	int value_a;
//	int value_b;
//};

#include <memory>
#include <vector>
#include <type_traits>

using namespace std;

namespace command_line {

	namespace details {

		// domain iterator range 
		struct range_t {
			const char
				** it,
				** end;

			// returns true if range is length is zero
			inline bool finished() const { return it == end; }

			// consume first item in range and return it's value
			inline const char * consume() {
				return *it++;
			}
		};

		// merge two ranges together
		// (TODO: this name and operator does not correctly represent function)
		inline range_t operator && (const range_t & v1, const range_t & v2) {
			if (distance(v1.it, v2.it) > 0)
				return{ v1.it, v2.it };
			else
				return{ v2.it, v1.it };
		}

		// action
		template < class _settings_t >
		struct base_action_t {
			virtual void run(const range_t & range, _settings_t & settings) const = 0;
		};

		// setter action specialization for single items
		template < class _settings_t, class _t >
		struct setter_t : public base_action_t < _settings_t > {
			using field_t = _t _settings_t::*;

			field_t address;

			setter_t(field_t address_v) : address(address_v) {}

			virtual void run(const range_t & r, _settings_t & settings) const override {
				//(TODO: should raise somekind of error)
				if (r.finished ())
					return;

				// aditional abstraction needed
				// perhaps should store processed value and not range...
				//settings.*address = 
			}
			
		};

		// setter specialization for vectors
		template < class _settings_t, class _t, class ... _other_tv >
		struct setter_t < _settings_t, std::vector < _t, _other_tv...> > : public base_action_t < _settings_t > {
			using field_t = std::vector < _t, _other_tv...> _settings_t::*;

			field_t address;

			setter_t(field_t address_v) : address(address_v) {}

			virtual void run(const range_t & r, _settings_t & settings) const override {
				// same implementation as before
			}
		};

		// callback action
		template < class _settings_t, class _method_t >
		struct callback_t;;

		template < class _settings_t >
		struct callback_t < _settings_t, void(*)(_settings_t) > : public base_action_t < _settings_t > {
			using function_t = void(*)(_settings_t);

			function_t callback;

			callback_t(function_t callback_v) : callback(callback_v) {}

			virtual void run(const range_t & r, _settings_t & settings) const override {
				// implement callback
			}
		};

		// callback no parameter action
		template < class _settings_t >
		struct callback_t < _settings_t, void (*)() > : public base_action_t < _settings_t > {
			using function_t = void(*)();

			function_t callback;

			callback_t(function_t callback_v) : callback(callback_v) {}

			virtual void run(const range_t & r, _settings_t & settings) const override {
				// implement callback
			}
		};

		// solution building/execution node
		template < class _settings_t >
		struct solution_node {
			range_t range;
			unique_ptr < base_action_t < _settings_t > > action;

			int sequence_index;
		};

		struct solution_tree_state_t {
			size_t	tree_size;
			int		parent_node;
		};

		template < class _settings_t >
		struct solution_tree_t {
		public:
			using item_t = solution_node < _settings_t >;
			using vector_t = vector < item_t >;
		private:
			vector_t tree;
			int32_t parent_node;
		public:

			inline item_t & operator [] (size_t i) {
				return tree[i];
			}

			inline size_t size() const {
				return tree.size();
			}

			inline solution_tree_t() : parent_node(-1) {}

			inline void add_node (const range_t & rng, base_action_t < _settings_t > * setter ) {
				tree.push_back(item_t {
					rng,
					unique_ptr < base_action_t < _settings_t > >(setter),
					parent_node++
				});
			}

			inline solution_tree_state_t state () const {
				return{
					tree.size(),
					parent_node
				};
			}

			inline void restore(const solution_tree_state_t & state) {
				tree.resize(state.tree_size);
				parent_node = state.parent_node;
			}

			inline void backtrack(const solution_tree_state_t & state) {
				parent_node = state.parent_node;
			}

		};

		struct context_state {
			solution_tree_state_t	tree_state;
			range_t					range;
		};

		template < class _settings_t >
		struct context {
			range_t							range;
			solution_tree_t	< _settings_t > solution_tree;
			_settings_t						settings;

			inline context_state state() const {
				return {
					solution_tree.state(),
					range
				};
			}

			inline void restore(const context_state & state) {
				solution_tree.restore(state.tree_state);
				range = state.range;
			}

			inline void backtrack(const context_state & state) {
				solution_tree.backtrack(state.tree_state);
				range = state.range;
			}
		};

		// setter operation
		template < class _exp_t, class _settings_t, class _t >
		struct setter_op {
			using field_t = _t _settings_t::*;

			_exp_t exp;
			field_t address;

			template < class cxt_t >
			inline bool eval(cxt_t & cxt) const {
				auto state = cxt.state();
				if (exp.eval(cxt)) {
					cxt.solution_tree.add_node(
						state.range && cxt.range, 
						new setter_t < _settings_t, _t >(address)
					);

					return true;
				}
				return false;
			}

			inline setter_op(const _exp_t & e, field_t address_v) : exp(e), address(address_v) {}
		};

		template < class _derived_t >
		struct setter_container_t {
			template < class _dest_t, class _t >
			inline auto operator [] (_t _dest_t::*address) {
				return setter_op < _derived_t, _dest_t, _t >(*static_cast < _derived_t * > (this), address);
			}
		};

		// callback operation
		template < class _exp_t, class _settings_t, class _method_t >
		struct callback_op {

			_exp_t exp;
			callback_t < _settings_t, _method_t > callback;

			template < class cxt_t >
			inline bool eval(cxt_t & cxt) const {
				auto state = cxt.state();
				if (exp.eval(cxt)) {
					cxt.solution_tree.add_node(
						state.range && cxt.range,
						new callback_t < _settings_t, _method_t >(callback)
						);

					return true;
				}
				return false;
			}
		};

		template < class _derived_t, class _settings_t >
		struct callback_container_t {

			template < class _method_t >
			inline auto operator [] (_method_t callback) {
				return callback_op < _settings_t, _method_t >(callback);
			}

		};

		// sequence
		template < class _lhe_t, class _rhe_t >
		struct seq_op {
			_lhe_t lhe;
			_rhe_t rhe;

			template < class cxt_t > 
			inline bool eval(cxt_t & cxt) const {
				return lhe.eval(cxt) && rhe.eval(cxt);
			}
		};

		template < class _lhe_t, class _rhe_t >
		auto operator >> (const _lhe_t & l, const _rhe_t & r) {
			return seq_op < _lhe_t, _rhe_t > { l, r };
		}

		// alternation
		template < class _lhe_t, class _rhe_t >
		struct alt_op {
			_lhe_t lhe;
			_rhe_t rhe;

			template < class cxt_t >
			inline bool eval(cxt_t & cxt) const {
				auto
					state = cxt.state(),
					lhe_state = state;

				bool 
					valid = lhe.eval(cxt),
					backtrack_lhe_state = valid;

				if (valid) {
					lhe_state = cxt.state();
					cxt.backtrack(state);
				} else {
					cxt.restore(state);
				}

				valid |= rhe.eval(cxt);

				if (!valid)
					cxt.restore(state);

				if (backtrack_lhe_state)
					cxt.backtrack(lhe_state);

				return valid;
			}
		};

		template < class _lhe_t, class _rhe_t >
		auto operator | (const _lhe_t & l, const _rhe_t & r) {
			return alt_op < _lhe_t, _rhe_t > { l, r };
		}

		// zero or one
		template < class _exp_t >
		struct z_one_op {
			_exp_t exp;

			template < class cxt_t >
			inline bool eval(cxt_t & cxt) const {
				auto state = context_state();

				if (!exp.eval(cxt))
					cxt.restore(state);

				return true;
			}
		};

		template < class _exp_t >
		auto operator - (const _exp_t & e) {
			return z_one_op < _exp_t > { e };
		}

		// one or n
		template < class _exp_t >
		struct one_n_op {
			_exp_t exp;

			template < class cxt_t >
			inline bool eval(cxt_t & cxt) const {
				bool one_valid = exp.eval(cxt);

				if (one_valid) {
					auto state = context_state();

					do {
						state = cxt.state();
					} while (exp.eval(cxt));

					cxt.restore(state);
				}

				return one_valid;
			}
		};

		template < class _exp_t >
		auto operator + (const _exp_t & e) {
			return one_n_op < _exp_t > { e };
		}

		// zero or n
		template < class _exp_t >
		struct z_n_op {
			_exp_t exp;

			template < class cxt_t >
			inline bool eval(cxt_t & cxt) const {
				auto state = context_state();

				do {
					state = cxt.state();
				} while (exp.eval(cxt));

				// restore last correct state
				cxt.restore(state);

				return true;
			}
		};

		template < class _exp_t >
		auto operator * (const _exp_t & e) {
			return z_n_op < _exp_t > { e };
		}

		// scaners
		struct value_t : setter_container_t < value_t > {
			const char * key;

			inline value_t ( const char * key_v ) : key (key_v) {}

			template < class cxt_t > 
			inline bool eval(cxt_t & cxt) const {
				if (cxt.range.finished ())
					return false;

				auto v = cxt.range.consume();
				return strcmp(v, key) == 0;
			}
		};

		struct any_t : setter_container_t < any_t > {

			template < class cxt_t >
			inline bool eval(cxt_t & cxt) const {
				if (cxt.range.finished())
					return false;

				cxt.range.consume();
				return true;
			}

		};

		// usage wrapper
		template < class _exp_t >
		struct usage_op {//: callback_container_t < usage_op < _exp_t > > {

			_exp_t exp;

			template < class cxt_t >
			inline bool eval(cxt_t & cxt) const {
				return exp.eval(cxt);
			}

			inline usage_op (const _exp_t & e) : exp(e) {}
		};

		// solution node inversion and execution
		template < class settings_t >
		inline bool execute(details::context < settings_t > & cxt, details::range_t & arg_range) {

			auto & tree = cxt.solution_tree;
			int solution_index = -1;

			for (
				int i = 0;
				i < static_cast < int > (tree.size());
				++i
			) {
				if (tree[i].range.end == arg_range.end) {
					solution_index = i;
					break;
				}
			}

			if (solution_index == -1)
				return false;

			// reorder instructions and execute
			{
				int n = solution_index;
				int i = tree[n].sequence_index;
				tree[n].sequence_index = -1;

				do {
					int t = tree[i].sequence_index;
					tree[i].sequence_index = n;
					n = i;
					i = t;
				} while (i != -1);

				while (i != -1) {
					auto & node = tree[i];
					node.action->run(node.range, cxt.settings);
					i = node.sequence_index;
				}
			}

			return true;
		}

	}

	// scanner methods
	inline details::value_t key(const char * k) {
		return{ k };
	}

	inline details::any_t any() { return{}; }

	// create callback expression
	template < class _exp_t >
	inline auto usage(const _exp_t & exp) {
		return details::usage_op < _exp_t > ( exp );
	}

	// parser 
	template < class settings_t, class _exp_t = void >
	inline bool parse(const _exp_t & exp, int arg_c, const char ** arg_v) {
		using namespace details;

		auto s_range = range_t {
			+arg_v + 1,	// ignore first item
			+arg_v + arg_c
		};

		context < settings_t > cxt = { s_range };

		// evaluate solutions
		exp.eval(cxt);

		// process and execute solutions
		return details::execute (cxt, s_range);
	}

}

struct demo {
	float	xxx;
	float	yyy;
	int		zzz;

	vector < string > files;

	bool show_version;
	bool show_help;
};

void main_usage(demo & d) {}

void show_version() {}

int main(int arg_c, char * arg_v[]) {

	using namespace command_line;
	
	const char * args[] = { "garbage", "-x", "-x", "-z", "luciano", "da", "silva" };
	
	//auto expression = key("-t") > key("-v") | +key("-x") > key("-v");

	auto usage_options = *(
		key("-x") [&demo::xxx] |
		key("-y") [&demo::yyy] |
		key("-z") [&demo::zzz]
	);
	
	auto usage_default_values = +any()[&demo::files];
	
	auto expression =
		usage (usage_options >> usage_default_values) | //.desc ("yada yada yada ayad acoiso e tal") |
		usage (usage_options) | //.desc ("coiso e tal")
		usage (key("-v"))
	;
	
	parse < demo > (expression, 7, args);
	
	//auto val = command_line::value(&demo::xxx, .0F);

	/*
	// define an instance of the command line parser
	parser < arguments_t > command_line;

	// set its available arguments and options
	command_line
		// option ( < member field address >, < list of command switches >, < description > )
		+ option(&arguments_t::show_version, { "-v", "--version" }, "shows the version of the application")
		+ option(&arguments_t::show_usage, { "-h, --help" }, "shows this usage page")
		// argument ( < member field address >, < list of command switches >, < description >, < is optional >, < default value > )
		+ argument(&arguments_t::value_a, { "-va", "--value_a" }, "multiplicand")
		+ argument(&arguments_t::value_b, { "-vb", "--value_b" }, "multiplier");

	arguments_t args;

	// command line parser returs a context with information about
	// the success of the parsing operation and related errors
	auto context = command_line.parse(arg_c, arg_v, args);

	// if the parsing failed
	if (context.has_failed()) {
		cout << "example failed with the following invalid arguments" << endl;
		// the user can print all errors to an ostream
		context.to_stream(cout);
		return -1;
	}

	if (args.show_version)
		cout << "example 1.0" << endl;

	// users can also print a list of options and arguments with their descriptions
	// to an ostream
	if (args.show_usage)
		command_line.usage_to_stream(cout);

	cout << "example value: " << (args.value_a * args.value_b) << endl;
	*/
	return 0;
}
