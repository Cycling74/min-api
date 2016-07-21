/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once


namespace c74 {
namespace min {

	
	class logger_line_ending {};	/// A type to represent line endings for the logger class.
	logger_line_ending endl;		/// An instance of a line ending for convenience
	
	
	/// Logging utility to deliver console messages
	/// in such a way that they are mirrored to the Max window.
	class logger {
	public:

		
		/// The output type of the message.
		/// These are not `levels` as in some languages (e.g. Ruby) but distinct targets.
		enum type {
			message = 0,	///< A regular console post to the Max Window.
			error			///< A highlighted and trappable error post to the Max Window.
		};


		/// Constructor: typically you do not call this directly,
		/// it used by min::object to create cout and cerr
		/// @param an_owner		Your object instance
		/// @param type			The type of console output to deliver
		logger(object_base* an_owner, logger::type type)
		: owner		{ *an_owner }
		, target	{ type }
		{}
		

		/// Use the insertion operator as for any other stream to build the output message
		/// @param	x	A token to be added to the output stream. d
		template<typename T>
		logger& operator<<(const T& x) {
			stream << x;
			return *this;
		}


		/// Pass endl to the insertion operator to complete the console post and flush it.
		/// @param x	The min::endl token
		logger& operator<<(const logger_line_ending& x) {
			const std::string& s = stream.str();
			
			switch(target) {
				case message:
					std::cout << s << std::endl;
					if (owner.initialized())
						max::object_post(owner, s.c_str());
					break;
				case error:
					std::cerr << s << std::endl;
					if (owner.initialized())
						max::object_error(owner, s.c_str());
					break;
			}

			stream.str("");
			return *this;
		}

	private:
		object_base&		owner;
		logger::type		target;
		std::stringstream	stream;
	};
	

}} // namespace c74::min
