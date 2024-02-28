#include <lt/integrator.h>

namespace LT_NAMESPACE {

Factory<Integrator>::CreatorRegistry &Factory<Integrator>::registry() {
  static Factory<Integrator>::CreatorRegistry registry{
      {"PathIntegrator", std::make_shared<PathIntegrator>},
      {"DirectIntegrator", std::make_shared<DirectIntegrator>},
      {"AOIntegrator", std::make_shared<AOIntegrator>}};
  return registry;
}

}  // namespace LT_NAMESPACE
