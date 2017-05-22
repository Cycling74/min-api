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
		
		

		class saturation {
		public:

			void set_drive(number drive_percentage) {
				m_drive = drive_percentage;
				auto f = MIN_CLAMP(drive_percentage / 100.0, 0.001, 0.999);

				m_z = M_PI * f;
				m_s = 1.0 / sin(m_z);
				m_b = MIN_CLAMP(1.0 / f, 0.0, 1.0);
				m_nb = m_b * -1.0;
				auto i = int(f);
				if ((f-(double)i) > 0.5)
					m_scale = sin(m_z); // sin(f * kTTPi);
				else 
					m_scale = 1.0;
			}


			sample operator()(sample x) {
				if (x > m_b)
					x = 1.0;
				else if (x < m_nb)
					x = -1.0;
#ifdef WIN_VERSION
				else {
					// http://redmine.jamoma.org/issues/show/54
					// It is insane, but on Windows sin() returns bad values
					// if the argument is negative, so we have to do this crazy workaround.
					number sign;
					if (x < 0.0) {
						x = -x;
						sign = -1.0;
					}
					else
						sign = 1.0;
					x = sign * sin(m_z * x) * m_s;
				}
#else
				else
					x = sin(m_z * x) * m_s;
#endif
				return x * m_scale;
			}


		private:
			number m_drive {};
			number m_z;
			number m_s;
			number m_b;
			number m_nb; // negative b
			number m_scale;
		};





	
	} // namespace filters

	

}} // namespace c74::min
