#pragma once

#include <lt/lt_common.h>
#include <lt/params.h>
#include <lt/sampler.h>

namespace LT_NAMESPACE {
	#define PARAMETER(type,name,default_values) type name = type(default_values)

	
	class Brdf
	{
	public:
		Brdf();
		~Brdf();
		
		virtual vec3 eval(vec3 wi, vec3 wo) = 0;
		
		virtual vec3 sample(vec3 wi, vec3 wo, Sampler sampler);
		
		virtual float pdf(vec3 w);

		virtual void setup() = 0;

		Params params;
		std::string name;
	};

	class Diffuse : public Brdf {
	public:
		PARAMETER(float, albedo, 0.5);
		PARAMETER(float, intensity, 0.5);
		Diffuse() {
			setup();
		}
		
		vec3 eval(vec3 wi, vec3 wo) {
			return vec3(0., albedo, intensity) * wo.z;
		}

		void setup() {
			name = "Diffuse";
			params.add("albedo", Params::Type::FLOAT, &albedo);
			params.add("intensity", Params::Type::FLOAT, &intensity);
		}
	};

	class RoughConductor : public Brdf{
	public:
		PARAMETER(vec3 , albedo, 0.5);
		PARAMETER(std::vector<float>, albedi, 0);
		PARAMETER(float, albeda, 0.5);
		
		RoughConductor() {
			albedi.push_back(2.);
			albedi.push_back(1.);
			albedi.push_back(3.);
			albedi.push_back(4.);
			albedi.push_back(9.);
			setup();
		}

		vec3 eval(vec3 wi, vec3 wo) {
			return albeda * albedo * wo.z;
		}

		void setup() {
			name = "RoughConductor";
			params.add("albedo", Params::Type::VEC3, &albedo);
			params.add("albedi", Params::Type::SH, &albedi);
			params.add("albeda", Params::Type::FLOAT, &albeda);
		}

	};

}