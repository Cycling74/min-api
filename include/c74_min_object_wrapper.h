/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74 { namespace min {


	// defined in c74_min_doc.h -- generates an updated maxref if needed

	template<class T>
	void doc_update(const T&, const std::string&, const std::string&);


	// All objects use A_GIMME signature for construction.
	// However, all Min classes may not define a constructor to handle those arguments.

	// Class has constructor -- The arguments will be handled manually

	template<class min_class_type, typename enable_if<std::is_constructible<min_class_type, atoms>::value, int>::type = 0>
	void min_ctor(minwrap<min_class_type>* self, const atoms& args) {
		new (&self->m_min_object) min_class_type(args);    // placement new
	}

	// Class has no constructor -- Handle the arguments automatically

	template<class min_class_type, typename enable_if<!std::is_constructible<min_class_type, atoms>::value, int>::type = 0>
	void min_ctor(minwrap<min_class_type>* self, const atoms& args) {
		new (&self->m_min_object) min_class_type;    // placement new
		self->m_min_object.process_arguments(args);
	}


	template<class min_class_type>
	minwrap<min_class_type>* wrapper_new(max::t_symbol* name, long ac, max::t_atom* av) {
		try {
			atom_reference args(ac, av);
			long           attrstart = attr_args_offset(static_cast<short>(args.size()), args.begin());    // support normal arguments
			auto           self      = static_cast<minwrap<min_class_type>*>(max::object_alloc(this_class));
			auto           self_ob   = reinterpret_cast<max::t_object*>(self);

			self->m_min_object.assign_instance(self_ob);    // maxobj needs to be set prior to placement new

			min_ctor<min_class_type>(self, atoms(args.begin(), args.begin() + attrstart));
			self->m_min_object.postinitialize();
			self->m_min_object.set_classname(name);

			self->setup();

			if (self->m_min_object.is_ui_class()) {
				max::t_dictionary* d = object_dictionaryarg(ac, av);
				if (d) {
					max::attr_dictionary_process(self, d);
					max::jbox_ready((max::t_jbox*)self);
				}
			}
			else {
				max::object_attach_byptr_register(
					self, self, k_sym_box);    // so that objects can get notifications about their own attributes
				max::attr_args_process(self, static_cast<short>(args.size()), args.begin());
			}
			return self;
		}
		catch (std::runtime_error& e) {
			max::object_error(nullptr, e.what());
			return nullptr;
		}
	}


	template<class min_class_type>
	void wrapper_free(minwrap<min_class_type>* self) {
		self->cleanup();    // cleanup routine specific to each type of object (e.g. to call dsp_free() for audio objects)
		self->m_min_object.~min_class_type();    // placement delete
	}


	template<class min_class_type>
	void wrapper_method_assist(minwrap<min_class_type>* self, void* b, long m, long a, char* s) {
		if (m == 2) {
			const auto& outlet = self->m_min_object.outlets()[a];
			strncpy(s, outlet->description().c_str(), 256);
		}
		else {
			if (!self->m_min_object.inlets().empty()) {
				const auto& inlet = self->m_min_object.inlets()[a];
				strncpy(s, inlet->description().c_str(), 256);
			}
		}
	}


	// Return the pointer to 'self' ... that is to say the pointer to the instance of our Max class
	// In the Jitter case there is a version of this function that returns instead the instance of the Jitter class
	template<class min_class_type, enable_if_not_matrix_operator<min_class_type> = 0>
	minwrap<min_class_type>* wrapper_find_self(max::t_object* obj) {
		return reinterpret_cast<minwrap<min_class_type>*>(obj);
	}

	template<class min_class_type, enable_if_matrix_operator<min_class_type> = 0>
	minwrap<min_class_type>* wrapper_find_self(max::t_object* mob) {
		auto job = max::max_jit_obex_jitob_get(mob);
		if (!job)
			job = mob;
		return static_cast<minwrap<min_class_type>*>(job);
	}


	template<class min_class_type, class message_name_type>
	void wrapper_method_zero(max::t_object* o) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];

		meth();
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_int(max::t_object* o, max::t_atom_long v) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		atoms as   = {v};

		meth(as);
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_float(max::t_object* o, double v) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		atoms as   = {v};

		meth(as);
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_symbol(max::t_object* o, max::t_symbol* v) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		atoms as   = {symbol(v)};

		meth(as);
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_anything(max::t_object* o, max::t_symbol* s, long ac, max::t_atom* av) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		atoms as(ac + 1);

		as[0] = s;
		for (auto i = 0; i < ac; ++i)
			as[i + 1] = av[i];
		meth(as);
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_ptr(max::t_object* o, void* v) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		atoms as   = {v};

		meth(as);
	}

	template<class min_class_type>
	void wrapper_method_savestate(max::t_object* o, max::t_dictionary* d) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()["savestate"];
		atoms as   = {d};
		meth(as);
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_self_ptr(max::t_object* o, void* arg1) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		atoms as{o, arg1};

		meth(as);
	}

	template<class min_class_type, class message_name_type>
	void* wrapper_method_oksize(max::t_object* o, void* arg1) {
		auto  self = wrapper_find_self<min_class_type>(o);

		// this method can be called by the ctor before the object is actually constructed
		if ( self->m_min_object.messages().empty() )
			return 0;
		else {
			auto& meth = *self->m_min_object.messages()[message_name_type::name];
			atoms as{arg1};
			atoms r = meth(as);
			return r[0];
		}
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_paint(max::t_object* o, void* arg1) {
		if (is_base_of<ui_operator_base, min_class_type>::value) {
			auto  self = wrapper_find_self<min_class_type>(o);
			auto& ui_op = const_cast<ui_operator_base&>(dynamic_cast<const ui_operator_base&>(self->m_min_object));
			auto& meth = *self->m_min_object.messages()[message_name_type::name];
			atoms as{ o, arg1 };

			ui_op.update_colors();
			meth(as);
		}
		else
			wrapper_method_self_ptr< min_class_type, message_name_type>(o, arg1);
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_self_ptr_pt_long(max::t_object* o, void* arg1, max::t_pt arg2, max::t_atom_long arg3) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		atoms as{o, arg1, arg2.x, arg2.y, arg3};

		meth(as);
	}

	template<class min_class_type, class message_name_type>
	max::t_max_err wrapper_method_self_sym_sym_ptr_ptr___err(max::t_object* o, max::t_symbol* s1, max::t_symbol* s2, void* p1, void* p2) {
		auto  self = wrapper_find_self<min_class_type>(o);

		// This supports notify methods for UI objects which don't actually have a notify method member in the min class
		if (self->m_min_object.messages().find(message_name_type::name) != self->m_min_object.messages().end()) {
			auto& meth = *self->m_min_object.messages()[message_name_type::name];
			atoms as{o, s1, s2, p1, p2};    // NOTE: self could be the jitter object rather than the max object -- so we pass `o` which is
											// always the correct `self` for box operations
			auto ret = meth(as);
			if (!ret.empty())
				return static_cast<long>(meth(as).at(0));
		}
		return 0;
	}

	template<class min_class_type, class message_name_type>
	max::t_max_err wrapper_method_notify(max::t_object* o, max::t_symbol* s1, max::t_symbol* s2, void* p1, void* p2) {
		if (is_base_of<ui_operator_base, min_class_type>::value) {
			auto err = wrapper_method_self_sym_sym_ptr_ptr___err<min_class_type, message_name_type>(o, s1, s2, p1, p2);
			if (!err)
				return c74::max::jbox_notify(reinterpret_cast<c74::max::t_jbox*>(o), s1, s2, p1, p2);
			else
				return err;
		}
		else
			return wrapper_method_self_sym_sym_ptr_ptr___err<min_class_type, message_name_type>(o, s1, s2, p1, p2);
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_self_ptr_long_ptr_long_ptr_long(max::t_object* o, void* arg1, max::t_atom_long arg2, max::t_atom_long* arg3,
		max::t_atom_long arg4, max::t_atom_long* arg5, max::t_atom_long arg6) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		atoms as{o, arg1, arg2, arg3, arg4, arg5, arg6};    // NOTE: self could be the jitter object rather than the max object -- so we
															// pass `o` which is always the correct `self` for box operations
		meth(as);
	}

	template<class min_class_type, class message_name_type>
	void wrapper_method_getplaystate(max::t_object* o, long* play, double* pos, long* loop) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		atoms as = meth();

		assert(as.size() == 3);
		*play = as[0];
		*pos = as[1];
		*loop = as[2];
	}

	// dictionary is a very special case because of the reference counting
	template<class min_class_type, class message_name_type>
	void wrapper_method_dictionary(max::t_object* o, max::t_symbol* s) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[message_name_type::name];
		auto  d    = dictobj_findregistered_retain(s);
		atoms as   = {atom(d)};

		meth(as);
		if (d)
			dictobj_release(d);
	}

	// this version is called for most message instances defined in the min class
	template<class min_class_type>
	void wrapper_method_generic(max::t_object* o, max::t_symbol* s, long ac, max::t_atom* av) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[s->s_name];
		atoms as(ac);

		for (auto i = 0; i < ac; ++i)
			as[i] = av[i];
		meth(as);
	}

	// same as wrapper_method_generic but can return values in an atom (A_GIMMEBACK)
	template<class min_class_type>
	void wrapper_method_generic_typed(max::t_object* o, max::t_symbol* s, long ac, max::t_atom* av, max::t_atom* rv) {
		auto  self = wrapper_find_self<min_class_type>(o);
		auto& meth = *self->m_min_object.messages()[s->s_name];
		atoms as(ac);

		for (auto i = 0; i < ac; ++i)
			as[i] = av[i];
		atoms ra = meth(as);

		if (rv)
			*rv = ra[0];
	}


	// In the above wrapper methods we need access to the Max message name,
	// either to pass it on or to perform a lookup.
	// However, we can't change the method function prototype so we instead
	// pass the name as a template parameter...
	// Except that you cannot pass non-integral data as a template parameter.
	// Thus we create types for each string that we would like to pass and then
	// specialize the template calls with the type.

