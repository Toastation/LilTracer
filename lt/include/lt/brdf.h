#pragma once

#include <lt/lt_common.h>

namespace LT_NAMESPACE {
	#define PARAMETER(type,name,default_values) type name = type(default_values)

	struct Params {
		enum class Type
		{
			FLOAT,
			VEC3
		};

		int count = 0;
		std::vector<void*> ptrs;
		std::vector<Type> types;
		std::vector<std::string> names;

		void add(const std::string& name, Type type, void* ptr) {
			count++; 
			ptrs.push_back(ptr);
			types.push_back(type);
			names.push_back(name);
		}
	};

	class Brdf
	{
	public:
		Brdf();
		~Brdf();
		
		virtual vec3 eval(vec3 wi, vec3 wo) = 0;

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
		PARAMETER(vec3, albedi, 0.5);
		PARAMETER(vec3 , albedo, 0.5);
		PARAMETER(float, albeda, 0.5);
		
		RoughConductor() {
			setup();
		}

		vec3 eval(vec3 wi, vec3 wo) {
			return albeda * albedo * wo.z;
		}

		void setup() {
			name = "RoughConductor";
			params.add("albedo", Params::Type::VEC3, &albedo);
			params.add("albedi", Params::Type::VEC3, &albedi);
			params.add("albeda", Params::Type::FLOAT, &albeda);
		}

	};

}