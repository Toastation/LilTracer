#pragma once

#include <lt/lt_common.h>
#include <lt/ray.h>


namespace LT_NAMESPACE {


	class Object
	{
	public:
		virtual bool intersect(Ray r, SurfaceInteraction& si) = 0;

	};

	class Mesh : public Object
	{
	public:
		bool intersect(Ray r, SurfaceInteraction& si) {
			return false;
		}
	};

	class Sphere : public Object
	{
	public:
		Sphere(vec3 pos, float rad) : pos(pos), rad(rad) {}

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
				return true;
			}
			return false;
		}

	private:
		vec3 pos;
		float rad;
	};

}
