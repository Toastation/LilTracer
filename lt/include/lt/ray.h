#pragma once

#include <lt/lt_common.h>
#include <lt/brdf.h>

namespace LT_NAMESPACE {

	class SurfaceInteraction
	{
	public:
		SurfaceInteraction() : nor(vec3(0.)), pos(vec3(0.)), t(1000000.), brdf(nullptr) {}
		SurfaceInteraction(vec3 pos, vec3 nor) : nor(nor), pos(pos), t(1000000.), brdf(nullptr) {}
		vec3 nor;
		vec3 pos;
		float t;
		std::shared_ptr<Brdf> brdf;
		
		glm::mat3 tbn;
		glm::mat3 inv_tbn;
		vec3 tan, bitan;
		
		void finalize() {
			orthonormal_basis(nor, tan, bitan);
			tbn = glm::mat3(tan, bitan, nor);
			inv_tbn = glm::transpose(tbn);
		}

		vec3 to_world(const vec3& v) {
			return tbn * v;
		}

		vec3 to_local(const vec3& v) {
			return inv_tbn * v;
		}

	};


	class Ray
	{
	public:
		Ray() : o(vec3(0.)), d(vec3(0.)) {}
		Ray(vec3 o, vec3 d) :
			o(o),
			d(d){}

		~Ray() {}

		vec3 o;
		vec3 d;


	};

}