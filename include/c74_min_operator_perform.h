/// @file
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
		
	class perform_operator_base {};
	
	//template<int input_count, int output_count>
	class perform_operator : public perform_operator_base {};
	
	
	template<class T>
	struct minwrap < T, typename std::enable_if<
		std::is_base_of< min::perform_operator_base, T>::value
		|| std::is_base_of< min::sample_operator_base, T>::value
	>::type > {
		maxobject_base	base;
		T				obj;
		
		void setup() {
			max::dsp_setup(base, obj.inlets.size());
		}
		
		void cleanup() {
			max::dsp_free(base);
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
	typename std::enable_if< has_dspsetup<T>::value && std::is_base_of<c74::min::perform_operator_base, T>::value>::type
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
	typename std::enable_if<std::is_base_of<c74::min::perform_operator_base, T>::value>::type
	min_dsp64(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		min_dsp64_sel<T>(self, dsp64, count, samplerate, maxvectorsize, flags);
	}
		
}} // namespace c74::min


template<class cpp_classname>
typename std::enable_if<std::is_base_of<c74::min::perform_operator_base, cpp_classname>::value>::type
define_min_external_audio() {
	c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_dsp64<cpp_classname>, "dsp64", c74::max::A_CANT, 0);
	c74::max::class_dspinit(c74::min::this_class);
}
