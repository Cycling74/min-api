/// @file
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once


namespace c74 {
namespace min {


	// time_interval is a hybrid object that can represent a time value
	// but also can be an actor by implementing the internals used by Max's ITM system.
	//
	// This allows us to pass a time_interval as a value
	// but also allows us to specialize attributes which create a time_interval member that interfaces with ITM.

	class time_interval {
	public:
		time_interval(object_base* owner, symbol attrname, double initial_interval)
		: m_owner		{ owner }
		, m_name		{ attrname }
		, m_timeobj		{ nullptr }
		{
			if (owner->maxobj())
				m_timeobj = (max::t_object*)max::time_new(owner->maxobj(), attrname, nullptr, 0);
			set_milliseconds(initial_interval);
		}

		time_interval(double interval_in_ms)
		: m_owner	{ nullptr }
		, m_timeobj	{ nullptr }
		{
			set_milliseconds(interval_in_ms);
		}

		time_interval(const atom& interval_in_ms)
		: m_owner	{ nullptr }
		, m_timeobj	{ nullptr }
		{
			set_milliseconds(interval_in_ms);
		}

		time_interval()
		: m_owner	{ nullptr }
		, m_timeobj	{ nullptr }
		{}

		~time_interval() {
			max::object_free(m_timeobj);
		}

		// The copy constructor and copy assignment are not straight forward because
		// this class is attempting to represent two related but different concerns.
		//
		// Ideally we could delete the copy ctor but it would require changes to all attributes
		// e.g. to set defaults by const ref

		time_interval(const time_interval& other)
		: m_owner		{ other.m_owner }
		, m_name		{ other.m_name }
		, m_timeobj		{ nullptr }		// we cannot copy the timeobj or we will potentially double-free it
		, m_interval_ms	{ other.m_interval_ms }
		{}

		time_interval& operator = (const time_interval& other) {
			// do not overwrite anything!
			// we just want to set the time for the existing timeobj
			(*this) = double(other);
			return *this;
		}

		time_interval& operator = (double value) {
			set_milliseconds(value);
			return *this;
		}

		time_interval& operator = (const atom& value) {
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

	private:
		object_base*	m_owner;
		symbol			m_name;
		max::t_object*	m_timeobj;
		double			m_interval_ms = 0;

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

}} // namespace c74::min
