#include <lt/light.h>

namespace LT_NAMESPACE {

Factory<Light>::CreatorRegistry &Factory<Light>::registry() {
  static Factory<Light>::CreatorRegistry registry{
      {"DirectionnalLight", std::make_shared<DirectionnalLight>}};
  return registry;
}

}  // namespace LT_NAMESPACE