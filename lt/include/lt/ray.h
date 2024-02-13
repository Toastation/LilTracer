#pragma once

#include <lt/lt_common.h>

namespace LT_NAMESPACE {

class Ray
{
public:
	Ray(vec3 o, vec3 d) :
		o(o),
		d(d){}

	~Ray() {}

	vec3 o;
	vec3 d;

private:

};

}