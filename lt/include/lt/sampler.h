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
		Float next_float() {
			return _urd(_rng);
		}

	private:
		std::random_device _dev;
		std::mt19937 _rng;
		std::uniform_real_distribution<Float> _urd;
	};



}