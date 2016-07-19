/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class texteditor {
	public:
		
		using textfunction = std::function<void(const char*)>;
		
		texteditor(object_base* an_owner, textfunction fn)
		: m_owner(an_owner)
		, m_callback(fn)
		{}
		
		
		~texteditor() {
			// TODO: Do I need this ????
			// object_free(instance);
		}
		
		
		void open(const char* contents) {
			if (!m_jed) {
				m_jed = c74::max::object_new(c74::max::CLASS_NOBOX, c74::max::gensym("jed"), (max::t_object*)m_owner, 0);
				object_attr_setsym(m_jed, c74::max::gensym("title"), c74::max::gensym("Code Editor"));
				object_attr_setchar(m_jed, c74::max::gensym("scratch"), 1);
				
				object_method_direct(void, (c74::max::t_object* , const char*, c74::max::t_symbol*),
									 m_jed, c74::max::gensym("settext"), contents, c74::max::gensym("utf-8"));
			}
			else
				c74::max::object_attr_setchar(m_jed, c74::max::gensym("visible"), 1);
		}
		
		void open(std::string& contents) {
			open(contents.c_str());
		}
		
		
	private:
		object_base*		m_owner = nullptr;
		textfunction		m_callback;
		c74::max::t_object*	m_jed = nullptr;

		
		c74::min::method edclose_meth = { m_owner, "edclose", MIN_FUNCTION {
			m_jed = nullptr;
			return {};
		}};

		
		c74::min::method okclose_meth = { m_owner, "okclose", MIN_FUNCTION {
			char* text = nullptr;
			
			object_method(m_jed, c74::max::gensym("gettext"), &text);
			if (text != nullptr) {
				m_callback(text);
				c74::max::sysmem_freeptr(text);
			}
			return {};
		}};

	};
	
}} // namespace c74::min
