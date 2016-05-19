/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	template<int input_count, int output_count>
	class sample_operator {
		
		void foo() {
			c74::max::cpost("foo");
		}
		
	public:
		int inputcount = input_count;
		int outputcount = output_count;
	};

	
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
	
	
	template <typename T>
	struct has_perform {
		template <class, class> class checker;
		
		template <typename C>
		static std::true_type test(checker<C, decltype(&C::perform)> *);
		
		template <typename C>
		static std::false_type test(...);
		
		typedef decltype(test<T>(nullptr)) type;
		static const bool value = std::is_same<std::true_type, decltype(test<T>(nullptr))>::value;
	};

	
	// the partial specialization of A is enabled via a template parameter
	template<class T, class Enable = void>
	class min_performer {
	public:
		static void perform(minwrap<T>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
			audio_bundle input = {in_chans, numins, sampleframes};
			audio_bundle output = {out_chans, numouts, sampleframes};
			self->obj.perform(input, output);
		}
	}; // primary template
	
	
	
	template<class T>
	class min_performer<T, typename std::enable_if< std::is_base_of<c74::min::sample_operator<1,1>, T >::value>::type> {
	public:
		static void perform(minwrap<T>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
			auto in_samps = in_chans[0];
			auto out_samps = out_chans[0];
			
			for (auto i=0; i<sampleframes; ++i) {
				auto in = in_samps[i];
				auto out = self->obj.calculate(in);
				out_samps[i] = out;
			}
		}
	};

	
	template<class T>
	class min_performer<T, typename std::enable_if< std::is_base_of<c74::min::sample_operator<3,1>, T >::value>::type> {
	public:
		static void perform(minwrap<T>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
			auto in_samps1 = in_chans[0];
			auto in_samps2 = in_chans[1];
			auto in_samps3 = in_chans[2];
			auto out_samps = out_chans[0];
			
			for (auto i=0; i<sampleframes; ++i) {
				auto in1 = in_samps1[i];
				auto in2 = in_samps2[i];
				auto in3 = in_samps3[i];
				auto out = self->obj.calculate(in1, in2, in3);
				out_samps[i] = out;
			}
		}
	}; // specialization for floating point types
	


	
	
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
	void min_dsp64_io(minwrap<T>* self, short* count) {
		int i = 0;
		
		while (i < self->obj.inlets.size()) {
			self->obj.inlets[i]->signal_connection = count[i];
			++i;
		}
		while (i < self->obj.outlets.size()) {
			self->obj.outlets[i - self->obj.inlets.size()]->signal_connection = count[i];
			++i;
		}
	}
	
	
	// TODO: enable a different add_perform if no perform with the correct sig is available?
	// And/or overload the min_perform for different input and output counts?
	
	template<class T>
	void min_dsp64_add_perform(minwrap<T>* self, max::t_object* dsp64) {
		// find the perform method and add it
		object_method_direct(void, (max::t_object*, max::t_object*, max::t_perfroutine64, long, void*),
							 dsp64, max::gensym("dsp_add64"), (max::t_object*)self, (max::t_perfroutine64)min_performer<T>::perform, 0, NULL);
	}
	
	template<class T>
	typename std::enable_if< has_dspsetup<T>::value && std::is_base_of<c74::min::audio_object, T>::value>::type
	min_dsp64_sel(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		min_dsp64_io(self, count);
		
		atoms args;
		args.push_back(atom(samplerate));
		args.push_back(atom(maxvectorsize));
		self->obj.dspsetup(args);
		
		min_dsp64_add_perform(self, dsp64);
	}
	
	template<class T>
	typename std::enable_if< !has_dspsetup<T>::value>::type
	min_dsp64_sel(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		min_dsp64_io(self, count);
		min_dsp64_add_perform(self, dsp64);
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
	std::string		smaxname;
	
	// maxname may come in as an entire path because of use of the __FILE__ macro
	{
		const char* start = strrchr(maxname, '/');
		if (start)
			start += 1;
		else {
			start = strrchr(maxname, '\\');
			if (start)
				start += 1;
			else
				start = maxname;
		}

		const char* end = strstr(start, "_tilde.cpp");
		if (end) {
			smaxname.assign(start, end-start);
			smaxname += '~';
		}
		else {
			const char* end = strrchr(start, '.');
			if (!end)
				end = start + strlen(start);
			smaxname.assign(start, end-start);
		}
	}
	
	c74::min::this_class = c74::max::class_new( smaxname.c_str() ,(c74::max::method)c74::min::min_new<cpp_classname>, (c74::max::method)c74::min::min_free<cpp_classname>, sizeof( c74::min::minwrap<cpp_classname> ), nullptr, c74::max::A_GIMME, 0);
	
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
