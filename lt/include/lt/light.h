#pragma once 
#include <lt/lt_common.h>

namespace LT_NAMESPACE {

	class Light {
	public:
		virtual vec3 sample_light_direction() = 0;
	};

	class DirectionnalLight : public Light {
	public:
		DirectionnalLight(vec3 dir) : dir(dir) {

		}

		vec3 sample_light_direction() {
			return dir;
		}

		vec3 dir;
	};

}