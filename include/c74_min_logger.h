/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

#include <ostream>
#include <streambuf>
namespace c74::min {

	// for backword compatibility. min::endl is no longer a empty class but an alias to std::endl.
	using std::endl;

	/// Logging utility to deliver console messages
	/// in such a way that they are mirrored to the Max window.
	///
	/// This class is not intended to be used directly,
	/// but rather through instances that are provided in min::object<> base class.
	///
	/// @see min::object::cout
	/// @see min::object::cerr
	/// @see min::object::cwarn
	/// @see min::endl


	class loggerbuf : public std::basic_stringbuf<char> {
	public:
		/// The output type of the message.
		/// These are not `levels` as in some languages (e.g. Ruby) but distinct targets.

		enum class type {
			message = 0,    ///< A regular console post to the Max Window.
			warning,        ///< A highlighted and trappable warning post to the Max Window.
			error           ///< A highlighted and trappable error post to the Max Window.
		};


		/// Constructor: typically you do not call this directly,
		/// it used by min::object to create cout and cerr
		/// @param an_owner		Your object instance
		/// @param a_type		The type of console output to deliver

		loggerbuf(object_base* an_owner, loggerbuf::type a_type)
		: std::basic_stringbuf<char>()
		, m_owner {*an_owner}
		, m_target {a_type} { }

		virtual int sync() override {
			const std::string& s = str();

			switch (m_target) {
				case type::message:
					std::cout << s;

					// if the max object is present then it is safe to post even if the owner isn't yet fully initialized
					if (m_owner.initialized() || k_sym_max)
						max::object_post(m_owner, s.c_str());
					break;
				case type::warning:
					std::cerr << s;

					// if the max object is present then it is safe to post even if the owner isn't yet fully initialized
					if (m_owner.initialized() || k_sym_max)
						max::object_warn(m_owner, s.c_str());
					break;
				case type::error:
					std::cerr << s;
					// if the max object is present then it is safe to post even if the owner isn't yet fully initialized
					if (m_owner.initialized() || k_sym_max)
						max::object_error(m_owner, s.c_str());
					break;
			}
			str("");
			return 0;
		}

	private:
		object_base&    m_owner;
		loggerbuf::type m_target;
	};

	class logger : public std::ostream {
	public:
		logger() = delete;
		explicit logger(object_base* an_owner, loggerbuf::type a_type)
		: buf(an_owner, a_type)
		, std::ostream(&buf) { }
		using type = loggerbuf::type;

	private:
		loggerbuf buf;
	};

}    // namespace c74::min
