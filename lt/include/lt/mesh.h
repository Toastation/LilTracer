#pragma once

#include <lt/lt_common.h>
#include <lt/ray.h>
#include <lt/brdf.h>


namespace LT_NAMESPACE {


	class Shape
	{
	public:
		virtual bool intersect(Ray r, SurfaceInteraction& si) = 0;
		std::shared_ptr<Brdf> brdf;

	};

	class Mesh : public Shape
	{
	public:
		bool intersect(Ray r, SurfaceInteraction& si) {
			return false;
		}
	};

	class Sphere : public Shape
	{
	public:
		Sphere(vec3 pos, float rad, std::shared_ptr<Brdf> brdf) : pos(pos), rad(rad) {
			this->brdf = brdf;
		}

		bool intersect(Ray r, SurfaceInteraction& si) {
			vec3 d = r.o - pos;
			float b = glm::dot(d, r.d);
			float c = glm::dot(d, d) - rad*rad;
			float t = b * b - c;
			if (t > 0.) {
				t = -b - std::sqrt(t);
				si.t = t;
				si.pos = r.o + t * r.d;
				si.nor = (si.pos - pos) / rad;
				si.brdf = brdf;
				return true;
			}
			return false;
		}

		vec3 pos;
		float rad;
	};

}
