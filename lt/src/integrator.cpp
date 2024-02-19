#include <lt/integrator.h>

namespace LT_NAMESPACE {

	Factory<Integrator>::CreatorRegistry& Factory<Integrator>::registry() {
		static Factory<Integrator>::CreatorRegistry registry{
			 {"DirectIntegrator", std::make_shared<DirectIntegrator>}
		};
		return registry;
	}

}
