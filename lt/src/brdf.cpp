#include <lt/brdf.h>

namespace LT_NAMESPACE {

Factory<Brdf>::CreatorRegistry& Factory<Brdf>::registry()
{
    static Factory<Brdf>::CreatorRegistry registry {
        { "Emissive", std::make_shared<Emissive> },
        { "Diffuse", std::make_shared<Diffuse> },
        { "RoughGGX", std::make_shared<RoughGGX> },
        { "TestBrdf", std::make_shared<TestBrdf> }
    };
    return registry;
}

Spectrum Brdf::eval(vec3 wi, vec3 wo) { return Spectrum(0.); };
vec3 Brdf::sample(const vec3& wi, Sampler& s) { return square_to_cosine_hemisphere(s.next_float(), s.next_float()); }
float Brdf::pdf(const vec3& wi, const vec3& wo) { return square_to_cosine_hemisphere_pdf(wo); }
bool Brdf::emissive() { return false; }
Spectrum Brdf::emission() { return Spectrum(0.); }

Spectrum Diffuse::eval(vec3 wi, vec3 wo)
{
    return albedo / pi * glm::clamp(wo[2], 0.f, 1.f);
}

Spectrum TestBrdf::eval(vec3 wi, vec3 wo)
{
    return Spectrum(wo.z);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::scale_wi(const vec3& wi) const
{
    Float inv_sin_theta_sqr = 1.0 / (1.0 - wi.z * wi.z);
    Float cos_phi_sqr = wi.x * wi.x * inv_sin_theta_sqr;
    Float sin_phi_sqr = wi.y * wi.y * inv_sin_theta_sqr;
    return std::sqrt(cos_phi_sqr * scale.x * scale.x + sin_phi_sqr * scale.y * scale.y);
}

template <class MICROSURFACE>
vec3 ShapeInvariantMicrosurface<MICROSURFACE>::to_unit_space(const vec3& wi) { return glm::normalize(wi * scale); }

template <class MICROSURFACE>
vec3 ShapeInvariantMicrosurface<MICROSURFACE>::to_transformed_space(const vec3& wi) { return glm::normalize(wi / scale); }

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::G1(const vec3& wh, const vec3& wi){
    return ms.G1(to_transformed_space(wh),to_unit_space(wi));
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::D(const vec3& wh)
{
    Float det_m = 1. / std::abs(scale.x * scale.y);
    vec3 wh_l   = to_transformed_space(wh);
    return ms.D(wh_l) * det_m * std::pow(wh_l.z / wh.z, 4.);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::pdf_wh(const vec3& wh) { return D(wh) * wh.z; }

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::D(const vec3& wh, const vec3& wi)
{
    Float det_m = 1. / std::abs(scale.x * scale.y);
    vec3 wh_l = to_transformed_space(wh);
    vec3 wi_l = to_unit_space(wi);
    return ms.D(wh_l,wi_l) * det_m * std::pow(wh_l.z / wh.z, 3.);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::pdf_wh(const vec3& wh, const vec3& wi) { return D(wh,wi); }


template <class MICROSURFACE>
vec3 ShapeInvariantMicrosurface<MICROSURFACE>::sample_D(Sampler& sampler)
{
    return to_unit_space(ms.sample_D(sampler));
}

template <class MICROSURFACE>
vec3 ShapeInvariantMicrosurface<MICROSURFACE>::sample_D(const vec3& wi, Sampler& sampler)
{
    return to_unit_space(ms.sample_D(to_unit_space(wi),sampler));
}


template <class MICROSURFACE>
vec3 ShapeInvariantMicrosurface<MICROSURFACE>::sample(const vec3& wi, Sampler& sampler)
{
    vec3 wh = sample_visible_distribution ? sample_D(wi, sampler) : sample_D(sampler);
    return glm::reflect(-wi, wh);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::pdf(const vec3& wi, const vec3& wo)
{
    vec3 wh = glm::normalize(wi + wo);
    Float pdf_wh_ = sample_visible_distribution ? pdf_wh(wh,wi) : pdf_wh(wh);
    return pdf_wh_ / (4. * glm::clamp(glm::dot(wo, wh), 0.0001f, 0.9999f));
}

template <class MICROSURFACE>
Spectrum ShapeInvariantMicrosurface<MICROSURFACE>::eval(vec3 wi, vec3 wo)
{
    vec3 wh = glm::normalize(wi + wo);
    Float d = D(wh);
    Float g = G1(wh,wi) * G1(wh,wo);
    Spectrum f = Spectrum(1);
    Spectrum brdf = d * g * f / (4.f * glm::clamp(wi[2], 0.0001f, 0.9999f) * glm::clamp(wo[2], 0.0001f, 0.9999f));
    return brdf * glm::clamp(wo[2], 0.0001f, 0.9999f);
}



} // namespace LT_NAMESPACE