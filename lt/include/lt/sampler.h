#pragma once 
#include <lt/lt_common.h>
#include <random>

namespace LT_NAMESPACE {

	class Sampler
	{
	public:

		Sampler() {
			_rng = std::mt19937(_dev());
			_urd = std::uniform_real_distribution<Float>(0, 1);
		};

		Sampler(Sampler& s) {
			_rng = std::mt19937(_dev());
			_urd = std::uniform_real_distribution<Float>(0, 1);
			/*_rng = s._rng;
			_urd = s._urd;*/
		};

		Float next_float() {
			return _urd(_rng);
		}

		void seed(uint32_t s) {
			_rng.seed(s);
		}
	private:
		std::random_device _dev;
		std::mt19937 _rng;
		std::uniform_real_distribution<Float> _urd;
	};



}