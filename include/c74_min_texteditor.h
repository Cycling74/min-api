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
		: owner(an_owner)
		, callback(fn)
		{}
		
		
		~texteditor() {
			// TODO: Do I need this ????
			// object_free(instance);
		}
		
		
		void open(const char* contents) {
			if (!jed) {
				jed = c74::max::object_new(c74::max::CLASS_NOBOX, c74::max::gensym("jed"), owner->maxobj, 0);
				object_attr_setsym(jed, c74::max::gensym("title"), c74::max::gensym("Code Editor"));
				object_attr_setchar(jed, c74::max::gensym("scratch"), 1);
				
				object_method_direct(void, (c74::max::t_object* , const char*, c74::max::t_symbol*),
									 jed, c74::max::gensym("settext"), contents, c74::max::gensym("utf-8"));
			}
			else
				c74::max::object_attr_setchar(jed, c74::max::gensym("visible"), 1);
		}
		
		void open(std::string& contents) {
			open(contents.c_str());
		}
		
		
	private:
		object_base*		owner;
		textfunction		callback;
		c74::max::t_object*	jed = nullptr;

		
		c74::min::method edclose_meth = { owner, "edclose", [this](c74::min::atoms& args) {
			jed = nullptr;
		}};

		
//		METHOD (okclose) {
		c74::min::method okclose_meth = { owner, "okclose", [this](c74::min::atoms& args) {
			char* text = nullptr;
			
			object_method(jed, c74::max::gensym("gettext"), &text);
			if (*text == 0)
				return;	// totally blank text editor
			
//			atoms as = {atom(c74::max::gensym("anonymous")), atom(c74::max::gensym(text))};
//			define(as);
			callback(text);
			
			if (text)
				c74::max::sysmem_freeptr(text);
		}};
//		END


	};
	
}} // namespace c74::min
