#include <lt/brdf.h>

namespace LT_NAMESPACE {

/////////////////////
// Brdf Factory
///////////////////
Factory<Brdf>::CreatorRegistry& Factory<Brdf>::registry()
{
    static Factory<Brdf>::CreatorRegistry registry {
        { "Emissive", std::make_shared<Emissive> },
        { "Diffuse", std::make_shared<Diffuse> },
        { "RoughGGX", std::make_shared<RoughGGX> },
        { "RoughBeckmann", std::make_shared<RoughBeckmann> },
        { "TestBrdf", std::make_shared<TestBrdf> }
    };
    return registry;
}

/////////////////////
// Base 
///////////////////
Spectrum Brdf::eval(vec3 wi, vec3 wo) { return Spectrum(0.); };
vec3 Brdf::sample(const vec3& wi, Sampler& s) { return square_to_cosine_hemisphere(s.next_float(), s.next_float()); }
float Brdf::pdf(const vec3& wi, const vec3& wo) { return square_to_cosine_hemisphere_pdf(wo); }
bool Brdf::emissive() { return false; }
Spectrum Brdf::emission() { return Spectrum(0.); }


/////////////////////
// Diffuse 
///////////////////
Spectrum Diffuse::eval(vec3 wi, vec3 wo)
{
    return albedo / pi * glm::clamp(wo[2], 0.f, 1.f);
}

/////////////////////
// TestBrdf 
///////////////////
Spectrum TestBrdf::eval(vec3 wi, vec3 wo)
{
    return Spectrum(wo.z);
}

/////////////////////
// ShapeInvariantMicrosurface<MICROSURFACE>
///////////////////
template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::scale_wi(const vec3& wi) const
{
    Float inv_sin_theta_sqr = 1.0 / (1.0 - wi.z * wi.z);
    Float cos_phi_sqr = wi.x * wi.x * inv_sin_theta_sqr;
    Float sin_phi_sqr = wi.y * wi.y * inv_sin_theta_sqr;
    return std::sqrt(cos_phi_sqr * scale.x * scale.x + sin_phi_sqr * scale.y * scale.y);
}

template <class MICROSURFACE>
vec3 ShapeInvariantMicrosurface<MICROSURFACE>::to_unit_space(const vec3& wi) 
{
    return glm::normalize(wi * scale);
}

template <class MICROSURFACE>
vec3 ShapeInvariantMicrosurface<MICROSURFACE>::to_transformed_space(const vec3& wi) 
{ 
    return glm::normalize(wi / scale);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::G1(const vec3& wh, const vec3& wi)
{
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
Float ShapeInvariantMicrosurface<MICROSURFACE>::pdf_wh(const vec3& wh) 
{ 
    Float det_m = 1. / std::abs(scale.x * scale.y);
    vec3 wh_l = to_transformed_space(wh);
    return ms.pdf(wh_l) * det_m * std::pow(wh_l.z / wh.z, 3.);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::D(const vec3& wh, const vec3& wi)
{
    Float det_m = 1. / std::abs(scale.x * scale.y);
    vec3 wh_l = to_transformed_space(wh);
    vec3 wi_l = to_unit_space(wi);
    return ms.D(wh_l,wi_l) * det_m * std::pow(wh_l.z / wh.z, 3.);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::pdf_wh(const vec3& wh, const vec3& wi) 
{
    Float det_m = 1. / std::abs(scale.x * scale.y);
    vec3 wh_l = to_transformed_space(wh);
    vec3 wi_l = to_unit_space(wi);
    return ms.pdf(wh_l, wi_l) * det_m * std::pow(wh_l.z / wh.z, 3.);
}


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


/////////////////////
// SphereMicrosurface
///////////////////

Float SphereMicrosurface::D(const vec3& wh) { 
    return 1. / pi; 
}

Float SphereMicrosurface::pdf(const vec3& wh)
{
    return square_to_cosine_hemisphere_pdf(wh);
}
vec3 SphereMicrosurface::sample_D(Sampler& sampler)
{
    return square_to_cosine_hemisphere(sampler.next_float(), sampler.next_float());
}

Float SphereMicrosurface::lambda(const vec3& wi)
{
    Float cos_sqr = glm::clamp(wi.z * wi.z, 0.0001f, 0.9999f);
    Float tan_sqr = (1. - cos_sqr) / cos_sqr;
    return (-1. + std::sqrt(1. + tan_sqr)) / 2.;
}

Float SphereMicrosurface::G1(const vec3& wh, const vec3& wi) {
    return 1. / (1. + lambda(wi));
}

Float SphereMicrosurface::D(const vec3& wh, const vec3& wi) 
{
    return G1(wh, wi) * glm::clamp(glm::dot(wi, wh), 0.f, 1.f) / (wi.z * pi);
}

Float SphereMicrosurface::pdf(const vec3& wh, const vec3& wi)
{
    return D(wh, wi);
}

// Sampling method from Sampling Visible GGX Normals with Spherical Caps, Jonathan Dupuy, Anis Benyoub
vec3 SphereMicrosurface::sample_D(const vec3& wi, Sampler& sampler)
{
    float phi = 2. * pi * sampler.next_float();
    float z = std::fma(1. - sampler.next_float(), 1 + wi.z, -wi.z);
    float sin_theta = std::sqrt(std::clamp(1. - z * z, 0., 1.));
    float x = sin_theta * std::cos(phi);
    float y = sin_theta * std::sin(phi);
    return glm::normalize(wi + vec3(x, y, z));
}

/////////////////////
// RoughGGX
///////////////////

 // RoughGGX = ShapeInvariantMicrosurface<SphereMicrosurface>


/////////////////////
// BeckmannMicrosurface
///////////////////

Float BeckmannMicrosurface::D(const vec3& wh) {
    Float cos_theta_h_sqr = wh.z * wh.z;
    Float tan_theta_h_sqr = (1 - cos_theta_h_sqr) / cos_theta_h_sqr;
    return std::exp(-tan_theta_h_sqr) / (cos_theta_h_sqr* cos_theta_h_sqr * pi);
}

Float BeckmannMicrosurface::pdf(const vec3& wh)
{
    return D(wh) * wh.z;
}
vec3 BeckmannMicrosurface::sample_D(Sampler& sampler)
{
    Float log_sample = std::log(1 - sampler.next_float());
    if (std::isinf(log_sample)) log_sample = 0;
    Float tan_theta_sqr = -log_sample;
    Float phi = sampler.next_float() * 2 * pi;
    Float cos_theta = 1 / std::sqrt(1 + tan_theta_sqr);
    Float sin_theta = std::sqrt(std::max((Float)0, 1 - cos_theta * cos_theta));
    vec3 wh = vec3(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);
    return wh;
}

Float BeckmannMicrosurface::lambda(const vec3& wi)
{
    Float tan_theta_i_sqr = (1 - wi.z * wi.z) / (wi.z * wi.z);
    Float abs_tan_theta = std::abs(std::sqrt(tan_theta_i_sqr));
    if (std::isinf(abs_tan_theta)) return 0.;

    Float a = 1 / (abs_tan_theta);
    if (a >= 1.6f)
        return 0;
    return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}

Float BeckmannMicrosurface::G1(const vec3& wh, const vec3& wi) {
    return 1. / (1. + lambda(wi));
}

Float BeckmannMicrosurface::D(const vec3& wh, const vec3& wi)
{
    return D(wh);
}

Float BeckmannMicrosurface::pdf(const vec3& wh, const vec3& wi)
{
    return pdf(wh);
}

vec3 BeckmannMicrosurface::sample_D(const vec3& wi, Sampler& sampler)
{
    return sample_D(sampler);
}


/////////////////////
// RoughBeckmann
///////////////////

 // RoughBeckmann = ShapeInvariantMicrosurface<BeckmannMicrosurface>

} // namespace LT_NAMESPACE