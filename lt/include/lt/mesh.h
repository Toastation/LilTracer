#pragma once

#include <lt/lt_common.h>
#include <lt/ray.h>
#include <lt/brdf.h>


namespace LT_NAMESPACE {

	class Shape : public Serializable
	{
	public:
		Shape(const std::string& type) : Serializable(type) {};

		virtual bool intersect(Ray r, SurfaceInteraction& si) = 0;
		std::shared_ptr<Brdf> brdf;
	};


	class Mesh : public Shape
	{
	public:
		Mesh() : Shape("Mesh") {
			link_params();
		};

		bool intersect(Ray r, SurfaceInteraction& si) {
			return false;
		}

		void init() {};

		std::string filename;

	protected:
		void link_params() {
			params.add("filename", Params::Type::PATH, &filename);
			params.add("brdf", Params::Type::BRDF, &brdf);
		}

	};

	class Sphere : public Shape
	{
	public:
		Sphere() :
			Shape("Sphere"),
			pos(vec3(0.)),
			rad(1.)
		{
			link_params();

			this->brdf = std::shared_ptr<Brdf>(nullptr);
		};

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


	protected:
		void link_params() {
			params.add("pos", Params::Type::VEC3 , &pos);
			params.add("rad", Params::Type::FLOAT, &rad);
			params.add("brdf", Params::Type::BRDF, &brdf);
		}

	};

}
