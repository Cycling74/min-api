/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {


	/// The base class for all template specializations of sample_operator.

	class sample_operator_base {};


	/// Inheriting from sample_operator extends your class functionality to processing audio
	/// by calculating samples one at a time using the call operator member of your class.
	///
	/// Your call operator must take the same number of parameters as the input_count template arg.
	/// Additionally, your call operator must return an array of samples of the same size as the output_count template arg.
	/// For example, if your object inherits from sample_operator<3,2> then your call operator will be prototyped as:
	/// @code
	/// samples<2> operator() (sample input1, sample input2, sample input3);
	/// @endcode
	///
	/// @tparam input_count		The number of audio inputs for your object.
	/// @tparam output_count	The number of audio outputs for your object.

	template<size_t input_count, size_t output_count>
	class sample_operator : public sample_operator_base {
	public:
		static constexpr size_t inputcount() {
			return m_inputcount;
		}

		static constexpr size_t outputcount() {
			return m_outputcount;
		}

	private:
		static constexpr size_t m_inputcount	{ input_count };
		static constexpr size_t m_outputcount	{ output_count };
	};


	// The min_performer class is used by the wrapper code to adapt the calls coming from the Max application
	// to the call operator implemented in the Min class.
	//
	// There are two versions of this.
	// This one is optimized for the most common case: a single input and a single output.
	// The other version is generic for N inputs and N outputs.

	template<class min_class_type>
	class min_performer<min_class_type, typename enable_if< is_base_of<sample_operator<1,1>, min_class_type >::value>::type> {
	public:
		static void perform(minwrap<min_class_type>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
			auto in_samps = in_chans[0];
			auto out_samps = out_chans[0];
			
			for (auto i=0; i<sampleframes; ++i) {
				auto in = in_samps[i];
				auto out = self->min_object(in);
				out_samps[i] = out;
			}
		}
	};


	// To implement the min_performer class generically we use std::array<sample> for both input and output.
	// However, we wish to define the call operator in the Min class with each sample as a
	// separate argument.
	// To make this translation efficiently and without out lots of duplicated code we use a pattern whereby
	// the sequence of indices for std::array are generated at compile time and then used to make the call
	// as a variadic template function.
	//
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


	template<class min_class_type, int count>
	struct callable_samples {

		explicit callable_samples(minwrap<min_class_type>* a_self)
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
			return self->min_object(data[Is]...);
		}

		samples<count>				data;
		minwrap<min_class_type>*	self;
	};


	// perform_copy_output() copies the output sample(s) from a sample_operator's call operator.
	// it does so in a way that the returned type can either be a single sample or an array of samples<N>

	template<class min_class_type, typename type_returned_from_call_operator>
	void perform_copy_output(minwrap<min_class_type>* self, size_t index, double** out_chans, type_returned_from_call_operator vals) {
		for (auto chan=0; chan < self->min_object.outputcount(); ++chan)
			out_chans[chan][index] = vals[chan];
	}

	template<class min_class_type>
	void perform_copy_output(minwrap<min_class_type>* self, size_t index, double** out_chans, sample val) {
		out_chans[0][index] = val;
	}
	
	
	// The generic version of the min_performer class, for N inputs and N outputs.
	// See above for the 1x1 optimized version.

	template<class min_class_type>
	class min_performer<min_class_type, typename enable_if< is_base_of<sample_operator_base, min_class_type >::value && !is_base_of<sample_operator<1,1>, min_class_type >::value>::type> {
	public:
		static void perform(minwrap<min_class_type>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
			for (auto i=0; i<sampleframes; ++i) {
				callable_samples<min_class_type, min_class_type::inputcount()> ins(self);

				for (auto chan=0; chan < self->min_object.inputcount(); ++chan)
					ins.set(chan, in_chans[chan][i]);

				auto out = ins.call();

				if (numouts > 0)
					perform_copy_output(self, i, out_chans, out);
			}
		}
	};

	// If you inherit from sample_operator then define this function to
	// add audio support to the Max class.
	// Will be called from define_min_external()

	template<class min_class_type, enable_if_sample_operator<min_class_type> = 0>
	void wrap_as_max_external_audio(max::t_class* c) {
		max::class_addmethod(c, (max::method)min_dsp64<min_class_type>, "dsp64", max::A_CANT, 0);
		max::class_dspinit(c);
	}
}} // namespace c74::min


