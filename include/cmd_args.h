#pragma once
#ifndef _cmd_args_h_
#define _cmd_args_h_

#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;

namespace cmd_args {

	namespace details {

		namespace arguments {

			struct context {
			private:
				vector < string >	_errors;
			public:
				inline bool has_failed() const { return _errors.size() > 0; }

				inline void push_error(const string & key, const string & message) {
					_errors.push_back("[" + key + "] " + message);
				}

				inline void to_stream(ostream & out) const {
					for (auto & e : _errors) {
						out << e << endl;
					}
				}
			};

			struct key_value_pair {
				string
					key,
					value;

				bool handled;
			};

			template < class _config_t >
			struct base_arg_parser {
				vector < string >	keys;
				string				description;

				virtual void parse(
					context & cxt,
					_config_t & inst,
					vector < key_value_pair > & args
					) const = 0;

				virtual void usage_to_stream(ostream & out, int32_t width) const {
					ostringstream str;

					str << "[";

					for (auto it = keys.begin(); it != keys.end(); ++it) {
						if (it != keys.begin())
							str << ", ";
						str << *it;
					}
					str << "] ";

					out << setw(width) << left << str.str();
					out << "= " << description;
				}
			};

			template < class _config_t, class _t >
			struct argument_parser : public base_arg_parser < _config_t > {

				_t _config_t::*		field_ref;
				_t					default_value;
				bool				optional;

				virtual inline void parse(
					context & cxt,
					_config_t & inst,
					vector < key_value_pair > & args
					) const override
				{
					for (auto & arg : args) {
						if (find(keys.begin(), keys.end(), arg.key) != keys.end()) {
							stringstream stream(arg.value);
							_t value;
							stream >> value;
							arg.handled = true;

							if (stream.bad() || stream.fail()) {
								cxt.push_error(arg.key, "unexpected value for argument ");
								value = default_value;
							}

							inst.*field_ref = value;
							return;
						}
					}

					if (optional) {
						inst.*field_ref = default_value;
					}
					else {
						cxt.push_error(keys.front(), "expected argument missing");
					}
				}

				virtual void usage_to_stream(ostream & out, int32_t width) const override {
					base_arg_parser < _config_t >::usage_to_stream(out, width);

					if (optional)
						out << " (optional)";
				}
			};

			template < class _config_t >
			struct option_parser : public base_arg_parser < _config_t > {
				bool _config_t::*	field_ref;

				virtual inline void parse(
					context & cxt,
					_config_t & inst,
					vector < key_value_pair > & args)
					const override
				{
					for (auto & arg : args) {
						if (inst.*field_ref = find(keys.begin(), keys.end(), arg.key) != keys.end()) {
							arg.handled = true;
							return;
						}
					}

					inst.*field_ref = false;
				}
			};

		}
	}

	template < class _config_t >
	inline unique_ptr < details::arguments::option_parser < _config_t > > option(bool _config_t::*field_ref, const vector < string > & keys, const string & description = "") {
		auto parser = make_unique < details::arguments::option_parser < _config_t > >();
		parser->keys = keys;
		parser->field_ref = field_ref;
		parser->description = description;
		return parser;
	}

	template < class _config_t, class _t >
	inline unique_ptr <  details::arguments::argument_parser < _config_t, _t > > argument(_t _config_t::*field_ref, const vector < string > & keys, const string & description = "", bool optional = true, const _t & default_value = _t()) {
		auto parser = make_unique < details::arguments::argument_parser < _config_t, _t > >();
		parser->keys = keys;
		parser->field_ref = field_ref;
		parser->description = description;
		parser->default_value = default_value;
		parser->optional = optional;
		return parser;
	}

	template < class _config_t >
	struct parser {
	private:
		vector < unique_ptr < details::arguments::base_arg_parser < _config_t > > > _parsers;
	public:

		inline parser < _config_t > & operator + (unique_ptr < details::arguments::base_arg_parser < _config_t > > && arg) {
			_parsers.push_back(move(arg));
			return *this;
		}

		inline  details::arguments::context parse(int arg_c, char * arg_v[], _config_t & res) {
			details::arguments::context cxt;
			// parse arguments
			vector < details::arguments::key_value_pair > args;

			for (int i = 1; i < arg_c; ++i) {

				string item = arg_v[i];

				auto n = item.find_first_of('=');
				if (n == string::npos) {
					args.push_back({
						item,
						"",
						false
					});
				}
				else {
					args.push_back({
						item.substr(0, n),
						item.substr(n + 1),
						false
					});
				}
			}

			// parse argument values
			for (auto & p : _parsers) {
				p->parse(cxt, res, args);
			}

			// check for unhandled arguments
			for (auto & arg : args) {
				if (!arg.handled) {
					cxt.push_error(arg.key, "unexpected argument");
				}
			}

			return cxt;
		}

		void usage_to_stream(ostream & out) const {

			// evaluate key column length
			int32_t max_width = 0;
			for (auto & p : _parsers) {
				int32_t width = 0;

				for (auto & k : p->keys) {
					width += k.size();
				}

				// account for additional chars
				width += 2 * p->keys.size();

				if (width > max_width)
					max_width = width;
			}

			for (auto & p : _parsers) {
				p->usage_to_stream(out, max_width + 1);
				out << endl;
			}
		}
	};

}

#endif
