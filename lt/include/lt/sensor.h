#pragma once
#include <lt/lt_common.h>

namespace LT_NAMESPACE {
	

	class Sensor
	{
	public:
		Sensor() : w(0), h(0) {};
		Sensor(uint32_t w, uint32_t h) : w(w), h(h) {
			data.resize(w * h);
			count.resize(w * h);
			u = linspace<Float>(-1, 1, w);
			v = linspace<Float>(1, -1, h);
		}
		
		void add(uint32_t x, uint32_t y, Spectrum s) {
			uint32_t idx = y * w + x;
			//data[idx] += s;
			data[idx] = (data[idx] * Spectrum(count[idx]) + s) / Spectrum(count[idx] + 1);
			count[idx]++;
		}

		void set(uint32_t x, uint32_t y, Spectrum s) {
			uint32_t idx = y * w + x;
			data[idx] = s;
			count[idx] = 1;
		}
		
		uint32_t w;
		uint32_t h;
		std::vector<Spectrum> data;
		std::vector<uint16_t> count;
		std::vector<Float> u;
		std::vector<Float> v;
	
	};

	
}