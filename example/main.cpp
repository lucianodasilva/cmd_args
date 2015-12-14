//#include "cmd_args.h"
#include <iostream>

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

//
//namespace command_line {
//
//	namespace details {
//
//		// struture 
//		using range_it = std::reverse_iterator < char ** >;
//
//		struct range_t {	
//			range_it it, end;
//
//			inline bool ended () const { return it == end; }
//
//			inline const char * consume() {
//				return *it++;
//			}
//
//			inline range_t union_with(const range_t & r) const {
//				if (distance(it, r.it) > 0)
//					return range_t{ it, r.it + 1 };
//				else
//					return range_t{ r.it, it + 1 };
//			}
//
//			inline char ** forward_begin() const {
//				return it.base() + 1;
//			}
//			
//			inline char ** forward_end() const {
//				return end.base() + 1;
//			}
//
//		};
//
//		struct base_setter_t {
//			using unique = unique_ptr < base_setter_t >;
//			virtual void set(const range_t & r) = 0;
//		};
//
//		template < class _dest_t, class _t > 
//		struct setter_t : public base_setter_t {
//			_t _dest_t::*address;
//
//			setter_t(_t _dest_t::*address_v) : address(address) {}
//
//			virtual void set(const range_t & r) {}
//		};
//
//		template < class _dest_t, class _t, class ... _other_tv >
//		struct setter_t < _dest_t, vector < _t, _other_tv...> > : public base_setter_t {
//			vector < _t, _other_tv...> _dest_t::*address;
//			setter_t(vector < _t, _other_tv...> _dest_t::*address_v) : address(address) {}
//
//			virtual void set(const range_t & r) {}
//		};
//
//		struct exec_node {
//			range_t range;
//			base_setter_t::unique setter;
//		};
//
//		using exec_queue_t = vector < exec_node >;
//
//		struct context_state {
//			uint32_t queue_len;
//			range_t range;
//		};
//
//		struct context {
//			exec_queue_t	exec_queue;
//			range_t			range;
//
//			inline context_state state() const {
//				return{
//					exec_queue.size(),
//					range
//				};
//			}
//
//			inline void restore(const context_state & state) {
//				exec_queue.resize(state.queue_len);
//				range = state.range;
//			}
//		};
//
//		// setter_t
//		template < class _exp_t, class _dest_t, class _t >
//		struct setter_op {
//			_exp_t exp;
//			_t _dest_t::* address;
//
//			inline bool eval(context & cxt) const {
//				auto state = cxt.state();
//				if (exp.eval(cxt)) {
//					auto range = state.range.union_with(cxt.range);
//					cxt.exec_queue.push_back( exec_node {
//						range,
//						make_unique < setter_t < _dest_t, _t > > (address)
//					});
//					return true;
//				}
//				return false;
//			}
//
//			inline setter_op(const _exp_t & e, _t _dest_t::*address_v) : exp(e), address(address_v) {}
//		};
//
//		// sequence
//		template < class _lhe_t, class _rhe_t >
//		struct seq_op  {
//			_lhe_t lhe;
//			_rhe_t rhe;
//
//			inline bool eval (context & cxt) const {
//				return rhe.eval(cxt) && lhe.eval(cxt);
//			}
//		};
//
//		template < class _lhe_t, class _rhe_t >
//		auto operator > (const _lhe_t & l, const _rhe_t & r) {
//			return seq_op < _lhe_t, _rhe_t > { l, r };
//		}
//
//		// alternation
//		template < class _lhe_t, class _rhe_t >
//		struct alt_op {
//			_lhe_t lhe;
//			_rhe_t rhe;
//
//			inline bool eval(context & cxt) const {
//				auto state = cxt.state();
//				bool valid = rhe.eval(cxt);
//
//				if (!valid) {
//					cxt.restore(state);
//					valid = lhe.eval(cxt);
//				}
//
//				return valid;
//			}
//		};
//
//		template < class _lhe_t, class _rhe_t >
//		auto operator | (const _lhe_t & l, const _rhe_t & r) {
//			return alt_op < _lhe_t, _rhe_t > { l, r };
//		}
//
//		// zero or one
//		template < class _exp_t >
//		struct z_one_op {
//			_exp_t exp;
//
//			inline bool eval(context & cxt) const {
//				auto state = context_state();
//
//				if (!exp.eval(cxt))
//					cxt.restore(state);
//
//				return true;
//			}
//		};
//
//		template < class _exp_t >
//		auto operator - (const _exp_t & e) {
//			return z_one_op < _exp_t > { e };
//		}
//
//		// one or n
//		template < class _exp_t >
//		struct one_n_op {
//			_exp_t exp;
//
//			inline bool eval(context & cxt) const {
//				bool one_valid = exp.eval(cxt);
//
//				if (one_valid) {
//					auto state = context_state();
//
//					do {
//						state = cxt.state ();
//					} while (exp.eval(cxt));
//
//					cxt.restore(state);
//				}
//
//				return one_valid;
//			}
//		};
//
//		template < class _exp_t >
//		auto operator + (const _exp_t & e) {
//			return one_n_op < _exp_t > { e };
//		}
//
//		// zero or n
//		template < class _exp_t >
//		struct z_n_op {
//			_exp_t exp;
//
//			inline bool eval(context & cxt) const{
//				auto state = context_state();
//
//				do {
//					state = cxt.state();
//				} while (exp.eval(cxt));
//
//				cxt.restore(state);
//				return true;
//			}
//		};
//
//		template < class _exp_t >
//		auto operator * (const _exp_t & e) {
//			return z_n_op < _exp_t > { e };
//		}
//
//		// scaners
//		struct value_t {
//			const char * key;
//
//			inline bool eval(context & cxt) const { 
//				if (cxt.range.ended())
//					return false;
//
//				auto v = cxt.range.consume();
//				return strcmp (v, key) == 0;
//			}
//
//			template < class _dest_t, class _t >
//			inline auto operator [] (_t _dest_t::*address) {
//				return setter_op < value_t, _dest_t, _t >(*this, address);
//			}
//		};
//		
//		struct any_t {
//
//			inline bool eval(context & cxt) const { 
//				if (cxt.range.ended())
//					return false;
//
//				cxt.range.consume();
//				return true;
//			}
//
//			template < class _dest_t, class _t >
//			inline auto operator [] (_t _dest_t::*address) {
//				return setter_op < any_t, _dest_t, _t >(*this, address);
//			}
//		};
//
//		// Using
//		/* template < class _exp_t >
//		struct using_t {
//			_exp_t exp;
//
//			inline bool eval(context & cxt) const {
//				return exp.eval(cxt);
//			}
//
//			template < class _dest_t, class _t >
//			inline auto operator [] (_t _dest_t::*address) {
//				
//			}
//
//		}; */
//	}
//
//	inline details::value_t key(const char * k) {
//		return { k };
//	}
//
//	inline details::any_t any() { return{}; }
//
//	template < class _t >
//	inline bool parse(const _t & exp, int arg_c, char ** arg_v ) {
//		using namespace details;
//
//		context cxt;
//
//		// reverse range
//		cxt.range = {
//			reverse_iterator < char ** >(arg_v + arg_c),
//			reverse_iterator < char ** >(+arg_v) // ignore first item
//		};
//		
//		bool has_valid = exp.eval(cxt) && cxt.range.ended ();
//
//		return false;
//	}
//
//}


