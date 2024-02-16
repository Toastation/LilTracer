#pragma once 

#include <lt/lt_common.h>
#include <lt/factory.h>
#include <lt/serialize.h>

namespace LT_NAMESPACE {

	class Light : public Serializable {
	public:
		Light(const std::string& type) : type(type) {}
		virtual vec3 sample_light_direction() = 0;
		std::string type;
	};

	class DirectionnalLight : public Light {
	public:
		
		Float intensity = 0.5;
		vec3  dir       = vec3(1,0,0);

		DirectionnalLight() : Light("DirectionnalLight") {
			link_params();
		}

		vec3 sample_light_direction() {
			return dir;
		}


	protected:
		void link_params(){
			params.add("dir", Params::Type::VEC3, &dir);
			params.add("intensity", Params::Type::FLOAT, &intensity);
		}
	};

}