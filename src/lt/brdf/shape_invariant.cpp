#include "shape_invariant.h"

namespace LT_NAMESPACE {




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