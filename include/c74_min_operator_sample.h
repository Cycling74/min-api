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
	public:
		static constexpr size_t inputcount() {
			return m_inputcount;
		}
		
		static constexpr size_t outputcount() {
			return m_outputcount;
		}
				
	private:
		static constexpr size_t m_inputcount = input_count;
		static constexpr size_t m_outputcount = output_count;
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

	
//	template<class T>
//	class min_performer<T, typename std::enable_if< std::is_base_of<c74::min::sample_operator<3,1>, T >::value>::type> {
//	public:
//		static void perform(minwrap<T>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
//			auto in_samps1 = in_chans[0];
//			auto in_samps2 = in_chans[1];
//			auto in_samps3 = in_chans[2];
//			auto out_samps = out_chans[0];
//
//			for (auto i=0; i<sampleframes; ++i) {
//				auto in1 = in_samps1[i];
//				auto in2 = in_samps2[i];
//				auto in3 = in_samps3[i];
//				auto out = self->obj.calculate(in1, in2, in3);
//				out_samps[i] = out;
//			}
//		}
//	};
	
	
	// for more information, see:
	// http://stackoverflow.com/questions/16834851/passing-stdarray-as-arguments-of-template-variadic-function
	namespace detail {
		template<int... Is>
		struct seq { };
		
		template<int N, int... Is>
		struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };
		
		template<int... Is>
		struct gen_seq<0, Is...> : seq<Is...> { };
	}

	
	template<class T, int count>
	struct callable_samples {
		
		callable_samples(minwrap<T>* a_self)
		: self (a_self)
		{}
		
		void set(size_t index, sample& value) {
			data[index] = value;
		}

		auto call() {
			return call(detail::gen_seq<count>());
		}
		
		template<int... Is>
		auto call(detail::seq<Is...>) {
			return self->obj.calculate(data[Is]...);
		}
		
		samples<count>	data;
		minwrap<T>*		self;
	};
	
	

	
	template<class T>
	class min_performer<T, typename std::enable_if<
		std::is_base_of<c74::min::sample_operator_base, T >::value
		&& !std::is_base_of<c74::min::sample_operator<1,1>, T >::value
	>::type> {
	public:
		static void perform(minwrap<T>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
			for (auto i=0; i<sampleframes; ++i) {
				callable_samples<T, T::inputcount()> ins(self);

				for (auto chan=0; chan < self->obj.inputcount(); ++chan)
					ins.set(chan, in_chans[chan][i]);

				auto out = ins.call(); // (self, &T::calculate);

				for (auto chan=0; chan < self->obj.outputcount(); ++chan)
					out_chans[chan][i] = out[chan];
			}
		}
	};

}} // namespace c74::min


template<class cpp_classname>
typename std::enable_if<std::is_base_of<c74::min::sample_operator_base, cpp_classname>::value>::type
define_min_external_audio(c74::max::t_class* c) {
	c74::max::class_addmethod(c, (c74::max::method)c74::min::min_dsp64<cpp_classname>, "dsp64", c74::max::A_CANT, 0);
	c74::max::class_dspinit(c);
}
