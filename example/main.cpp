//#include "cmd_args.h"
#include <iostream>
#include <sstream>
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

	// iterator domain range 
	struct _range_t {
		const char
			** it,
			** end;

		// returns true if range is length is zero
		inline bool finished() const { return it == end; }

		// consume first item in range and return it's value
		inline const char * consume() {
			return *it++;
		}

		inline static _range_t intersect ( 
				const _range_t & v1, 
				const _range_t & v2 
		) {
			if (distance(v1.it, v2.it) > 0)
				return { v1.it, v2.it };
			else
				return { v2.it, v1.it };
		}

	};

	struct _action_t {
		virtual ~_action_t () {}
	};

	template < class _settings_t >
	struct _typed_action_t : public _action_t {
		virtual void exec ( const _range_t & range, _settings_t & settings) const = 0;
	};
    
    // action for options
    template < class _settings_t >
    struct _option_t : public _typed_action_t < _settings_t > {
        using field_t = bool _settings_t::*;
        field_t address;
        
        _option_t (field_t address_v) : address(address_v) {}
        
        virtual void exec(const _range_t & range, _settings_t & settings) const override {
            //(TODO: should raise somekind of error)
            if (range.finished ())
                return;
            
            settings.*address = true;
        }
    };

	// setter action specialization for single items
	template < class _settings_t, class _t >
	struct _setter_t : public _typed_action_t < _settings_t > {
		using field_t = _t _settings_t::*;
		field_t address;

		_setter_t (field_t address_v) : address(address_v) {}

		virtual void exec(const _range_t & range, _settings_t & settings) const override {
			//(TODO: should raise somekind of error)
			if (range.finished ())
				return;

            stringstream stream (*range.it);
            stream >> settings.*address;
            
            if (!stream) {
                // TODO: should report error
            }
		}
	};

	// setter specialization for vectors
	template < class _settings_t, class _t, class ... _other_tv >
	struct _setter_t < _settings_t, std::vector < _t, _other_tv...> > : public _typed_action_t < _settings_t > {
		using field_t = std::vector < _t, _other_tv...> _settings_t::*;
		field_t address;

		_setter_t (field_t address_v) : address(address_v) {}

		virtual void exec (const _range_t & range, _settings_t & settings) const override {
            //(TODO: should raise somekind of error)
            if (range.finished ())
                return;
            
            stringstream stream;
            
            for ( auto it = range.it; it != range.end; ++it) {
                stringstream stream (*it);
                stream.clear ();
                
                _t value;
                
                stream >> value;

                if (stream) {
                    (settings.*address).push_back (value);
                } else {
                    // TODO: should report error
                }
            }
		}
	};

	// callback action
	template < class _settings_t >
	struct _callback_t : public _typed_action_t < _settings_t > {
		using function_t = void(*)(_settings_t&);
		function_t callback;

		_callback_t(function_t callback_v) : callback(callback_v) {}

		virtual void exec (const _range_t & range, _settings_t & settings) const override {
			// implement callback
		}
	};

	//  execution node
	struct _exec_node {
		_range_t range;
		unique_ptr < _action_t > action;

		int sequence_index;
	};

	struct _tree_state_t {
		size_t	tree_size;
		int		parent_node;
	};

	struct _tree_t {
	public:
		using item_t = _exec_node;
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

		inline _tree_t() : parent_node(-1) {}

		inline void add_node (const _range_t & rng, _action_t * setter ) {
			tree.push_back(item_t {
				rng,
				unique_ptr < _action_t >(setter),
				parent_node++
			});
		}

		inline _tree_state_t state () const {
			return{
				tree.size(),
				parent_node
			};
		}

		inline void restore(const _tree_state_t & state) {
			tree.resize(state.tree_size);
			parent_node = state.parent_node;
		}

		inline void backtrack(const _tree_state_t & state) {
			parent_node = state.parent_node;
		}

	};

	struct _cxt_state {
		_tree_state_t	tree_state;
		_range_t					range;
	};

	struct _cxt_t {
		_range_t		range;
		_tree_t			solution_tree;

		inline _cxt_state state() const {
			return {
				solution_tree.state(),
				range
			};
		}

		inline void restore(const _cxt_state & state) {
			solution_tree.restore(state.tree_state);
			range = state.range;
		}

		inline void backtrack(const _cxt_state & state) {
			solution_tree.backtrack(state.tree_state);
			range = state.range;
		}
	};
    
    // option operation
    template < class _exp_t, class _settings_t >
    struct _option_op {
        using field_t = bool _settings_t::*;
        
        _exp_t exp;
        field_t address;
        
        inline bool eval(_cxt_t & cxt) const {
            auto state = cxt.state();
            if (exp.eval(cxt)) {
                cxt.solution_tree.add_node(
                    _range_t::intersect (state.range, cxt.range),
                    new _option_t < _settings_t >(address)
                );
                
                return true;
            }
            return false;
        }
        
        inline _option_op(const _exp_t & e, field_t address_v) : exp(e), address(address_v) {}
    };

    template < class _derived_t >
    struct _with_option_t {
        template < class _dest_t >
        inline auto operator [] ( bool _dest_t::*address ) {
            return _option_op < _derived_t, _dest_t > (*static_cast < _derived_t * > (this), address);
        }
    };
    
	// setter operation
	template < class _exp_t, class _settings_t, class _t >
	struct _setter_op {
		using field_t = _t _settings_t::*;

		_exp_t exp;
		field_t address;

		inline bool eval(_cxt_t & cxt) const {
			auto state = cxt.state();
			if (exp.eval(cxt)) {
				cxt.solution_tree.add_node(
                    _range_t::intersect (state.range, cxt.range),
					new _setter_t < _settings_t, _t >(address)
				);

				return true;
			}
			return false;
		}

		inline _setter_op(const _exp_t & e, field_t address_v) : exp(e), address(address_v) {}
	};

	template < class _derived_t >
	struct _with_setter_t {
		template < class _dest_t, class _t >
		inline auto operator [] (_t _dest_t::*address) {
			return _setter_op < _derived_t, _dest_t, _t >(*static_cast < _derived_t * > (this), address);
		}
	};

	// callback operation
	template < class _exp_t, class _settings_t >
	struct _callback_op {

		_exp_t exp;
        typename _callback_t < _settings_t >::function_t method;

		inline bool eval(_cxt_t & cxt) const {
			auto state = cxt.state();
			if (exp.eval(cxt)) {
				cxt.solution_tree.add_node(
					_range_t::intersect (state.range, cxt.range),
					new _callback_t < _settings_t >(method)
                );

				return true;
			}
			return false;
		}
	};

	template < class _derived_t >
	struct _with_callback_t {

		template < class _settings_t >
        inline auto operator [] ( void (*method)(_settings_t &) ) {
            return _callback_op < _derived_t, _settings_t > {
                *static_cast < _derived_t * > (this),
                method
            };
		}

	};

	// sequence
	template < class _lhe_t, class _rhe_t >
	struct _seq_op {
		_lhe_t lhe;
		_rhe_t rhe;

		inline bool eval(_cxt_t & cxt) const {
			return lhe.eval(cxt) && rhe.eval(cxt);
		}
	};

	template < class _lhe_t, class _rhe_t >
	auto operator >> (const _lhe_t & l, const _rhe_t & r) {
		return _seq_op < _lhe_t, _rhe_t > { l, r };
	}

	// alternation
	template < class _lhe_t, class _rhe_t >
	struct _alt_op {
		_lhe_t lhe;
		_rhe_t rhe;
        
		inline bool eval(_cxt_t & cxt) const {
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
		return _alt_op < _lhe_t, _rhe_t > { l, r };
	}

	// zero or one
	template < class _exp_t >
	struct _z_one_op {
		_exp_t exp;

		inline bool eval(_cxt_t & cxt) const {
			auto state = _cxt_state();

			if (!exp.eval(cxt))
				cxt.restore(state);

			return true;
		}
	};

	template < class _exp_t >
	auto operator - (const _exp_t & e) {
		return _z_one_op < _exp_t > { e };
	}

	// one or n
	template < class _exp_t >
	struct _one_n_op {
		_exp_t exp;

		inline bool eval(_cxt_t & cxt) const {
			bool one_valid = exp.eval(cxt);

			if (one_valid) {
				auto state = _cxt_state();

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
		return _one_n_op < _exp_t > { e };
	}

	// zero or n
	template < class _exp_t >
	struct _z_n_op {
		_exp_t exp;

		template < class cxt_t >
		inline bool eval(cxt_t & cxt) const {
			auto state = _cxt_state ();

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
		return _z_n_op < _exp_t > { e };
	}

	// scaners
    struct _switch_t : _with_option_t< _switch_t > {
        
        vector < const char * > keys;
        
        inline _switch_t ( const initializer_list< const char * > & keys_v ) : keys ( keys_v) {}
        
        inline bool eval (_cxt_t & cxt) const {
            if (cxt.range.finished ())
                return false;
            
            // cache key
            auto v = cxt.range.consume ();
            
            for ( auto * k : keys ) {
                if (strcmp (v, k) == 0)
                    return true;
            }
            
            return false;
        }
    };
    
	struct _value_t : _with_setter_t < _value_t > {
        
		vector < const char * > keys;

		inline _value_t ( const initializer_list < const char * > & keys_v ) : keys (keys_v) {}

		inline bool eval(_cxt_t & cxt) const {
			if (cxt.range.finished ())
				return false;
            
            if (keys.size () == 0)
                return false;
            
            // cache key
            auto * v    = cxt.range.consume ();
            
            // find '=' key / value separator
            auto * vc   = strchr (v, '=');
            
            if (!vc) {
                // report malformed string
                return false;
            }
            
            size_t v_len = size_t (vc - v);
            
            // parse for proper key
            for ( const char * k : keys) {
                if (strncmp (v, k, v_len) == 0)
                    return true;
            }
            
			return false;
		}
	};

	struct _any_t : _with_setter_t < _any_t > {

		inline bool eval(_cxt_t & cxt) const {
			if (cxt.range.finished())
				return false;

			cxt.range.consume();
			return true;
		}

	};

	// usage wrapper
	template < class _exp_t >
    struct _usage_op : _with_callback_t < _usage_op < _exp_t > >  {

		_exp_t exp;

		inline bool eval(_cxt_t & cxt) const {
			return exp.eval(cxt);
		}

		inline _usage_op (const _exp_t & e) : exp(e) {}
	};

	// solution node inversion and execution
	template < class settings_t >
	inline bool _execute(_cxt_t & cxt, _range_t & arg_range) {

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
            
            // create settings instance
            settings_t settings_instance = { 0 };
            
            // reset position
            i = 0;

			while (i != -1) {
				auto & node = tree[i];
                auto typed_node = dynamic_cast < _typed_action_t < settings_t > * > (node.action.get());
                
                if (typed_node) {
                    typed_node->exec (node.range, settings_instance);
                } else {
                    // report error
                }
            
				//node.action->run(node.range, cxt.settings);
				i = node.sequence_index;
			}
		}

		return true;
	}

	// scanner methods
    
    template < class _char_t >
    inline _switch_t option ( const _char_t * v ... ) {
        return { v };
    }
    
    template < class _char_t >
	inline _value_t key( const _char_t * v ... ) {
        return { v };
	}

	inline _any_t any() { return{}; }

	// create callback expression
	template < class _exp_t >
	inline auto usage(const _exp_t & exp) {
		return _usage_op < _exp_t > ( exp );
	}

	// parser 
	template < class settings_t, class _exp_t = void >
	inline bool parse(const _exp_t & exp, int arg_c, const char ** arg_v) {
		auto s_range = _range_t {
			+arg_v + 1,	// ignore first item
			+arg_v + arg_c
		};

		_cxt_t cxt = { s_range };

		// evaluate solutions
		exp.eval(cxt);

		// process and execute solutions
		return _execute < settings_t > (cxt, s_range);
	}

}

struct demo {
    
    bool show_version;
    bool show_help;
    
    vector < string > source_files;
    
};

void show_help (demo & d) {
    std::cout << "demo show help" << std::endl;
}

void show_version(demo & d) {
    std::cout << "demo show version" << std::endl;
}

void process (demo & d) {
    std::cout << "demo process files" << std::endl;
}

int main(int arg_c, char * arg_v[]) {

	using namespace command_line;
	
	const char * args[] = { "garbage", "ficheiro1", "ficheiro2", "ficheiro3" };
	
    auto cmd_expr =
        usage ( option ("-v", "--version") ) [&show_version] |
        usage ( option ("-h", "--help") ) [&show_help] |
        usage ( +(any ())[&demo::source_files] ) [&process];
	
	parse < demo > (cmd_expr, 4, args);
	
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
