#pragma once
#include <lt/lt_common.h>

namespace LT_NAMESPACE {
	

	class Sensor
	{
	public:
		Sensor() : w(0), h(0) {};
		Sensor(uint16_t w, uint16_t h) : w(w), h(h) {
			data.resize(w * h);
			count.resize(w * h);
			u = linspace<Float>(-1, 1, w);
			v = linspace<Float>(1, -1, h);
		}
		
		void add(uint16_t x, uint16_t y, Spectrum s) {
			uint32_t idx = y * w + x;
			data[idx] += s;
			count[idx]++;
		}

		void set(uint16_t x, uint16_t y, Spectrum s) {
			uint32_t idx = y * w + x;
			data[idx] = s;
			count[idx] = 1;
		}
		
		uint16_t w;
		uint16_t h;
		std::vector<Spectrum> data;
		std::vector<uint16_t> count;
		std::vector<Float> u;
		std::vector<Float> v;
	
	};

	
}