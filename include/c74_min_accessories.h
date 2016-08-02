/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include <random>
#include <numeric>

namespace c74 {
namespace min {


	
	#ifdef WIN_VERSION
		#define MIN_CLAMP( in, lo, hi )		c74::max::clamp<std::remove_reference<decltype(in)>::type>(in, (decltype(in))lo, (decltype(in))hi)
	#else
		#define MIN_CLAMP( in, lo, hi )		c74::max::clamp<typeof(in)>(in, lo, hi)
	#endif

	
	
	namespace math {
	
		/// code from http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
		/// see also http://en.cppreference.com/w/cpp/numeric/random
		
		double random(double min, double max) {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dis(min, max);
			return dis(gen);
		}
		
		/// Generates a cosine wave constrained between -1 to 1
		///	@param T       render output as this datatype. algorithm was designed to assume the use of floating point.
		///	@param size		size of the target vector
		///	@param count	number of cycles of the wave to generate across the vector
		
		template<typename T>
		class cosine {
		public:
			explicit cosine (size_t size, double count = 1.0)
			: mCycleSize	{ static_cast<double>(size) }
			, mCycleCount	{ count }
			{
				//TODO: we need way to protect against zero. static_assert did not work.
			}
			
			T operator()() {
				++mCurrent;
				T output = std::cos((mCurrent*mCycleCount / mCycleSize) * M_PI * 2.0);
				return output;
			}
			
		private:
			int		mCurrent { -1 };
			double	mCycleSize;
			double	mCycleCount;
		};
		
		
		template<class T>
		auto mean(const std::vector<T>& v) {
			double sum = std::accumulate(v.begin(), v.end(), 0.0);
			double mean = sum / v.size();
			
			std::vector<double> diff(v.size());
			std::transform(v.begin(), v.end(), diff.begin(),
						   [mean](double x) {
							   return x - mean;
						   });
			
			double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
			double stdev = std::sqrt(sq_sum / v.size());
			
			return std::make_pair(mean, stdev);
		}

	
	}

	
	
	namespace filters {
	
		/// Utility: generate an impulse response from a set of coefficients
		/// param	a	feedforward coefficients (numerator)
		/// param	b	feedback coefficients (denominator)
		/// param	N	optional size of the generated response, default 64
		auto generate_impulse_response(const sample_vector& a, const sample_vector& b, int N = 64) {
			sample_vector	x(N);				// input -- feedforward history
			sample_vector	y(N);				// output -- feedback history
			
			std::fill_n(x.begin(), N, 0.0);
			std::fill_n(y.begin(), N, 0.0);
			x[0] = 1.0;
			
			for (int n=0; n<N; n++) {
				for (auto i=0; i<a.size(); i++) {
					if (n-i < 0 )
						y[n] += 0.0;
					else
						y[n] += ( a[i] * x[n-i]);
				}
				
				// ignore b[0] and assume it is normalized to 1.0
				for (auto i=1; i<b.size(); i++) {
					if (n-i < 0)
						y[n] -= 0.0;
					else
						y[n] -= ( b[i] * y[n - i] );
				}
			}
			return y;
		}
		
	
	
	}


	namespace strings {

		/// Split a string into a vector of substrings on a specified delimiter
		/// @param	s		The string to split
		/// @param	delim	The delimiter on which to split the string
		/// @return			A vector of substrings

		std::vector<std::string> split(const std::string &s, char delim) {
			std::vector<std::string>	substrings;
			std::string					substring;
			std::stringstream			ss(s);

			while (getline(ss, substring, delim))
				substrings.push_back(substring);
			return substrings;
		}

	}
	
	

}} // namespace c74::min
