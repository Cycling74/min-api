/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74 { namespace min {

	enum class time_flags {
		none = 0,									/// anything goes -- both fixed times and tempo-based times
		tempo_based = max::TIME_FLAGS_TICKSONLY		/// only tempo-based times permitted: ticks, bbu, and notevalues
	};

	using time_callback = std::function<void()>;

	using time_quantization = attribute<time_value>;

	class time_value;
	void time_value_callback(time_value* self);

	// time_interval is a hybrid object that can represent a time value
	// but also can be an actor by implementing the internals used by Max's ITM system.
	//
	// This allows us to pass a time_interval as a value
	// but also allows us to specialize attributes which create a time_interval member that interfaces with ITM.

	class time_value {
	public:

		/// constructor
		/// @param	owner				The object (i.e. the instance of a subclass of min::object<>) to whom this belongs.
		/// @param	attrname			The name of the Max attribute
		/// @param	initial_interval	The initial interval in milliseconds
		/// @param	finalize			The constructor should finalize creation automatically (default is true)
		///
		///	@see	finalize()

		time_value(object_base* owner, symbol attrname, double initial_interval, bool finalize = true)
		: m_owner		{ owner }
		, m_name		{ attrname }
		, m_timeobj		{ nullptr }
		{
			if (finalize)
				this->finalize();
			if (owner->maxobj())
				m_timeobj = max::time_new(owner->maxobj(), attrname, nullptr, 0);
			set_milliseconds(initial_interval);
		}

		time_value(double interval_in_ms)
		: m_owner{nullptr}
		, m_timeobj{nullptr} {
			set_milliseconds(interval_in_ms);
		}

		time_value(const atom& interval_in_ms)
		: m_owner{nullptr}
		, m_timeobj{nullptr} {
			set_milliseconds(interval_in_ms);
		}

		time_value()
		: m_owner{nullptr}
		, m_timeobj{nullptr} {}

		~time_value() {
			max::object_free(m_timeobj);
		}

		/// if the constructor did not finalize the creation of its members (e.g. creating the timeobj)
		/// then you must manually call this function after setting up the flags, callbacks, etc.
		void finalize() {
			if (m_owner->maxobj()) {
				auto		flags = static_cast<long>(m_flags);
				max::method	callback = nullptr;

				if (m_callback) {
					flags |= max::TIME_FLAGS_USECLOCK;
					callback = (max::method)time_value_callback;
				}
				m_timeobj = (max::t_object*)max::time_new_custom(m_owner->maxobj(), m_name, callback, flags, this);
			}
		}

		// The copy constructor and copy assignment are not straight forward because
		// this class is attempting to represent two related but different concerns.
		//
		// Ideally we could delete the copy ctor but it would require changes to all attributes
		// e.g. to set defaults by const ref

		time_value(const time_value& other)
		: m_owner{other.m_owner}
		, m_name{other.m_name}
		, m_timeobj{nullptr}    // we cannot copy the timeobj or we will potentially double-free it
		, m_interval_ms{other.m_interval_ms} {}

		time_value& operator=(const time_value& other) {
			// do not overwrite anything!
			// we just want to set the time for the existing timeobj
			(*this) = static_cast<double>(other);
			return *this;
		}

		time_value& operator=(double value) {
			set_milliseconds(value);
			return *this;
		}

		time_value& operator=(const atom& value) {
			set_milliseconds(value);
			return *this;
		}

		operator double() const {
			return get_milliseconds();
		}

		operator atom() const {
			return atom(get_milliseconds());
		}

		void print() const {
			std::cout << "TIME_INTERVAL this: " << this << " timeobj: " << m_timeobj << std::endl;
		}


		// TODO: const is a lie -- we are in-fact altering our state
		void start() const {}

		// TODO: const is a lie -- we are in-fact altering our state
		void stop() const {}


		void define_flags(time_flags flags) {
			m_flags = flags;
			// TODO: actually apply to the timeobj
			// TODO: need to OR with the callback flag
		}

		void define_callback(const time_callback& callback) {
			m_callback = callback;
			// TODO: need to OR with flags
		}

		void define_quantization(const time_quantization& quantize_attr) {
			m_quantize_attr = &quantize_attr;
		}


	private:
		object_base*				m_owner;
		symbol						m_name;
		max::t_object*				m_timeobj;
		double						m_interval_ms = 0;
		time_flags					m_flags = time_flags::none;
		time_callback				m_callback = nullptr;
		const time_quantization*	m_quantize_attr = nullptr;


		double get_milliseconds() const {
			if (m_timeobj)
				return max::time_getms(m_timeobj);
			else
				return m_interval_ms;
		}


		void set_milliseconds(double value) {
			if (m_timeobj) {
				atom a(value);
				max::time_setvalue(m_timeobj, nullptr, 1, &a);
			}
			m_interval_ms = value;
		}
	};


	void time_value_callback(time_value* self) {
//		a_timer->tick();

		// TODO: get the minobj for the maxobj -- could do this with an A_CANT message in object_owner?
		// TODO: but that isn't enough... what if we have multiple time_value attrs -- we really need a way to get this time_value instance
		//

	}

}} // namespace c74::min