namespace command_line {

	namespace details {

		// struture 
		struct range_t {
			char 
				** it, 
				** end;

			inline bool ended() const { return it == end; }

			inline const char * consume() {
				return *it++;
			}

			inline range_t union_with(const range_t & r) const {
				if (distance(it, r.it) > 0)
					return range_t{ it, r.it + 1 };
				else
					return range_t{ r.it, it + 1 };
			}

		};

		struct base_setter_t {
			using unique = unique_ptr < base_setter_t >;
			virtual void set(const range_t & r) = 0;
		};

		template < class _dest_t, class _t >
		struct setter_t : public base_setter_t {
			_t _dest_t::*address;

			setter_t(_t _dest_t::*address_v) : address(address) {}

			virtual void set(const range_t & r) {}
		};

		template < class _dest_t, class _t, class ... _other_tv >
		struct setter_t < _dest_t, vector < _t, _other_tv...> > : public base_setter_t {
			vector < _t, _other_tv...> _dest_t::*address;
			setter_t(vector < _t, _other_tv...> _dest_t::*address_v) : address(address) {}

			virtual void set(const range_t & r) {}
		};

		struct solution_node {
			range_t range;
			base_setter_t::unique setter;

			uint32_t seq_index;
		};

		using solution_tree_t = vector < solution_node >;

