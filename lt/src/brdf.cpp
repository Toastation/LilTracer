#include <lt/brdf.h>

namespace LT_NAMESPACE {

vec3 Brdf::sample(const vec3 &wi, Sampler &s) {
  return square_to_cosine_hemisphere(s.next_float(), s.next_float());
}

float Brdf::pdf(vec3 w) { return square_to_cosine_hemisphere_pdf(w); }

Factory<Brdf>::CreatorRegistry &Factory<Brdf>::registry() {
  static Factory<Brdf>::CreatorRegistry registry{
      {"Diffuse", std::make_shared<Diffuse>},
      {"RoughConductor", std::make_shared<RoughConductor>},
      {"TestBrdf", std::make_shared<TestBrdf>}};
  return registry;
}

}  // namespace LT_NAMESPACE