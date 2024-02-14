#pragma once

#include <lt/lt_common.h>
#include <lt/ray.h>

namespace LT_NAMESPACE {

class Camera
{
public:
	Camera();
	~Camera();

	virtual Ray generate_ray(Float u, Float v) = 0;


};

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(vec3 pos, vec3 center, float fov) :
		_pos(pos),
		_center(center),
		_fov(fov)
	{
		_view = glm::lookAt(_pos, _center, vec3(0.,1.,0.));
		_inv_view = glm::inverse(_view);
		_proj = glm::perspective((double)_fov, 1., 0.0001, 1.);
		_inv_proj = glm::inverse(_proj);
	}
	

	// u in [-1 , 1]
	// v in [-1 , 1]
	Ray generate_ray(Float u, Float v) {		

		glm::vec4 d_eye = _inv_proj * glm::vec4(u, v, -1., 1.);
		d_eye.w = 0.;

		vec3 d = glm::vec3(_inv_view * d_eye);
		d = glm::normalize(d);

		return Ray(_pos,d);
	}

	vec3 _pos;
	vec3 _center;
	float _fov;
	glm::mat4 _view;
	glm::mat4 _proj;
	glm::mat4 _inv_view;
	glm::mat4 _inv_proj;
};




}
