/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	class inlet_base : public port {
		friend void object_base::create_inlets();
	public:
		inlet_base(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
		: port { an_owner, a_description, a_type}
		{}
	private:
		void* m_instance { nullptr };
	};


	enum class inlet_trigger {
		automatic,
		hot,
		cold
	};

	template<inlet_trigger inlet_trigger_type = inlet_trigger::automatic>
	class inlet : public inlet_base {
	public:
		inlet(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
		: inlet_base { an_owner, a_description, a_type}
		{
			m_owner->inlets().push_back(this);
		}
	};

}} // namespace c74::min
