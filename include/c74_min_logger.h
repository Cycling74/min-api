/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once


namespace c74 {
namespace min {


	/// Logging utility to deliver output messages.
	/// In most cases the logged output will be delivered to the Max window.
	class logger {
	public:

		/// The output type of the message.
		/// These are not `levels` as in some languages (e.g. Ruby) but distinct targets.
		enum type {
			message = 0		///< A regular console post to the Max Window.
			,warning		///< A highlighted warning post to the Max Window.
			,error			///< A highlighted and trappable error post to the Max Window.
			,console		///< A console post to the OS system console.
			,obtrusive		///< A pop-up error in the patcher windows, to be used exceedingly sparingly.
		};


		/// Constructor: typically you do not call this directly, but via the post() method of object_base
		/// @param obj	The Max object header for your instance
		/// @param type	The type of console output to deliver
		logger(max::t_object* obj, logger::type type = type::message)
		: maxobj(obj)
		, target(type)
		{}
		
		
		// No copy constructor
		logger(logger& other) = delete;
		
		
		// Move constructor is required because we return our instance, by value,
		// from the post() method of object_base
		logger(logger&& other)
		: maxobj(other.maxobj)
		, target(other.target)
		, stream(std::move(other.stream))
		{}
		
		
		// The call to post() is complete, thus our message is complete and we go out of scope,
		// meaning we can now deliver the message.
		~logger() {
			const char* msgstr = stream.str().c_str();

			switch(target) {
				case message:
					max::object_post(maxobj, msgstr);
					break;
				case warning:
					max::object_warn(maxobj, msgstr);
					break;
				case error:
					max::object_error(maxobj, msgstr);
					break;
				case console:
					max::cpost(msgstr);
					break;
				case obtrusive:
					max::object_error_obtrusive(maxobj, msgstr);
					break;
			}
		}


		/// Use the insertion operator as for any other stream to build the output message
		/// @param	x	A token to be added to the output stream. d
		template <typename T>
		logger& operator<<(const T& x) {
			stream << x;
			return *this;
		}


	private:
		max::t_object*		maxobj;
		logger::type		target;
		std::stringstream	stream;
	};

}} // namespace c74::min
