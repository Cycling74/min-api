/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	class sample_operator_base {};
	
	template<int input_count, int output_count>
	class sample_operator : public sample_operator_base {
		
		void foo() {
			c74::max::cpost("foo");
		}
		
	public:
		int inputcount = input_count;
		int outputcount = output_count;
	};

	
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
	
	/*
//	template<class T>
//	typename std::enable_if< has_dspsetup<T>::value && std::is_base_of<c74::min::sample_operator_base, T>::value>::type
	min_dsp64_sel(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		min_dsp64_io(self, count);
		
		atoms args;
		args.push_back(atom(samplerate));
		args.push_back(atom(maxvectorsize));
		self->obj.dspsetup(args);
		
		min_dsp64_add_perform(self, dsp64);
	}
	*/
	
//	template<class T>
//	typename std::enable_if<std::is_base_of<c74::min::sample_operator_base, T>::value>::type
//	min_dsp64(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
//		min_dsp64_sel<T>(self, dsp64, count, samplerate, maxvectorsize, flags);
//	}

}} // namespace c74::min


template<class cpp_classname>
typename std::enable_if<std::is_base_of<c74::min::sample_operator_base, cpp_classname>::value>::type
define_min_external_audio() {
	c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_dsp64<cpp_classname>, "dsp64", c74::max::A_CANT, 0);
	c74::max::class_dspinit(c74::min::this_class);
}
