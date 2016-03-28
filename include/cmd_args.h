#pragma once
#ifndef _cmd_args_h_
#define _cmd_args_h_

#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <type_traits>

using namespace std;

namespace command_line {

	template < class _t >
	inline bool _cast(const string & orig, _t & dest) {
		stringstream stream(orig);

		_t val = _t ();
		stream >> val;

		if (stream) {
			dest = val;
			return true;
		} else {
			return false;
		}
	}

	inline bool _cast(const string & orig, std::string & dest) {
		dest = orig;
		return true;
	}

	// iterator domain range
    template < class _it_t >
	struct _base_range_t {
        
        using iterator = _it_t;
        
		_it_t it;
		_it_t end;

		// returns true if range is length is zero
		inline bool finished() const { return it == end; }

		// consume first item in range and return it's value
		inline auto consume() {
			return *it++;
		}

		inline static _base_range_t intersect(
			const _base_range_t & v1,
			const _base_range_t & v2
			) {
			if (distance(v1.it, v2.it) > 0)
				return{ v1.it, v2.it };
			else
				return{ v2.it, v1.it };
		}
        
        template < class _t >
        inline static _base_range_t from_source (const _t & source, size_t offset = 0) {
            return {
                std::begin (source) + offset,
                std::end (source)
            };
        }

	};
    
    using _range_t = _base_range_t < char const * const * >;
    
    //  execution node
    template < class _t >
    struct _address_traits;
    
    template < class _t, class _class_t >
    struct _address_traits < _t _class_t::* > {
        using data_type = _t;
        using settings_type = _class_t;
    };
    
    template < class _t >
    struct _address_traits < void (*)( _t & ) > {
        using settings_type = _t;
    };
    
    struct _act_t {
        virtual ~_act_t() {}
    };
    
    
    struct _base_action_ctor {
        using shared = shared_ptr < _base_action_ctor >;
        virtual _act_t * create_action (const string & value) const = 0;
    };
    
    template < class _address_t, class _action_t >
    struct _action_ctor : public _base_action_ctor {
        
        _address_t address;
        
        inline _action_ctor ( _address_t address_v ) : address (address_v) {}
        
        virtual _act_t * create_action (const string & value) const override {
            return new _action_t (address, value);
        }
        
    };

	template < class _settings_t >
	struct _typed_act_t : public _act_t {
        virtual void exec(
            _settings_t & settings
        ) const = 0;
	};

	// action for options
	template <
        class _address_t,
        class _settings_t = typename _address_traits < _address_t >::settings_type
    >
	struct _option_act_t : public _typed_act_t < _settings_t > {
        
        _address_t address;
		_option_act_t(_address_t address_v, const string &) : address(address_v) {}

		virtual void exec(
            _settings_t & settings
        ) const override {
			settings.*address = true;
		}
	};

	// setter action specialization for single items
	template <
        class _address_t,
        class _settings_t = typename _address_traits < _address_t >::settings_type
    >
	struct _setter_act_t : public _typed_act_t < _settings_t > {
        
		_address_t  address;
        string      value;
        
		_setter_act_t(_address_t address_v, const string & v) : address(address_v), value(v) {}

		virtual void exec(
            _settings_t & settings
        ) const override {
			// TODO: evaluate cast result and report
			_cast(value, settings.*address);
		}
	};

	// setter specialization for vectors
	template < class _settings_t, class _item_t, class ... _other_tv >
	struct _setter_act_t <
        std::vector < _item_t, _other_tv...> _settings_t::*
    >
        : public _typed_act_t < _settings_t >
    {
        
		using field_t = std::vector < _item_t, _other_tv...> _settings_t::*;
		field_t address;
        string  value;

		_setter_act_t(field_t address_v, const string & v) : address(address_v), value (v) {}

		virtual void exec(_settings_t & settings) const override {
            auto v = _item_t ();
            _cast (value, v);
            
            (settings.*address).push_back (v);
		}
	};

	// callback action
	template <
        class _address_t,
        class _settings_t = typename _address_traits < _address_t >::settings_type
    >
	struct _call_act_t : public _typed_act_t < _settings_t > {
		_address_t callback;

		_call_act_t(_address_t callback_v, const string &) : callback(callback_v) {}

		virtual void exec(
            _settings_t & settings
        ) const override {
			if (callback)
				callback(settings);
		}
	};

	//  execution node
    using _solution_data = vector < string >;
    
	struct _exec_node {
        _range_t                range;
		unique_ptr < _act_t >   action;
        
		int sequence_index;
	};

	struct _tree_state_t {
		size_t	tree_size;
		int		parent_node;
	};

	const int _tree_t_default_parent_node = -1;

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

		inline _tree_t() : parent_node(_tree_t_default_parent_node) {}

		inline void add_node(const _range_t & rng, _act_t * setter) {
			tree.push_back(item_t{
				rng,
				unique_ptr < _act_t >(setter),
				parent_node++
			});
		}

		inline _tree_state_t state() const {
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
		_range_t		range;
	};

	struct _cxt_t {
		_range_t		range;
		_tree_t			solution_tree;
        
        string          value;

