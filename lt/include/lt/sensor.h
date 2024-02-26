#pragma once
#include <lt/lt_common.h>

namespace LT_NAMESPACE {
	

	class Sensor
	{
	public:
		Sensor() : w(0), h(0) {};
		Sensor(uint32_t w, uint32_t h) : w(w), h(h) {
			value.resize(w * h);
			acculumator.resize(w * h);
			count.resize(w * h);
			u = linspace<Float>(-1, 1, w);
			v = linspace<Float>(1, -1, h);
		}

		void reset() {
			memset(acculumator.data(), 0, sizeof(Spectrum) * acculumator.size());
			memset(value.data(), 0, sizeof(Spectrum) * value.size());
			memset(count.data(), 0, sizeof(uint16_t) * count.size());
		}
		
		void add(const uint32_t& x, const uint32_t& y, Spectrum s) {
			uint32_t idx = y * w + x;
			acculumator[idx] += s;
			count[idx]++;
			value[idx] = acculumator[idx] / Spectrum(count[idx]);
		}

		void set(const uint32_t& x, const uint32_t& y, Spectrum s) {
			uint32_t idx = y * w + x;
			value[idx] = s;
			acculumator[idx] = s;
			count[idx] = 1;
		}

		uint16_t n_sample(const uint32_t& x, const uint32_t& y) {
			return count[y * w + x];
		}
		
		uint32_t w;
		uint32_t h;
		std::vector<Spectrum> acculumator;
		std::vector<Spectrum> value;
		std::vector<uint16_t> count;
		std::vector<Float> u;
		std::vector<Float> v;
	
	};

	
}