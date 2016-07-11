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
	};
	
	
	
	
	// for more information, see:
	// http://stackoverflow.com/questions/16834851/passing-stdarray-as-arguments-of-template-variadic-function
	namespace detail
	{
		template<int... Is>
		struct seq { };
		
		template<int N, int... Is>
		struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };
		
		template<int... Is>
		struct gen_seq<0, Is...> : seq<Is...> { };
	}

	
	template<class T>
	class min_performer<T, typename std::enable_if< std::is_base_of<c74::min::sample_operator<2,2>, T >::value>::type> {
		
		template<int N, typename U>
		struct Container {
			template<typename... Us>
			Container(Us&&... vs) : data{{std::forward<Us>(vs)...}} {
				static_assert(sizeof...(Us)==N,"Not enough args supplied!");
			}
			
			template<typename F>
			auto doOperation(minwrap<T>* self, F&& func) {
				return doOperation(self, std::forward<F>(func), detail::gen_seq<N>());
			}
			
			template<typename F, int... Is>
			auto doOperation(minwrap<T>* self, F&& func, detail::seq<Is...>) {
				return self->obj.calculate(data[Is]...);
			}
			
			std::array<U,N> data;
		};


	public:
		static void perform(minwrap<T>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
			auto in_samps1 = in_chans[0];
			auto in_samps2 = in_chans[1];
			auto out_samps1 = out_chans[0];
			auto out_samps2 = out_chans[1];
			
			for (auto i=0; i<sampleframes; ++i) {
				Container<2, sample> ins( in_samps1[i], in_samps2[i] );
				
				//auto out = self->obj.calculate(in1, in2);
//				auto out = ins.doOperation(std::bind(&T::calculate, &self->obj, std::placeholders::_1, std::placeholders::_2));
				auto out = ins.doOperation(self, &T::calculate);

				out_samps1[i] = out[0];
				out_samps2[i] = out[1];
			}
		}
	}; // specialization for floating point types

}} // namespace c74::min


template<class cpp_classname>
typename std::enable_if<std::is_base_of<c74::min::sample_operator_base, cpp_classname>::value>::type
define_min_external_audio(c74::max::t_class* c) {
	c74::max::class_addmethod(c, (c74::max::method)c74::min::min_dsp64<cpp_classname>, "dsp64", c74::max::A_CANT, 0);
	c74::max::class_dspinit(c);
}
