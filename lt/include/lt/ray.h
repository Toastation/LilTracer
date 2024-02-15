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
		Brdf* brdf;
	};


	class Ray
	{
	public:
		Ray(vec3 o, vec3 d) :
			o(o),
			d(d){}

		~Ray() {}

		vec3 o;
		vec3 d;


	};

}