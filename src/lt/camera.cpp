#include <lt/camera.h>

namespace LT_NAMESPACE {

Factory<Camera>::CreatorRegistry& Factory<Camera>::registry()
{
    static Factory<Camera>::CreatorRegistry registry {
        { "PerspectiveCamera", std::make_shared<PerspectiveCamera> }
    };
    return registry;
}


void PerspectiveCamera::init()
{
    view = glm::lookAt(pos, center, vec3(0., 1., 0.));
    inv_view = glm::inverse(view);
    proj = glm::perspective((double)fov * pi / 180.f, (double)aspect, 0.0001, 1.);
    inv_proj = glm::inverse(proj);
}


Ray PerspectiveCamera::generate_ray(Float u, Float v)
{
    glm::vec4 d_eye = inv_proj * glm::vec4(u, v, -1., 1.);
    d_eye.w = 0.;

    vec3 d = glm::vec3(inv_view * d_eye);
    d = glm::normalize(d);

    return Ray(pos, d);
}


} // namespace LT_NAMESPACE