/// @file
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
		
	class perform_operator_base {};
	

	class perform_operator : public perform_operator_base {};
	
	
	template<class min_class_type>
	struct minwrap <min_class_type, type_enable_if_audio_class<min_class_type> > {
		maxobject_base	max_base;
		min_class_type	min_object;
		
		void setup() {
			max::dsp_setup(max_base, min_object.inlets().size());
			min_object.create_outlets();
		}
		
		void cleanup() {
			max::dsp_free(max_base);
		}
	};
	
	
	
	struct audio_bundle {
		
		audio_bundle(double** samples, long channelcount, long framecount)
		: m_samples			{ samples }
		, m_channelcount	{ channelcount }
		, m_framecount		{ framecount }
		{}

		double** samples() {
			return m_samples;
		}
		
		double* samples(size_t channel) {
			return m_samples[channel];
		}
		
		long channelcount() {
			return m_channelcount;
		}
		
		long framecount() {
			return m_framecount;
		}
		
		double**	m_samples = nullptr;
		long		m_channelcount = 0;
		long		m_framecount = 0;
	};
	
	
	template<typename min_class_type>
	struct has_perform {
		template<class,class> class checker;
		
		template<typename C>
		static std::true_type test(checker<C, decltype(&C::perform)> *);
		
		template<typename C>
		static std::false_type test(...);
		
		typedef decltype(test<min_class_type>(nullptr)) type;
		static const bool value = is_same<std::true_type, decltype(test<min_class_type>(nullptr))>::value;
	};
	
	
	// the partial specialization of A is enabled via a template parameter
	template<class min_class_type, class Enable = void>
	class min_performer {
	public:
		static void perform(minwrap<min_class_type>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
			audio_bundle input = {in_chans, numins, sampleframes};
			audio_bundle output = {out_chans, numouts, sampleframes};
			self->min_object.perform(input, output);
		}
	}; // primary template
	
	
	template<typename min_class_type>
	struct has_dspsetup {
		template<class,class> class checker;
		
		template<typename C>
		static std::true_type test(checker<C, decltype(&C::dspsetup)> *);
		
		template<typename C>
		static std::false_type test(...);
		
		typedef decltype(test<min_class_type>(nullptr)) type;
		static const bool value = is_same<std::true_type, decltype(test<min_class_type>(nullptr))>::value;
	};
	
	//	static_assert(has_dspsetup<slide>::value, "error");
	
	
	template<class min_class_type>
	void min_dsp64_io(minwrap<min_class_type>* self, short* count) {
		int i = 0;
		
		while (i < self->min_object.inlets().size()) {
			self->min_object.inlets()[i]->update_signal_connection(count[i]);
			++i;
		}
		while (i < self->min_object.outlets().size()) {
			self->min_object.outlets()[i - self->min_object.inlets().size()]->update_signal_connection(count[i]);
			++i;
		}
	}
	
	
	// TODO: enable a different add_perform if no perform with the correct sig is available?
	// And/or overload the min_perform for different input and output counts?
	
	template<class min_class_type>
	void min_dsp64_add_perform(minwrap<min_class_type>* self, max::t_object* dsp64) {
		// find the perform method and add it
		object_method_direct(void, (max::t_object*, max::t_object*, max::t_perfroutine64, long, void*),
							 dsp64, max::gensym("dsp_add64"), (max::t_object*)self, (max::t_perfroutine64)min_performer<min_class_type>::perform, 0, NULL);
	}
	
	template<class min_class_type>
	typename enable_if< has_dspsetup<min_class_type>::value && is_base_of<perform_operator_base, min_class_type>::value>::type
	min_dsp64_sel(minwrap<min_class_type>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		min_dsp64_io(self, count);
		
		atoms args;
		args.push_back(atom(samplerate));
		args.push_back(atom(maxvectorsize));
		self->min_object.dspsetup(args);
		
		min_dsp64_add_perform(self, dsp64);
	}
	
	template<class min_class_type>
	typename enable_if< !has_dspsetup<min_class_type>::value>::type
	min_dsp64_sel(minwrap<min_class_type>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		min_dsp64_io(self, count);
		min_dsp64_add_perform(self, dsp64);
	}
	
	template<class min_class_type>
	type_enable_if_audio_class<min_class_type> min_dsp64(minwrap<min_class_type>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		min_dsp64_sel<min_class_type>(self, dsp64, count, samplerate, maxvectorsize, flags);
	}


	template<class min_class_type, enable_if_perform_operator<min_class_type> = 0>
	void wrap_as_max_external_audio(max::t_class* c) {
		max::class_addmethod(c, (max::method)min_dsp64<min_class_type>, "dsp64", max::A_CANT, 0);
		max::class_dspinit(c);
	}
}} // namespace c74::min

