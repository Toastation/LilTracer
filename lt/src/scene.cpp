#include <lt/scene.h>

namespace LT_NAMESPACE {

Scene::Scene()
{
	std::cout << "Creation de la scene" << std::endl;
}

Scene::~Scene()
{
}

inline const std::vector<Mesh*> Scene::meshes() const
{
	return _meshes;
}

}
