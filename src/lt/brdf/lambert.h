/**
 * @file brdf.h
 * @brief Defines classes related to Bidirectional Reflectance Distribution
 * Functions (BRDFs).
 */

#pragma once

#include "brdf.h"

namespace LT_NAMESPACE {

/**
 * @brief Diffuse BRDF class.
 */
class Diffuse : public Brdf {
public:
    PARAMETER(Spectrum, albedo, 0.5); /**< Albedo of the surface. */

    Diffuse()
        : Brdf("Diffuse")
    {
        flags = Flags::diffuse | Flags::reflection;
        link_params();
    }

    Spectrum eval(vec3 wi, vec3 wo, Sampler& sampler);
    Sample sample(const vec3& wi, Sampler& sampler);
    float pdf(const vec3& wi, const vec3& wo);

protected:
    void link_params() { params.add("albedo", Params::Type::VEC3, &albedo); }
};

} // namespace LT_NAMESPACE