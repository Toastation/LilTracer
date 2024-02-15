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
		
		virtual Spectrum eval(vec3 wi, vec3 wo) = 0;
		
		virtual vec3 sample(vec3 wi, vec3 wo, Sampler sampler);
		
		virtual float pdf(vec3 w);

		virtual void setup() = 0;

		Params params;
		std::string name;
	};

	class Diffuse : public Brdf {
	public:
		PARAMETER(Spectrum, albedo, 0.5);
		Diffuse() {
			setup();
		}
		
		Spectrum eval(vec3 wi, vec3 wo) {
			return albedo * wo.z;
		}

		void setup() {
			name = "Diffuse";
			params.add("albedo", Params::Type::VEC3, &albedo);
		}
	};

	class RoughConductor : public Brdf {
	public:
		PARAMETER(Float, alpha_x, 0.5);
		PARAMETER(Float, alpha_y, 0.5);
		PARAMETER(Spectrum, albedo, 0.5);

		RoughConductor() {
			setup();
		}

		Spectrum eval(vec3 wi, vec3 wo) {
			return albedo * wo.z;
		}

		void setup() {
			name = "RoughConductor";
			params.add("albedo", Params::Type::VEC3, &albedo);
			params.add("alpha_x", Params::Type::FLOAT, &alpha_x);
			params.add("alpha_y", Params::Type::FLOAT, &alpha_y);
		}
	};

	class TestBrdf : public Brdf{
	public:
		PARAMETER(float, v1, 0.5);
		PARAMETER(Spectrum, v2, 0.5);
		PARAMETER(std::vector<float>, v3, 0);
		
		TestBrdf() {
			v3.push_back(2.);
			v3.push_back(1.);
			v3.push_back(3.);
			v3.push_back(4.);
			v3.push_back(9.);
			setup();
		}

		Spectrum eval(vec3 wi, vec3 wo) {
			return Spectrum(wo.z);
		}

		void setup() {
			name = "TestBrdf";
			params.add("float", Params::Type::FLOAT, &v1);
			params.add("vec3", Params::Type::VEC3, &v2);
			params.add("array", Params::Type::SH, &v3);
		}

	};

}