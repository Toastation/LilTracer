#pragma once

#include <lt/lt_common.h>
#include <lt/mesh.h>
#include <lt/light.h>
#include <lt/ray.h>

namespace LT_NAMESPACE {

class Scene
{
public:
	Scene();
	~Scene();

	bool intersect(const Ray& r, SurfaceInteraction& si) {

		SurfaceInteraction si_t;
		
		bool has_intersect = false;

		for (int i = 0; i < objs.size(); i++) {
			if (objs[i]->intersect(r, si_t) && si_t.t < si.t) {
				si = si_t;
				has_intersect = true;
			}
		}

		return has_intersect;
		
	}

	std::vector<Object*> objs;
	std::vector<Light*> lights;


};


class CornellBox : public Scene
{
public:
	CornellBox() {
		//objs.push_back(new Sphere(vec3(0.), 2.));
		//objs.push_back(new Sphere(vec3(0., 4., 0.), 2.));
		//objs.push_back(new Sphere(vec3(0., 0., 3.), 1.));
	};

private:

};


//inline Scene cornell_box() {
//
//	Scene scn;
//
//	scn.objs.push_back(new Sphere(vec3(0.), 2.));
//	scn.objs.push_back(new Sphere(vec3(0.,4.,0.), 2.));
//	scn.objs.push_back(new Sphere(vec3(0.,0.,3.), 1.));
//
//	return scn;
//
//}


}
