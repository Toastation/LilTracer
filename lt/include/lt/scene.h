#pragma once

#include <lt/lt_common.h>
#include <lt/mesh.h>
#include <lt/light.h>
#include <lt/brdf.h>



namespace LT_NAMESPACE {

class Scene
{
public:

	bool intersect(const Ray& r, SurfaceInteraction& si) {

		SurfaceInteraction si_t;
		
		bool has_intersect = false;

		for (int i = 0; i < shapes.size(); i++) {
			if (shapes[i]->intersect(r, si_t) && si_t.t < si.t) {
				si = si_t;
				has_intersect = true;
			}
		}

		return has_intersect;
		
	}

	void init() {
	}


	std::vector<std::shared_ptr<Shape>> shapes;
	std::vector<std::shared_ptr<Light>> lights;
	std::vector<std::shared_ptr<Brdf>>  brdfs;
};



}