		struct context_state {
			uint32_t solution_len;
			uint32_t parent_node;
			range_t	 range;
		};

		struct context {
			solution_tree_t	solution_tree;
			uint32_t		parent_node;
			range_t			range;

			inline context_state state() const {
				return{
					static_cast < uint32_t > (solution_tree.size()),
					parent_node,
					range
				};
			}

			inline void restore(const context_state & state) {
				solution_tree.resize(state.solution_len);
				range = state.range;
			}

			inline void backtrack(const context_state & state) {
				range = state.range;
				parent_node = state.parent_node;
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
					auto range = state.range.union_with(cxt.range);
					cxt.solution_tree.push_back(solution_node {
						range,
						make_unique < setter_t < _dest_t, _t > >(address),
						cxt.parent_node++
					});
					return true;
				}
				return false;
			}

			inline setter_op(const _exp_t & e, _t _dest_t::*address_v) : exp(e), address(address_v) {}
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
				auto state = cxt.state();
				bool valid = lhe.eval(cxt);

				cxt.backtrack(state);
				valid |= rhe.eval(cxt);

				if (!valid)
					cxt.restore(state);

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

				cxt.restore(state);
				return true;
			}
		};

		template < class _exp_t >
		auto operator * (const _exp_t & e) {
			return z_n_op < _exp_t > { e };
		}

		// scaners
		struct value_t {
			const char * key;

			inline bool eval(context & cxt) const {
				if (cxt.range.ended())
					return false;

				auto v = cxt.range.consume();
				return strcmp(v, key) == 0;
			}

			template < class _dest_t, class _t >
			inline auto operator [] (_t _dest_t::*address) {
				return setter_op < value_t, _dest_t, _t >(*this, address);
			}
		};

		struct any_t {

			inline bool eval(context & cxt) const {
				if (cxt.range.ended())
					return false;

				cxt.range.consume();
				return true;
			}

			template < class _dest_t, class _t >
			inline auto operator [] (_t _dest_t::*address) {
				return setter_op < any_t, _dest_t, _t >(*this, address);
			}
		};

		// Using
		/* template < class _exp_t >
		struct using_t {
		_exp_t exp;

		inline bool eval(context & cxt) const {
		return exp.eval(cxt);
		}

		template < class _dest_t, class _t >
		inline auto operator [] (_t _dest_t::*address) {

		}

		}; */
	}

	inline details::value_t key(const char * k) {
		return{ k };
	}

	inline details::any_t any() { return{}; }

	template < class _t >
	inline bool parse(const _t & exp, int arg_c, char ** arg_v) {
		using namespace details;

		context cxt;

		cxt.parent_node = 0;
		// reverse range
		cxt.range = {
			arg_v,
			+arg_v + arg_c// ignore first item
		};

		// check for valid and complete solutions
		bool has_valid_solutions = exp.eval(cxt) && cxt.range.ended();

		// assemble complete solution

		return false;
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
		usage_options |
		usage_options >> usage_default_values;
	;
	
	parse(expression, 7, args);
	
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