		inline _cxt_state state() const {
			return{
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
	auto operator > (const _lhe_t & l, const _rhe_t & r) {
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

		inline _z_one_op(const _exp_t & e) : exp(e) {}

		inline bool eval(_cxt_t & cxt) const {
			auto state = cxt.state ();

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

		inline _one_n_op(const _exp_t & e) : exp(e) {}

		inline bool eval(_cxt_t & cxt) const {
			bool one_valid = exp.eval(cxt);

			if (one_valid) {
				auto state = cxt.state ();

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

		inline _z_n_op(const _exp_t & e) : exp(e) {}

		template < class cxt_t >
		inline bool eval(cxt_t & cxt) const {
			auto state = cxt.state ();

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
	struct _option_t {

        vector < const char * >     keys;
        _base_action_ctor::shared   action_ctor;

		inline _option_t(const initializer_list< const char * > & keys_v) : keys(keys_v) {}

		inline bool eval(_cxt_t & cxt) const {
			if (cxt.range.finished())
				return false;

			// cache key
            auto it = cxt.range.it;
			auto v = cxt.range.consume();

			for (auto * k : keys) {
                if (strcmp(v, k) == 0) {
                    if (action_ctor){
                        cxt.solution_tree.add_node (
                            _range_t { it, it + 1 },
                            action_ctor->create_action("")
                        );
                    }
                    
                    return true;
                }
			}

			return false;
		}
        
        template < class _address_t >
        inline const _option_t & operator [] ( _address_t address) {
            action_ctor = new _action_ctor < _address_t, _option_act_t < _address_t > > (address);
            return *this;
        }
        
	};

	struct _value_t {

		vector < const char * >     keys;
        mutable _base_action_ctor::shared
                                    action_ctor;

		inline _value_t(const initializer_list < const char * > & keys_v) : keys(keys_v) {}

		inline bool eval(_cxt_t & cxt) const {
			if (cxt.range.finished())
				return false;

			if (keys.size() == 0)
				return false;

			// cache key
            auto it = cxt.range.it;
			auto * v = cxt.range.consume();

			// find '=' key / value separator
			auto * vc = strchr(v, '=');

			if (!vc) {
				// report malformed string
				return false;
			}

			size_t v_len = size_t(vc - v);

			// parse for proper key
			for (const char * k : keys) {
                if (strncmp(v, k, v_len) == 0) {
                    if (action_ctor) {
                        cxt.solution_tree.add_node (
                            _range_t { it, it + 1 },
                            action_ctor->create_action(vc + 1)
                        );
                    }
					return true;
                }
			}

			return false;
		}
        
        template < class _address_t >
        inline const _value_t operator [] ( _address_t address) const {
            action_ctor = new _action_ctor < _address_t, _setter_act_t < _address_t > > (address);
            return *this;
        }
	};

	struct _any_t {
  
        mutable _base_action_ctor::shared action_ctor;

		inline _any_t() {}

		inline bool eval(_cxt_t & cxt) const {
			if (cxt.range.finished())
				return false;

            auto it = cxt.range.it;
			auto v = cxt.range.consume();
            
            if (action_ctor) {
                cxt.solution_tree.add_node (
                    _range_t { it, it + 1 },
                    action_ctor->create_action(v)
                );
            }
			return true;
		}
        
        template < class _address_t >
        inline const _any_t & operator [] ( _address_t address) const {
            action_ctor = new _action_ctor < _address_t, _setter_act_t < _address_t > > (address);
            return *this;
        }

	};

	// usage wrapper
	template < class _exp_t >
	struct _usage_op {

		_exp_t                      exp;
        mutable _base_action_ctor::shared
                                    action_ctor;

		inline bool eval(_cxt_t & cxt) const {
            auto state = cxt.state ();
            
			bool res = exp.eval(cxt);
            if (res && action_ctor) {
                cxt.solution_tree.add_node (
                    _range_t::intersect(state.range, cxt.range),
                    action_ctor->create_action("")
                );
            }
            
            return res;
		}

		inline _usage_op(const _exp_t & e) : exp(e) {}
        
        template < class _address_t >
        inline _usage_op < _exp_t > operator [] ( _address_t address) const {
            action_ctor = new _action_ctor < _address_t, _call_act_t < _address_t > > (address);
            return *this;
        }
	};

	// solution node inversion and execution
	template < class settings_t >
	inline bool _execute(_cxt_t & cxt, _range_t & arg_range, settings_t & settings ) {

		auto & tree = cxt.solution_tree;
		int solution_index = -1;

		for (
			int i = static_cast <int> (tree.size() - 1);
			i >= 0;
			--i
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

			while (i != -1) {
				int t = tree[i].sequence_index;
				tree[i].sequence_index = n;
				n = i;
				i = t;
			}

			// reset position
			i = 0;

			while (i != -1) {
				auto & node = tree[i];
				auto typed_node = dynamic_cast < _typed_act_t < settings_t > * > (node.action.get());

				if (typed_node) {
					typed_node->exec(settings);
				} else {
					// report error
				}

				i = node.sequence_index;
			}
		}

		return true;
	}

	// scanner methods

	template < class ... _char_tv >
	inline _option_t option(const _char_tv * ... v) {
		return{ v... };
	}

	template < class ... _char_t >
	inline _value_t key(const _char_t * ... v) {
		return{ v... };
	}

	//inline _any_t any() { return{}; }
	const _any_t any;

	// create callback expression
	template < class _exp_t >
	inline auto usage(const _exp_t & exp) {
		return _usage_op < _exp_t >(exp);
	}

	// parser 
	template < class settings_t, class _exp_t = void >
	inline bool parse(const _exp_t & exp, int arg_c, char ** arg_v, settings_t & settings ) {

		auto s_range = _range_t {
			+arg_v + 1,	// ignore first item
			+arg_v + arg_c
		};

		_cxt_t cxt = { s_range };

		// evaluate solutions
		exp.eval(cxt);

		// process and execute solutions
		return _execute < settings_t >(cxt, s_range, settings);
	}

	// parser 
	template < class settings_t, class _exp_t = void >
	inline bool parse(const _exp_t & exp, int arg_c, char ** arg_v) {
		settings_t settings = {};
		return parse(exp, arg_c, arg_v, settings);
	}

}

#endif
