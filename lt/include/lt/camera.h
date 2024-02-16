#pragma once

#include <lt/lt_common.h>
#include <lt/ray.h>
#include <lt/serialize.h>


namespace LT_NAMESPACE {

class Camera : public Serializable
{
public:
	Camera(const std::string& type) : type(type) {}

	virtual Ray generate_ray(Float u, Float v) = 0;

	std::string type;
};

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(vec3 pos, vec3 center, float fov, float aspect) :
		Camera("PerspectiveCamera"),
		pos(pos),
		center(center),
		fov(fov),
		aspect(aspect)
	{
		view = glm::lookAt(pos, center, vec3(0.,1.,0.));
		inv_view = glm::inverse(view);
		proj = glm::perspective((double)fov, (double)aspect, 0.0001, 1.);
		inv_proj = glm::inverse(proj);
	}
	

	// u in [-1 , 1]
	// v in [-1 , 1]
	Ray generate_ray(Float u, Float v) {		

		glm::vec4 d_eye = inv_proj * glm::vec4(u, v, -1., 1.);
		d_eye.w = 0.;

		vec3 d = glm::vec3(inv_view * d_eye);
		d = glm::normalize(d);

		return Ray(pos,d);
	}

	vec3 pos;
	vec3 center;
	float fov;
	float aspect;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 inv_view;
	glm::mat4 inv_proj;

protected:
	void link_params() {
		params.add("pos", Params::Type::VEC3, &pos);
		params.add("center", Params::Type::VEC3, &center);
		params.add("aspect", Params::Type::FLOAT, &aspect);
		params.add("fov", Params::Type::FLOAT, &fov);
	}

};




}
