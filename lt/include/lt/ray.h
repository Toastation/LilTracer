#pragma once

#include <lt/lt_common.h>

namespace LT_NAMESPACE {

	class SurfaceInteraction
	{
	public:
		SurfaceInteraction() : nor(vec3(0.)), pos(vec3(0.)), t(1000000.) {}
		SurfaceInteraction(vec3 pos, vec3 nor) : nor(nor), pos(pos), t(1000000.) {}
		vec3 nor;
		vec3 pos;
		float t;
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