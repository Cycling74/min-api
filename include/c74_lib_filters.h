/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once


namespace c74 {
namespace min {



	namespace filters {
	
		/// Utility: generate an impulse response from a set of coefficients
		/// param	a	feedforward coefficients (numerator)
		/// param	b	feedback coefficients (denominator)
		/// param	N	optional size of the generated response, default 64
		inline auto generate_impulse_response(const sample_vector& a, const sample_vector& b, int N = 64) {
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
		











		class dcblocker {
		public:

			void clear() {
				x_1 = y_1 = 0.0;
			}

			sample operator()(sample x) {
				auto y = x - x_1 + y_1 * 0.9997;
				y_1 = y;
				x_1 = x;
				return y;
			}

		private:
			sample	x_1 { 0.0 };	///< Input history
			sample	y_1 { 0.0 };	///< Output history
		};








	
	} // namespace filters

	

}} // namespace c74::min
