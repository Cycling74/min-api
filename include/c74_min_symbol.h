/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74 { namespace min {

	class atom;


	/// A hash function using the Murmur3 algorithm ( https://en.wikipedia.org/wiki/MurmurHash ).
	/// This hash function is capable of being executed at compile time,
	/// meaning that the compiled binary will have a constant int value and no actually need to execute any code at runtime.
	/// @param	str		A c-string to be hashed into an int.
	/// @param	seed	An optional seed value.  For most uses you should not override the default.
	/// @return			An int (specifically a uint32_t) representing the hash of the string input.

	constexpr inline uint32_t hash(const char* const str, const uint32_t seed = 0xAED123FD) noexcept {
		return Murmur3_32(str, _StringLength(str), seed);
	}


	/// A Max symbol represents a string that is cached in a lookup table to speed up string comparisons / usage.
	//	This is a lightweight wrapper for a Max t_symbol instance.
	//  We do not inherit from that type because we need to be able to compare
	//  the pointer to the t_symbol instance as returned by calls to gensym()

	class symbol {
	public:
		/// The default constructor produces an empty symbol (no chars) or, optionally, a unique random symbol.
		/// @param unique	If true then produce a unique/random symbol instead of an empty symbol.

		symbol(bool unique = false) {
			if (unique)
				s = max::symbol_unique();
			else
				s = max::gensym("");
		}


		/// Constructor with an initial value (of any assignable type)
		/// @param value	Value of an assignable type (e.g. some sort of string or symbol)

		symbol(const max::t_symbol* value) {
			s = const_cast<max::t_symbol*>(value);
		}


		/// Constructor with an initial value (of any assignable type)
		/// @param value	Value of an assignable type (e.g. some sort of string or symbol)

		symbol(const char* value) {
			s = max::gensym(value);
		}


		/// Constructor with an initial value (of any assignable type)
		/// @param value	Value of an assignable type (e.g. some sort of string or symbol)

		symbol(const std::string& value) {
			s = max::gensym(value.c_str());
		}


		/// Constructor with an integer value that will be turned into a symbol.
		/// @param value	Initial value that will be stringified.

		symbol(int number_to_be_symbolized)
		: symbol(std::to_string(number_to_be_symbolized)) {}


		/// Constructor with an initial value (of any assignable type)
		/// @param value	Value of an assignable type (e.g. some sort of string or symbol)

		symbol(const atom& value);


		symbol& operator=(max::t_object* value) {
			s->s_thing = value;
			return *this;
		}

		symbol& operator=(max::t_symbol* value) {
			s = value;
			return *this;
		}

		symbol& operator=(const char* value) {
			s = max::gensym(value);
			return *this;
		}

		symbol& operator=(const std::string& value) {
			s = max::gensym(value.c_str());
			return *this;
		}

		symbol& operator=(const atom& value);


		friend bool operator==(const symbol& lhs, const symbol& rhs) {
			return lhs.s == rhs.s;
		}

		friend bool operator==(const symbol& lhs, const char* rhs) {
			return lhs.s == max::gensym(rhs);
		}


		operator max::t_symbol*() const {
			return s;
		}

		operator std::string &&() const {
			return std::move(std::string(s->s_name));
		}

		operator const char*() const {
			return s->s_name;
		}

		operator max::t_dictionary*() const {
			static max::t_symbol* ps_dictionary = nullptr;
			if (!ps_dictionary)
				ps_dictionary = max::gensym("dictionary");
			if (object_classname_compare(s->s_thing, ps_dictionary))
				return s->s_thing;
			else
				return nullptr;
		}

		operator void*() const {
			return s->s_thing;
		}

		operator bool() const {
			return s->s_thing != nullptr;
		}

		operator float() const = delete;

		const char* c_str() const {
			return s->s_name;
		}

	private:
		max::t_symbol* s;
	};


	/// Expose symbol for use in std output streams.
	template<class charT, class traits>
	std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream, const symbol& s) {
		return stream << static_cast<const char*>(s);
	}


#ifdef __APPLE__
#pragma mark -
#pragma mark Cache of Pre-Defined Symbols
#endif

	static const symbol k_sym_box{"box"};            ///< The symbol "box", which is the max-namespace for normal user-facing classes.
	static const symbol k_sym__empty{""};            ///< The special empty symbol which contains no chars at all.
	static const symbol k_sym__pound_d{"#D"};        ///< The special "#D" symbol used for accessing an object's dictionary in the patcher.
	static const symbol k_sym_float32{"float32"};    ///< The symbol "float32".
	static const symbol k_sym_float64{"float64"};    ///< The symbol "float64".
	static const symbol k_sym_getmatrix{"getmatrix"};    ///< The symbol "getmatrix".
	static const symbol k_sym_long{"long"};              ///< The symbol "long".
	static const symbol k_sym_modified{"modified"};      ///< The symbol "modified".
	static const symbol k_sym_symbol{"symbol"};          ///< The symbol "symbol".
	static const symbol k_sym_list{"list"};              ///< The symbol "list".
	static const symbol k_sym_bang{"bang"};              ///< The symbol "bang".
	static const symbol k_sym_getname{"getname"};        ///< The symbol "getname".
	static const symbol k_sym_max{"max"};                ///< The symbol "max" -- the max object.
	static const symbol k_sym_size{"size"};              ///< Cached symbol "size"
	static const symbol k_sym_time{"time"};              ///< The symbol "time".

	static const symbol k_sym_globalsymbol_binding{"globalsymbol_binding"};        ///< Cached symbol "globalsymbol_binding"
	static const symbol k_sym_binding{"binding"};                                  ///< Cached symbol "binding"
	static const symbol k_sym_globalsymbol_unbinding{"globalsymbol_unbinding"};    ///< Cached symbol "globalsymbol_unbinding"
	static const symbol k_sym_unbinding{"unbinding"};                              ///< Cached symbol "unbinding"
	static const symbol k_sym_buffer_modified{"buffer_modified"};                  ///< Cached symbol "buffer_modified"


}}    // namespace c74::min