#ifdef C74_MIN_WITH_IMPLEMENTATION
#define MIN_WRAPPER_CREATE_TYPE_FROM_STRING(str)                                                                                           \
	struct wrapper_message_name_##str {                                                                                                    \
		static const char name[];                                                                                                          \
	};                                                                                                                                     \
	const char wrapper_message_name_##str::name[] = #str;
#else
#define MIN_WRAPPER_CREATE_TYPE_FROM_STRING(str)                                                                                           \
	struct wrapper_message_name_##str {                                                                                                    \
		static const char name[];                                                                                                          \
	};
#endif

	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(anything)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(bang)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(dblclick)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(dictionary)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(edclose)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(fileusage)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(float)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(getplaystate)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(int)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(loadbang)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(mouseenter)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(mouseleave)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(mousedown)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(mouseup)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(mousemove)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(mousedragdelta)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(mousedoubleclick)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(notify)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(okclose)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(oksize)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(paint)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(patchlineupdate)


	// Simplify the meth switches in the following code to reduce excessive and tedious code duplication

#define MIN_WRAPPER_ADDMETHOD(c, methname, wrappermethod, methtype)                                                                        \
	if (a_message.first == #methname) {                                                                                                    \
		max::class_addmethod(c,                                                                                                            \
			reinterpret_cast<max::method>(wrapper_method_##wrappermethod<min_class_type, wrapper_message_name_##methname>), #methname,     \
			max::methtype, 0);                                                                                                             \
	}


	// Shared class definition code for wrapping a Min class as a Max class

	template<class min_class_type>
	max::t_class* wrap_as_max_external_common(min_class_type& instance, const char* cppname, const char* cmaxname, void* resources) {
		using max::method;

		auto  maxname = deduce_maxclassname(cmaxname);
		auto* c       = max::class_new(maxname.c_str(), reinterpret_cast<method>(wrapper_new<min_class_type>),
            reinterpret_cast<method>(wrapper_free<min_class_type>), sizeof(minwrap<min_class_type>), nullptr, max::A_GIMME, 0);

		if (instance.is_ui_class()) {
			// wrapping as ui *must* occur immediately after class_new() or notifications will be broken
			wrap_as_max_external_ui<min_class_type>(c, instance);

			// if there is no 'notify' message then provide the default
			if (instance.messages().find("notify") == instance.messages().end())
				max::class_addmethod(c, reinterpret_cast<max::method>(wrapper_method_notify<min_class_type,wrapper_message_name_notify>), "notify", max::A_CANT, 0);
		}

		// messages

		for (auto& a_message : instance.messages()) {
			MIN_WRAPPER_ADDMETHOD(c, bang, zero, A_NOTHING)
			else MIN_WRAPPER_ADDMETHOD(c, dblclick, zero, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, okclose, zero, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, edclose, zero, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, loadbang, zero, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, anything, anything, A_GIMME)
            else MIN_WRAPPER_ADDMETHOD(c, int, int, A_LONG)
            else MIN_WRAPPER_ADDMETHOD(c, float, float, A_FLOAT)
            else MIN_WRAPPER_ADDMETHOD(c, getplaystate, getplaystate, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, dictionary, dictionary, A_SYM)
            else MIN_WRAPPER_ADDMETHOD(c, notify, notify, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, patchlineupdate, self_ptr_long_ptr_long_ptr_long, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, fileusage, ptr,
				A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, paint, paint, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, mouseenter, self_ptr_pt_long,
				A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, mouseleave, self_ptr_pt_long, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, mousedown,
				self_ptr_pt_long, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, mouseup, self_ptr, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, mousemove, self_ptr_pt_long, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c, oksize, oksize, A_CANT)
            else MIN_WRAPPER_ADDMETHOD(c,
				mousedragdelta, self_ptr_pt_long, A_CANT) else MIN_WRAPPER_ADDMETHOD(c, mousedoubleclick, self_ptr_pt_long,
				A_CANT) else if (a_message.first == "dspsetup");    // skip -- handle it in operator classes
			else if (a_message.first == "maxclass_setup");          // for min class construction only, do not add for exposure to max
			else if (a_message.first == "savestate") {
				max::class_addmethod(
					c, reinterpret_cast<max::method>(wrapper_method_savestate<min_class_type>), "appendtodictionary", max::A_CANT, 0);
			}
			else {
				max::class_addmethod(c, reinterpret_cast<method>(wrapper_method_generic<min_class_type>), a_message.first.c_str(),
					a_message.second->type(), 0);
			}

			// if there is a 'float' message but no 'int' message, generate a wrapper for it
			if (a_message.first == "float" && (instance.messages().find("int") == instance.messages().end())) {
				max::class_addmethod(
					c, reinterpret_cast<method>(wrapper_method_int<min_class_type, wrapper_message_name_float>), "int", max::A_LONG, 0);
			}
		}

		// attributes

		for (auto& an_attribute : instance.attributes()) {
			std::string     attr_name = an_attribute.first;
			attribute_base& attr      = *an_attribute.second;

			if (attr.visible() == visibility::disable)
				continue;

			attr.create(
				c, reinterpret_cast<method>(min_attr_getter<min_class_type>), reinterpret_cast<method>(min_attr_setter<min_class_type>));

			// Attribute Metadata
			CLASS_ATTR_LABEL(c, attr_name.c_str(), 0, attr.label_string());

			if (attr.editor_style() != style::none) {
				CLASS_ATTR_STYLE(c, attr_name.c_str(), 0, style_symbols[attr.editor_style()]);
			}

			if (!(attr.editor_category() == k_sym__empty)) {
				atom category_atom(attr.editor_category());
				max::class_attr_addattr_atoms(c, attr_name.c_str(), "category", k_sym_symbol, 0, 1, &category_atom);
			}

			atom order_atom{attr.editor_order()};
			max::class_attr_addattr_atoms(c, attr_name.c_str(), "order", k_sym_long, 0, 1, &order_atom);

			auto range_string = attr.range_string();
			if (!range_string.empty()) {
				if (attr.datatype() == "symbol") {
					CLASS_ATTR_ENUM(c, attr_name.c_str(), 0, range_string.c_str());
				}
				else if (attr.datatype() == "long" && attr.editor_style() == style::enum_index) {
					CLASS_ATTR_ENUMINDEX(c, attr_name.c_str(), 0, range_string.c_str());
				}
			}

			if (instance.is_ui_class()) {
				CLASS_ATTR_SAVE(c, attr_name.c_str(), 0);

				static std::unordered_map<string,symbol> attributes_associated_with_styles {
					{ "fontname", k_sym_symbol },
					{ "fontsize", symbol("double") },
					{ "fontface", symbol("char") },
					{ "textjustification", symbol("char") },
					{ "centerjust", symbol("char") },
					{ "color", symbol("rgba") },
					{ "bgcolor", symbol("rgba") },
					{ "bgfillcolor", symbol("rgba") },
					{ "accentcolor", symbol("rgba") },
					{ "gradientcolor", symbol("rgba") },
					{ "textcolor_inverse", symbol("rgba") },
					{ "textcolor", symbol("rgba") },
					{ "bordercolor", symbol("rgba") },
					{ "elementcolor", symbol("rgba") },
					{ "accentcolor", symbol("rgba") },
					{ "selectioncolor", symbol("rgba") },
					{ "stripecolor", symbol("rgba") },
					{ "patchlinecolor", symbol("rgba") },
					{ "clearcolor", symbol("rgba") }
				};
				auto found = attributes_associated_with_styles.find(attr_name);
				if (found != attributes_associated_with_styles.end())
					c74::max::class_attr_setstyle(c, attr_name.c_str());
			}
		}

		// documentation update (if neccessary)
		doc_update<min_class_type>(instance, maxname, cppname);

		return c;
	}


	template<class min_class_type>
	type_enable_if_not_audio_class<min_class_type> wrap_as_max_external_audio(max::t_class*) {}

	template<class min_class_type>
	type_enable_if_not_ui_class<min_class_type> wrap_as_max_external_ui(max::t_class*, min_class_type&) {}


	template<class min_class_type>
	void wrap_as_max_external_finish(max::t_class* c, const min_class_type& instance) {
		max::class_addmethod(c, reinterpret_cast<max::method>(wrapper_method_assist<min_class_type>), "assist", max::A_CANT, 0);

		behavior_flags flags = behavior_flags::none;
		class_get_flags<min_class_type>(instance, flags);
		if (flags == behavior_flags::nobox)
			max::class_register(max::CLASS_NOBOX, c);
		else
			max::class_register(max::CLASS_BOX, c);
	}


	template<class min_class_type, enable_if_not_jitter_class<min_class_type> = 0>
	void wrap_as_max_external(const char* cppname, const char* maxname, void* resources, min_class_type* instance = nullptr) {
		if (this_class != nullptr)
			return;

		this_class_init = true;

		std::unique_ptr<min_class_type> dummy_instance = nullptr;

		if (!instance) {
			dummy_instance = std::make_unique<min_class_type>();
			instance       = dummy_instance.get();
		}

		host_flags flags = host_flags::none;
		class_get_flags<min_class_type>(*instance, flags);
		if (flags == host_flags::no_live) {
			if (max::object_attr_getlong(k_sym_max, symbol("islib")))
				return;    // we are being loaded in Live, and a flag to the class specifically prohibits that
		}

		auto c = wrap_as_max_external_common<min_class_type>(*instance, cppname, maxname, resources);

		wrap_as_max_external_audio<min_class_type>(c);

		wrap_as_max_external_finish<min_class_type>(c, *instance);
		this_class = c;
		instance->try_call("maxclass_setup", c);
	}


	template<class min_class_type, enable_if_matrix_operator<min_class_type> = 0>
	void wrap_as_max_external(const char* cppname, const char* cmaxname, void* resources, min_class_type* instance = nullptr) {
		using c74::max::class_addmethod;
		using c74::max::method;

		if (this_class != nullptr)
			return;

		this_class_init = true;

		std::unique_ptr<min_class_type> dummy_instance = nullptr;
		auto                            maxname        = deduce_maxclassname(cmaxname);

		if (!instance) {
			dummy_instance = std::make_unique<min_class_type>();
			instance       = dummy_instance.get();
		}

		// 1. Boxless Jit Class

		this_jit_class = static_cast<max::t_class*>(max::jit_class_new(cppname, reinterpret_cast<method>(jit_new<min_class_type>),
			reinterpret_cast<method>(jit_free<min_class_type>), sizeof(minwrap<min_class_type>), 0));

		size_t inlet_count = 0;
		for (auto i : instance->inlets()) {
			if (i->type() == "matrix")
				inlet_count++;
		}

		size_t outlet_count = 0;
		for (auto i : instance->outlets()) {
			if (i->type() == "matrix")
				outlet_count++;
		}

		// If no matrix inputs are declared, the mop is a generator
		bool ownsinput = (inlet_count == 0);

		if (instance->has_call("jitclass_setup")) {
			instance->try_call("jitclass_setup", this_jit_class);
		}
		else {
			// add mop
			auto mop = max::jit_object_new(max::_jit_sym_jit_mop, inlet_count, outlet_count);
			max::jit_class_addadornment(this_jit_class, mop);

			// add methods
			max::jit_class_addmethod(
				this_jit_class, reinterpret_cast<method>(jit_matrix_calc<min_class_type>), "matrix_calc", max::A_CANT, 0);
		}

		for (auto& an_attribute : instance->attributes()) {
			std::string     attr_name = an_attribute.first;
			attribute_base& attr      = *an_attribute.second;

			attr.create(this_jit_class, reinterpret_cast<method>(min_attr_getter<min_class_type>),
				reinterpret_cast<method>(min_attr_setter<min_class_type>), true);

			// Attribute Metadata
			CLASS_ATTR_LABEL(this_jit_class, attr_name.c_str(), 0, attr.label_string());

			auto range_string = attr.range_string();
			if (!range_string.empty()) {
				if (attr.datatype() == "symbol") {
					CLASS_ATTR_ENUM(this_jit_class, attr_name.c_str(), 0, range_string.c_str());
				}
				else if (attr.datatype() == "long" && attr.editor_style() == style::enum_index) {
					CLASS_ATTR_ENUMINDEX(this_jit_class, attr_name.c_str(), 0, range_string.c_str());
				}
			}
		}

		jit_class_register(this_jit_class);


		// 2. Max Wrapper Class

		max::t_class* c = max::class_new(maxname.c_str(), reinterpret_cast<method>(max_jit_mop_new<min_class_type>),
			reinterpret_cast<method>(max_jit_mop_free<min_class_type>), sizeof(max_jit_wrapper), nullptr, max::A_GIMME, 0);

		max::max_jit_class_obex_setup(c, calcoffset(max_jit_wrapper, m_obex));

		// add special messages to max class, and object messages to jitter class
		// must happen pror to max_jit_class_wrap_standard call
		for (auto& a_message : instance->messages()) {
			MIN_WRAPPER_ADDMETHOD(c, bang, zero, A_NOTHING)
			else MIN_WRAPPER_ADDMETHOD(c, dblclick, zero, A_CANT) else MIN_WRAPPER_ADDMETHOD(
				c, okclose, zero, A_CANT) else MIN_WRAPPER_ADDMETHOD(c, edclose, zero, A_CANT) else MIN_WRAPPER_ADDMETHOD(c, loadbang, zero,
				A_CANT) else MIN_WRAPPER_ADDMETHOD(c, anything, anything, A_GIMME) else MIN_WRAPPER_ADDMETHOD(c, int, int,
				A_LONG) else MIN_WRAPPER_ADDMETHOD(c, float, float, A_FLOAT) else MIN_WRAPPER_ADDMETHOD(c, dictionary, dictionary,
				A_SYM) else MIN_WRAPPER_ADDMETHOD(c, notify, notify, A_CANT) else MIN_WRAPPER_ADDMETHOD(c,
				patchlineupdate, self_ptr_long_ptr_long_ptr_long,
				A_CANT) else MIN_WRAPPER_ADDMETHOD(c, fileusage, ptr, A_CANT) else MIN_WRAPPER_ADDMETHOD(c, paint, paint,
				A_CANT) else MIN_WRAPPER_ADDMETHOD(c, mouseenter, self_ptr_pt_long, A_CANT) else MIN_WRAPPER_ADDMETHOD(c, mouseleave,
				self_ptr_pt_long, A_CANT) else MIN_WRAPPER_ADDMETHOD(c, mousedown, self_ptr_pt_long, A_CANT)
					else MIN_WRAPPER_ADDMETHOD(c, mouseup, self_ptr, A_CANT)
						else MIN_WRAPPER_ADDMETHOD(c, mousemove, self_ptr_pt_long, A_CANT)
							else MIN_WRAPPER_ADDMETHOD(c, oksize, oksize, A_CANT)
						else MIN_WRAPPER_ADDMETHOD(c, mousedragdelta, self_ptr_pt_long,
				A_CANT) else MIN_WRAPPER_ADDMETHOD(c, mousedoubleclick, self_ptr_pt_long, A_CANT) else if (a_message.first == "savestate") {
				max::class_addmethod(
					c, reinterpret_cast<max::method>(wrapper_method_savestate<min_class_type>), "appendtodictionary", max::A_CANT, 0);
			}
			else if (a_message.first == "dspsetup");          // skip -- handle it in operator classes
			else if (a_message.first == "maxclass_setup");    // for min class construction only, do not add for exposure to max
			else if (a_message.first == "jitclass_setup");    // for min class construction only, do not add for exposure to max
			else if (a_message.first == "mop_setup");         // for min class construction only, do not add for exposure to max
			else if (a_message.first == "maxob_setup");       // for min class construction only, do not add for exposure to max
			else if (a_message.first == "setup");             // for min class construction only, do not add for exposure to max
			else {
				if (a_message.second->type() == max::A_GIMMEBACK) {
					// add handlers for gimmeback messages, allowing for return values in JS and max wrapper dumpout
					class_addmethod(this_jit_class, reinterpret_cast<method>(wrapper_method_generic<min_class_type>),
						a_message.first.c_str(), max::A_CANT, 0);
					class_addtypedwrapper(this_jit_class, reinterpret_cast<method>(wrapper_method_generic_typed<min_class_type>),
						a_message.first.c_str(), a_message.second->type(), 0);
					max_jit_class_addmethod_defer_low(
						c, reinterpret_cast<method>(max::max_jit_obex_gimmeback_dumpout), a_message.first.c_str());
				}
				else {
					// all other messages are added to the jitter class
					class_addmethod(this_jit_class, reinterpret_cast<method>(wrapper_method_generic<min_class_type>),
						a_message.first.c_str(), a_message.second->type(), 0);
				}
			}
		}

		if (instance->has_call("maxclass_setup")) {
			instance->try_call("maxclass_setup", c);
		}
		else {
			// for generator mops, override jit_matrix and outputmatrix
			long flags = (ownsinput ? max::MAX_JIT_MOP_FLAGS_OWN_OUTPUTMATRIX | max::MAX_JIT_MOP_FLAGS_OWN_JIT_MATRIX : 0);

			max::max_jit_class_mop_wrap(
				c, this_jit_class, flags);    // attrs & methods for name, type, dim, plane_count, bang, outputmatrix, etc
			max::max_jit_class_wrap_standard(
				c, this_jit_class, 0);    // attrs & methods for getattributes, dumpout, maxjitclassaddmethods, etc

			if (ownsinput)
				max::max_jit_class_addmethod_usurp_low(
					c, reinterpret_cast<method>(min_jit_mop_outputmatrix<min_class_type>), (char*)"outputmatrix");

			max::class_addmethod(c, reinterpret_cast<method>(max::max_jit_mop_assist), "assist", max::A_CANT,
				0);    // standard matrix-operator (mop) assist fn
		}

		this_class_name = symbol(cppname);

		max::class_register(max::CLASS_BOX, c);
		this_class = c;

		// documentation update (if neccessary)
		doc_update<min_class_type>(*instance, maxname, cppname);
	}

#undef MIN_WRAPPER_ADDMETHOD
#undef MIN_WRAPPER_CREATE_TYPE_FROM_STRING

}}    // namespace c74::min
