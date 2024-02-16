#include <lt/brdf.h>

namespace LT_NAMESPACE {


vec3 Brdf::sample(vec3 wi, vec3 wo, Sampler s) {
	return square_to_uniform_hemisphere(s.next_float(), s.next_float());
}

float Brdf::pdf(vec3 w) {
	return square_to_cosine_hemisphere_pdf(w);
}


Factory<Brdf>::CreatorRegistry& Factory<Brdf>::registry() {
	static Factory<Brdf>::CreatorRegistry registry{ 
		 {"Diffuse"       , std::make_shared<Diffuse>}
		,{"RoughConductor", std::make_shared<RoughConductor>}
		,{"TestBrdf", std::make_shared<TestBrdf>}
	};
	return registry;
}


}