#pragma once
#include <lt/lt_common.h>

namespace LT_NAMESPACE {
	

	class Sensor
	{
	public:
		Sensor() : w(0), h(0) {};
		Sensor(int w, int h) : w(w), h(h) {
			data.resize(w * h);
		}
		~Sensor() {}

		int w;
		int h;
		std::vector<Spectrum> data;
	
	};

	
}