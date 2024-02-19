#include <lt/mesh.h>

namespace LT_NAMESPACE {

	Factory<Shape>::CreatorRegistry& Factory<Shape>::registry() {
		static Factory<Shape>::CreatorRegistry registry{
			 {"Mesh"  , std::make_shared<Mesh>}
			,{"Sphere", std::make_shared<Sphere>}
		};
		return registry;
	}

}
