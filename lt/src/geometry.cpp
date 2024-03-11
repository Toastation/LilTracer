#include <lt/geometry.h>

namespace LT_NAMESPACE {

Factory<Geometry>::CreatorRegistry& Factory<Geometry>::registry()
{
    static Factory<Geometry>::CreatorRegistry registry {
        { "Mesh", std::make_shared<Mesh> }, { "Sphere", std::make_shared<Sphere> }
    };
    return registry;
}

} // namespace LT_NAMESPACE
