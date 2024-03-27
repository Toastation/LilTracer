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
Spectrum Brdf::eval(vec3 wi, vec3 wo) 
{
    return Spectrum(0.); 
};

Brdf::Sample Brdf::sample(const vec3& wi, Sampler& s)
{ 
    Sample bs;
    bs.wo = square_to_cosine_hemisphere(s.next_float(), s.next_float());
    bs.value = eval(wi, bs.wo) / pdf(wi, bs.wo);
    return bs;
}

float Brdf::pdf(const vec3& wi, const vec3& wo) 
{
    return square_to_cosine_hemisphere_pdf(wo); 
}

Spectrum Brdf::emission() 
{
    return Spectrum(0.); 
}

/////////////////////
// Emissive 
///////////////////
Spectrum Emissive::emission()
{
    return intensity;
}

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
Float ShapeInvariantMicrosurface<MICROSURFACE>::G2(const vec3& wh, const vec3& wi, const vec3& wo)
{
    return G1(wh, wi) * G1(wh, wo);
    return ms.G2(to_transformed_space(wh), to_unit_space(wi), to_unit_space(wo));
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::D(const vec3& wh)
{
    Float det_m = 1. / std::abs(scale.x * scale.y);
    vec3 wh_u   = to_transformed_space(wh);
    return ms.D(wh_u) * det_m * std::pow(wh_u.z / wh.z, 4.);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::pdf_wh(const vec3& wh) 
{ 
    Float det_m = 1. / std::abs(scale.x * scale.y);
    vec3 wh_u = to_transformed_space(wh);
    return ms.pdf(wh_u) * det_m * std::pow(wh_u.z / wh.z, 3.);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::D(const vec3& wh, const vec3& wi)
{
    Float det_m = 1. / std::abs(scale.x * scale.y);
    vec3 wh_u = to_transformed_space(wh);
    vec3 wi_u = to_unit_space(wi);
    return ms.D(wh_u,wi_u) * det_m * std::pow(wh_u.z / wh.z, 3.);
}

template <class MICROSURFACE>
Float ShapeInvariantMicrosurface<MICROSURFACE>::pdf_wh(const vec3& wh, const vec3& wi) 
{
    Float det_m = 1. / std::abs(scale.x * scale.y);
    vec3 wh_u = to_transformed_space(wh);
    vec3 wi_u = to_unit_space(wi);
    return ms.pdf(wh_u, wi_u) * det_m * std::pow(wh_u.z / wh.z, 3.);
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
Float ShapeInvariantMicrosurface<MICROSURFACE>::pdf(const vec3& wi, const vec3& wo)
{
    vec3 wh = glm::normalize(wi + wo);
    Float pdf_wh_ = sample_visible_distribution ? pdf_wh(wh,wi) : pdf_wh(wh);
    return pdf_wh_ / (4. * glm::clamp(glm::dot(wh, wi), 0.0001f, 0.9999f));
}

/////////////////////
// RoughShapeInvariantMicrosurface<MICROSURFACE>
///////////////////

template <class MICROSURFACE>
Spectrum RoughShapeInvariantMicrosurface<MICROSURFACE>::eval(vec3 wi, vec3 wo)
{
    vec3 wh = glm::normalize(wi + wo);
    Float d = D(wh);
    Float g = G2(wh,wi,wo);
    Spectrum f = fresnelConductor(glm::dot(wh,wi),eta,kappa);
    Spectrum brdf = d * g * f / (4.f * glm::clamp(wi[2], 0.0001f, 0.9999f) * glm::clamp(wo[2], 0.0001f, 0.9999f));
    return brdf * glm::clamp(wo[2], 0.0001f, 0.9999f);
}


template <class MICROSURFACE>
Brdf::Sample RoughShapeInvariantMicrosurface<MICROSURFACE>::sample(const vec3& wi, Sampler& sampler)
{
    Sample bs;
    vec3 wh = sample_visible_distribution ? sample_D(wi, sampler) : sample_D(sampler);
    bs.wo = glm::reflect(-wi, wh);
    bs.value = eval(wi, bs.wo) / pdf(wi, bs.wo);

    //Float g = sample_visible_distribution ? G2(wh,wi, bs.wo)  / G1(wh,bs.wo) : G1(wh, wi) * G1(wh, bs.wo) * glm::dot(wi, wh) / (glm::clamp(wi[2], 0.0001f, 0.9999f) * glm::clamp(wh[2], 0.0001f, 0.9999f));
    //bs.value = F * e;

    return bs;
}


/////////////////////
// SphereMicrosurface
///////////////////

Float SphereMicrosurface::lambda(const vec3& wi_u)
{
    Float cos_sqr = glm::clamp(wi_u.z * wi_u.z, 0.0001f, 0.9999f);
    Float tan_sqr = (1. - cos_sqr) / cos_sqr;
    return (-1. + std::sqrt(1. + tan_sqr)) / 2.;
}

Float SphereMicrosurface::G1(const vec3& wh_u, const vec3& wi_u) {
    return 1. / (1. + lambda(wi_u));
}

Float SphereMicrosurface::G2(const vec3& wh_u, const vec3& wi_u, const vec3& wo_u) {
    // [Ross05]
    return 1. / (1. + lambda(wi_u) + lambda(wo_u));
}

Float SphereMicrosurface::D(const vec3& wh_u) {
    return 1. / pi; 
}

Float SphereMicrosurface::pdf(const vec3& wh_u)
{
    return square_to_cosine_hemisphere_pdf(wh_u);
}
vec3 SphereMicrosurface::sample_D(Sampler& sampler)
{
    return square_to_cosine_hemisphere(sampler.next_float(), sampler.next_float());
}

Float SphereMicrosurface::D(const vec3& wh_u, const vec3& wi_u)
{
    return G1(wh_u, wi_u) * glm::clamp(glm::dot(wh_u, wi_u), 0.f, 1.f) / (wi_u.z * pi);
}

Float SphereMicrosurface::pdf(const vec3& wh_u, const vec3& wi_u)
{
    return D(wh_u, wi_u);
}

// Sampling method from Sampling Visible GGX Normals with Spherical Caps, Jonathan Dupuy, Anis Benyoub
vec3 SphereMicrosurface::sample_D(const vec3& wi_u, Sampler& sampler)
{
    float phi = 2. * pi * sampler.next_float();
    float z = std::fma(1. - sampler.next_float(), 1 + wi_u.z, -wi_u.z);
    float sin_theta = std::sqrt(std::clamp(1. - z * z, 0., 1.));
    float x = sin_theta * std::cos(phi);
    float y = sin_theta * std::sin(phi);
    return glm::normalize(wi_u + vec3(x, y, z));
}

/////////////////////
// RoughGGX
///////////////////

 // RoughGGX = ShapeInvariantMicrosurface<SphereMicrosurface>


/////////////////////
// BeckmannMicrosurface
///////////////////

Float BeckmannMicrosurface::D(const vec3& wh_u) {
    Float cos_theta_h_sqr = wh_u.z * wh_u.z;
    Float tan_theta_h_sqr = (1 - cos_theta_h_sqr) / cos_theta_h_sqr;
    return std::exp(-tan_theta_h_sqr) / (cos_theta_h_sqr* cos_theta_h_sqr * pi);
}

Float BeckmannMicrosurface::pdf(const vec3& wh_u)
{
    return D(wh_u) * wh_u.z;
}
vec3 BeckmannMicrosurface::sample_D(Sampler& sampler)
{
    Float log_sample = std::log(1 - sampler.next_float());
    if (std::isinf(log_sample)) log_sample = 0;
    Float tan_theta_sqr = -log_sample;
    Float phi = sampler.next_float() * 2 * pi;
    Float cos_theta = 1 / std::sqrt(1 + tan_theta_sqr);
    Float sin_theta = std::sqrt(std::max((Float)0, 1 - cos_theta * cos_theta));
    vec3 wh_u = vec3(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);
    return wh_u;
}

Float BeckmannMicrosurface::lambda(const vec3& wi_u)
{
    Float tan_theta_i_sqr = (1 - wi_u.z * wi_u.z) / (wi_u.z * wi_u.z);
    Float abs_tan_theta = std::abs(std::sqrt(tan_theta_i_sqr));
    if (std::isinf(abs_tan_theta)) return 0.;

    Float a = 1 / (abs_tan_theta);
    if (a >= 1.6f)
        return 0;
    return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}

Float BeckmannMicrosurface::G1(const vec3& wh_u, const vec3& wi_u) {
    return 1. / (1. + lambda(wi_u));
}

Float BeckmannMicrosurface::G2(const vec3& wh_u, const vec3& wi_u, const vec3& wo_u) {
    // [Ross05]
    return 1. / (1. + lambda(wi_u) + lambda(wo_u));
}

Float BeckmannMicrosurface::D(const vec3& wh_u, const vec3& wi_u)
{
    return D(wh_u);
}

Float BeckmannMicrosurface::pdf(const vec3& wh_u, const vec3& wi_u)
{
    return pdf(wh_u);
}

vec3 BeckmannMicrosurface::sample_D(const vec3& wi_u, Sampler& sampler)
{
    return sample_D(sampler);
}


/////////////////////
// RoughBeckmann
///////////////////

 // RoughBeckmann = ShapeInvariantMicrosurface<BeckmannMicrosurface>



} // namespace LT_NAMESPACE