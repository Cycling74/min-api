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
		
		inline double random(double min, double max) {
			std::random_device					rd;
			std::mt19937						gen { rd() };
			std::uniform_real_distribution<>	dis { min, max };

			return dis(gen);
		}
        
        
        // calculates the fold of x between lo and hi.
        
        inline double fold(double x, double lo, double hi) {
            long	di;
			double	m;
			double	d;

            if (lo > hi) {
				auto tmp { lo };
                lo = hi;
                hi = tmp;
            }
            if (lo)
                x -= lo;

			m = hi - lo;
            if (m) {
                if (x < 0.0)
                    x = -x;
                if (x > m) {
                    if (x > (m*2.0)) {
                        d = x / m;
                        di = static_cast<long>(d);	// cast to long
                        d = d - di;					// cast to double
                        if (di % 2) {
                            if (d < 0)
                                d = -1.0 - d;
							else
                                d = 1.0 - d;
                        }
                        x = d * m;
                        if (x < 0.0)
                            x = m + x;
                    }
					else {
                        x = m - (x-m);
                    }
                }
            }
			else
				x = 0.0; //don't divide by zero
            
            return x + lo;
        }
        
        
        //Calculates the wrap of x between lo and hi.
        
        inline double wrap(double x, double lo, double hi) {
			double m;
			double d;
            long di;
            
            if (lo > hi) {
				auto tmp { lo };
                lo = hi;
                hi = tmp;
            }
            if (lo)
                x -= lo;
            m = hi-lo;
            if (m) {		
                if (x > m) {
                    if (x>(m*2.0)) {
                        d = x / m;
                        di = (long) d;
                        d = d - (double) di;
                        x = d * m;
                    }
					else {
                        x -= m;
                    }
                }
				else if (x < 0.0) {
                    if (x < (-m)) {
                        d = x / m;
                        di = static_cast<long>(d);		// cast to long
                        d = d - di;						// cast to double
                        x = d * m;
                        if (x < 0.0)
                            x += m;
                    }
					else {
                        x += m;
                    }
                }
            }
			else
				x = 0.0; //don't divide by zero
            
            return x + lo; 
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
						   }
			);
			
			double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
			double stdev = std::sqrt(sq_sum / v.size());
			
			return std::make_pair(mean, stdev);
		}

	
	}

	
	

	

}} // namespace c74::min
