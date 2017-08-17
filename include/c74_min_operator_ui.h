/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

//#include "c74_jitter.h"

namespace c74 {
namespace min {

	
	class ui_operator_base {};
	

	template<int default_width_type = 20, int default_height_type = 20>
	class ui_operator : public ui_operator_base {
	public:
		explicit ui_operator(object_base* instance, const atoms& args)
		: m_instance { instance }
		{
			if (!box()) // box will be a nullptr when being dummy-constructed
				return;

			long flags = 0
				| c74::max::JBOX_DRAWFIRSTIN		// 0
				| c74::max::JBOX_NODRAWBOX		// 1
				| c74::max::JBOX_DRAWINLAST		// 2
			 //	| JBOX_TRANSPARENT		// 3
			 //	| JBOX_NOGROW			// 4
			 //	| JBOX_GROWY			// 5
				| c74::max::JBOX_GROWBOTH			// 6
			 //	| JBOX_IGNORELOCKCLICK	// 7
			 //	| JBOX_HILITE			// 8
				| c74::max::JBOX_BACKGROUND		// 9
			 //	| JBOX_NOFLOATINSPECTOR	// 10
			 // | c74::max::JBOX_TEXTFIELD		// 11
				| c74::max::JBOX_MOUSEDRAGDELTA	// 12
			 //	| JBOX_COLOR			// 13
			 //	| JBOX_BINBUF			// 14
			 //	| JBOX_DRAWIOLOCKED		// 15
			 //	| JBOX_DRAWBACKGROUND	// 16
			 //	| JBOX_NOINSPECTFIRSTIN	// 17
			 //	| JBOX_DEFAULTNAMES		// 18
			 //	| JBOX_FIXWIDTH			// 19
			;

			c74::max::jbox_new(box(), flags, static_cast<long>(args.size()), static_cast<const c74::max::t_atom*>(&args[0]));
			box()->b_firstin = m_instance->maxobj();
		}


		virtual ~ui_operator() {
			if (box())  // box will be a nullptr when being dummy-constructed
				jbox_free(box());
		}


		void redraw() {
			jbox_redraw(box());
		}


		int default_width() const {
			return default_width_type;
		}


		int default_height() const {
			return default_height_type;
		}


	private:
		object_base* m_instance;

		c74::max::t_jbox* box() const {
			return reinterpret_cast<c74::max::t_jbox*>(m_instance->maxobj());
		}
	};


	// If you inherit from ui_operator then define this function to
	// add ui support to the Max class.
	// Will be called from define_min_external()

	template<class min_class_type>
	typename enable_if< is_base_of<ui_operator_base, min_class_type>::value >::type
	wrap_as_max_external_ui(max::t_class* c, min_class_type& instance) {
		long flags {};

		// flags |= c74::max::JBOX_TEXTFIELD;
		jbox_initclass(c, flags);
		c->c_flags |= c74::max::CLASS_FLAG_NEWDICTIONARY; // to specify dictionary constructor

		string default_patching_rect {"0. 0. "};
		default_patching_rect += std::to_string(instance.default_width());
		default_patching_rect += " ";
		default_patching_rect += std::to_string(instance.default_height());

		auto attr = (c74::max::t_object*)c74::max::class_attr_get(c, c74::max::gensym("patching_rect"));
		auto attr_type = (c74::max::t_symbol*)object_method(attr, c74::max::gensym("gettype"));
		c74::max::class_attr_addattr_parse(c, "patching_rect", "default", attr_type, 0, default_patching_rect.c_str());
	}

}} // namespace c74::min
