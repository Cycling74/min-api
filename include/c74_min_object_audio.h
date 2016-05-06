/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	
	template<class T>
	struct minwrap < T, typename std::enable_if< std::is_base_of< min::audio_object, T>::value >::type > {
		max::t_pxobject 		header;
		T						obj;
		
		void setup() {
			max::dsp_setup(&header, obj.inlets.size());
		}
		
		void cleanup() {
			dsp_free(&header);
		}
	};
	

	
	
	
	
	
	struct audio_bundle {
		double **samples;
		long channel_count;
		long frame_count;
	};
	
	template<class T>
	typename std::enable_if<std::is_base_of<c74::min::audio_object, T>::value>::type
	min_perform(minwrap<T>* self, max::t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
		audio_bundle input = {ins, numins, sampleframes};
		audio_bundle output = {outs, numouts, sampleframes};
		self->obj.perform(input, output);
	}
	
	
	
	
	template <typename T>
	struct has_dspsetup {
		template <class, class> class checker;
		
		template <typename C>
		static std::true_type test(checker<C, decltype(&C::dspsetup)> *);
		
		template <typename C>
		static std::false_type test(...);
		
		typedef decltype(test<T>(nullptr)) type;
		static const bool value = std::is_same<std::true_type, decltype(test<T>(nullptr))>::value;
	};
	
	//	static_assert(has_dspsetup<slide>::value, "error");
	
	
	template<class T>
	typename std::enable_if< has_dspsetup<T>::value && std::is_base_of<c74::min::audio_object, T>::value>::type
	min_dsp64_sel(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		int i = 0;
		
		while (i < self->obj.inlets.size()) {
			self->obj.inlets[i]->signal_connection = count[i];
			++i;
		}
		while (i < self->obj.outlets.size()) {
			self->obj.outlets[i - self->obj.inlets.size()]->signal_connection = count[i];
			++i;
		}
		
		atoms args;
		args.push_back(atom(samplerate));
		args.push_back(atom(maxvectorsize));
		self->obj.dspsetup(args);
		
		// find the perform method and add it
		object_method_direct(void, (max::t_object*, max::t_object*, max::t_perfroutine64, long, void*),
							 dsp64, max::gensym("dsp_add64"), (max::t_object*)self, (max::t_perfroutine64)min_perform<T>, 0, NULL);
	}
	
	template<class T>
	typename std::enable_if< !has_dspsetup<T>::value>::type
	min_dsp64_sel(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		int i = 0;
		
		while (i < self->obj.inlets.size()) {
			self->obj.inlets[i]->signal_connection = count[i];
			++i;
		}
		while (i < self->obj.outlets.size()) {
			self->obj.outlets[i - self->obj.inlets.size()]->signal_connection = count[i];
			++i;
		}
		
		// find the perform method and add it
		object_method_direct(void, (max::t_object*, max::t_object*, max::t_perfroutine64, long, void*),
							 dsp64, max::gensym("dsp_add64"), (max::t_object*)self, (max::t_perfroutine64)min_perform<T>, 0, NULL);
	}
	
	template<class T>
	typename std::enable_if<std::is_base_of<c74::min::audio_object, T>::value>::type
	min_dsp64(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		min_dsp64_sel<T>(self, dsp64, count, samplerate, maxvectorsize, flags);
	}

}} // namespace c74::min




template<class cpp_classname>
typename std::enable_if<std::is_base_of<c74::min::audio_object, cpp_classname>::value>::type
define_min_external(const char* cppname, const char* maxname, void *resources)
{
	c74::min::atoms	a;
	cpp_classname	dummy(a);
	
	c74::min::this_class = c74::max::class_new( maxname ,(c74::max::method)c74::min::min_new<cpp_classname>, (c74::max::method)c74::min::min_free<cpp_classname>, sizeof( c74::min::minwrap<cpp_classname> ), nullptr, c74::max::A_GIMME, 0);
	
	for (auto& a_method : dummy.methods) {
		if (a_method.first == "dspsetup")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_dsp64<cpp_classname>, "dsp64", c74::max::A_CANT, 0);
		else if (a_method.first == "dblclick")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method_dblclick<cpp_classname>, "dblclick", c74::max::A_CANT, 0);
		else if (a_method.first == "notify")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method_notify<cpp_classname>, "notify", c74::max::A_CANT, 0);
		else if (a_method.first == "okclose")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method<cpp_classname>, "okclose", c74::max::A_CANT, 0);
		else if (a_method.first == "edclose")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method<cpp_classname>, "edclose", c74::max::A_CANT, 0);
		else if (a_method.first == "anything")
			class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_anything<cpp_classname>, "anything", c74::max::A_NOTHING, 0);
		else if (a_method.first == "bang")
			class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_bang<cpp_classname>, "bang", c74::max::A_NOTHING, 0);
		else if (a_method.first == "int")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_int<cpp_classname>, "int", c74::max::A_LONG, 0);
		else if (a_method.first == "float") {
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_float<cpp_classname>, "float", c74::max::A_FLOAT, 0);
			
			// if there is a 'float' method but no 'int' method, generate a wrapper for it
			auto got = dummy.methods.find("int");
			if ( got == dummy.methods.end() )
				c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_int_converted_to_float<cpp_classname>, "int", c74::max::A_LONG, 0);
		}
		else
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method<cpp_classname>, a_method.first.c_str(), a_method.second->type, 0);
	}
	
	// if there is no custom dspsetup, and this is an audio_object, add the default
	if (std::is_base_of<c74::min::audio_object, cpp_classname>()) {
		auto got = dummy.methods.find("dspsetup");
		if ( got == dummy.methods.end() )
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_dsp64<cpp_classname>, "dsp64", c74::max::A_CANT, 0);
	}
	
	for (auto& an_attribute : dummy.attributes) {
		std::string				attr_name = an_attribute.first;
		//min::attribute_base*	attr = an_attribute.second;
		
		c74::max::class_addattr(c74::min::this_class,
								c74::max::attr_offset_new(attr_name.c_str(),
														  c74::max::gensym("float64"),
														  0,
														  (c74::max::method)c74::min::min_attr_getter<cpp_classname>,
														  (c74::max::method)c74::min::min_attr_setter<cpp_classname>,
														  0
														  )
								);
	}
	
	c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_assist<cpp_classname>, "assist", c74::max::A_CANT, 0);
	
	c74::max::class_dspinit(c74::min::this_class);
	c74::max::class_register(c74::max::CLASS_BOX, c74::min::this_class);
}
