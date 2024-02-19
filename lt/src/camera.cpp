#include <lt/camera.h>

namespace LT_NAMESPACE {

	Factory<Camera>::CreatorRegistry& Factory<Camera>::registry() {
		static Factory<Camera>::CreatorRegistry registry{
			 {"PerspectiveCamera", std::make_shared<PerspectiveCamera>}
		};
		return registry;
	}

}