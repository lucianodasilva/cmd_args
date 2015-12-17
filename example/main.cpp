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

using namespace std;

namespace command_line {

	namespace details {

		// domain iterator range 
		struct range_t {
			char 
				** it, 
				** end;

			// returns true if range is length is zero
			inline bool finished () const { return it == end; }

			// consume first item in range and return it's value
			inline const char * consume() {
				return *it++;
			}
		};

		// merge two ranges together
		// (TODO: this name and operator does not correctly represent method)
		inline range_t operator && (const range_t & v1, const range_t & v2) {
			if (distance(v1.it, v2.it) > 0)
				return{ v1.it, v2.it };
			else
				return{ v2.it, v1.it };
		}

		// action
		struct base_action_t {
			using unique = unique_ptr < base_action_t >;
			virtual void run(const range_t & range) const = 0;
		};

		// setter action specialization for single items
		template < class _dest_t, class _t >
		struct setter_t : public base_action_t {
			_t _dest_t::*address;

			setter_t(_t _dest_t::*address_v) : address(address) {}

			virtual void run(const range_t & r) const override {}
		};

		// setter specialization for vectors
		template < class _dest_t, class _t, class ... _other_tv >
		struct setter_t < _dest_t, vector < _t, _other_tv...> > : public base_action_t {
			vector < _t, _other_tv...> _dest_t::*address;
			setter_t(vector < _t, _other_tv...> _dest_t::*address_v) : address(address) {}

			virtual void run(const range_t & r) const override {}
		};

		// solution building/execution node
		struct solution_node {
			range_t range;
			base_action_t::unique action;

			int sequence_index;
		};

		struct solution_tree_state_t {
			size_t	tree_size;
			int		parent_node;
		};

		struct solution_tree_t {
		private:
			vector < solution_node > tree;
			int32_t parent_node;
		public:

			inline solution_node & operator [] (size_t i) {
				return tree[i];
			}

			inline size_t size() const {
				return tree.size();
			}

			inline solution_tree_t() : parent_node(-1) {}

			inline void add_node (const range_t & rng, base_action_t * setter ) {
				tree.push_back(solution_node{
					rng,
					unique_ptr < base_action_t >(setter),
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

		struct context {
			range_t			range;
			solution_tree_t	solution_tree;

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

		// setter_t
		template < class _exp_t, class _dest_t, class _t >
		struct setter_op {
			_exp_t exp;
			_t _dest_t::* address;

			inline bool eval(context & cxt) const {
				auto state = cxt.state();
				if (exp.eval(cxt)) {
					cxt.solution_tree.add_node(
						state.range && cxt.range, 
						new setter_t < _dest_t, _t >(address)
					);

					return true;
				}
				return false;
			}

			inline setter_op(const _exp_t & e, _t _dest_t::*address_v) : exp(e), address(address_v) {}
		};

		template < class _derived_t >
		struct setter_container_t {
			template < class _dest_t, class _t >
			inline auto operator [] (_t _dest_t::*address) {
				return setter_op < _derived_t, _dest_t, _t >(*static_cast < _derived_t * > (this), address);
			}
		};

		// sequence
		template < class _lhe_t, class _rhe_t >
		struct seq_op {
			_lhe_t lhe;
			_rhe_t rhe;

			inline bool eval(context & cxt) const {
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

			inline bool eval(context & cxt) const {
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

			inline bool eval(context & cxt) const {
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

			inline bool eval(context & cxt) const {
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

			inline bool eval(context & cxt) const {
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
		struct value_t : public setter_container_t < value_t > {
			const char * key;

			inline value_t ( const char * key_v ) : key (key_v) {}

			inline bool eval(context & cxt) const {
				if (cxt.range.finished ())
					return false;

				auto v = cxt.range.consume();
				return strcmp(v, key) == 0;
			}
		};

		struct any_t : public setter_container_t < any_t > {

			inline bool eval(context & cxt) const {
				if (cxt.range.finished())
					return false;

				cxt.range.consume();
				return true;
			}

		};

		// solution node inversion and execution
		inline bool execute(details::context & cxt, details::range_t & arg_range) {

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
					node.action->run(node.range);
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

	// create parameterless callback expression
	template < class _exp_t >
	inline auto usage(void(*callback)(), const _exp_t & exp) {
		return exp;
	}

	// create expression callback with parameter
	template < class _out_t, class _exp_t >
	inline auto usage(void (*callback)(_out_t &), const _exp_t & exp ) {
		return exp;
	}

	// parser 
	template < class _exp_t >
	inline bool parse(const _exp_t & exp, int arg_c, char ** arg_v) {
		using namespace details;

		auto s_range = range_t {
			+arg_v + 1,	// ignore first item
			+arg_v + arg_c
		};

		context cxt = { s_range };

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

	
	char * args[] = { "garbage", "-x", "-x", "-z", "luciano", "da", "silva" };
	
	//auto expression = key("-t") > key("-v") | +key("-x") > key("-v");

	auto usage_options = *(
		key("-x") [&demo::xxx] |
		key("-y") [&demo::yyy] |
		key("-z") [&demo::zzz]
	);
	
	auto usage_default_values = +any()[&demo::files];
	
	auto expression =
		usage (&main_usage,		usage_options >> usage_default_values) | //.desc ("yada yada yada ayad acoiso e tal") |
		usage (&main_usage,		usage_options) | //.desc ("coiso e tal")
		usage (&show_version,	key("-v"))
	;
	
	parse (expression, 7, args);
	
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
