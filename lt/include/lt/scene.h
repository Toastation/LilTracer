#pragma once

#include <lt/lt_common.h>
#include <lt/mesh.h>
#include <lt/ray.h>

namespace LT_NAMESPACE {

class Scene
{
public:
	Scene();
	~Scene();

	void intersect(Ray r) {

	}

	inline const std::vector<Mesh*> meshes() const;

private:
	
	std::vector<Mesh*> _meshes;

};


}
