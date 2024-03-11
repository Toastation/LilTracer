/**
 * @file brdf.h
 * @brief Defines classes related to Bidirectional Reflectance Distribution
 * Functions (BRDFs).
 */

#pragma once

#include <lt/factory.h>
#include <lt/lt_common.h>
#include <lt/sampler.h>
#include <lt/serialize.h>

namespace LT_NAMESPACE {
#define PARAMETER(type, name, default_values) type name = type(default_values)

/**
 * @brief Base class for Bidirectional Reflectance Distribution Functions
 * (BRDFs).
 */
class Brdf : public Serializable {
public:
    /**
     * @brief Constructor.
     * @param type The type of the BRDF.
     */
    Brdf(const std::string& type)
        : Serializable(type) {};

    /**
     * @brief Evaluates the BRDF.
     * @param wi Incident direction.
     * @param wo Outgoing direction.
     * @return The evaluated spectrum.
     */
    virtual Spectrum eval(vec3 wi, vec3 wo) { return Spectrum(0.); };

    /**
     * @brief Samples the BRDF.
     * @param wi Incident direction.
     * @param sampler The sampler object used for sampling.
     * @return The sampled direction.
     */
    virtual vec3 sample(const vec3& wi, Sampler& sampler);

    /**
     * @brief Computes the density of a sample of wo.
     * @param wo Direction.
     * @return The density value.
     */
    virtual float pdf(const vec3& wi, const vec3& wo);

    virtual bool emissive() { return false; }
    virtual Spectrum emission() { return Spectrum(0.); }
};

class Emissive : public Brdf {
public:
    PARAMETER(Spectrum, intensity, 1.); /**< Albedo of the surface. */

    Emissive()
        : Brdf("Emissive")
    {
        link_params();
    }

    bool emissive() { return true; }
    Spectrum emission() { return intensity; }

protected:
    void link_params()
    {
        params.add("intensity", Params::Type::VEC3, &intensity);
    }
};

/**
 * @brief Diffuse BRDF class.
 */
class Diffuse : public Brdf {
public:
    PARAMETER(Spectrum, albedo, 0.5); /**< Albedo of the surface. */

    Diffuse()
        : Brdf("Diffuse")
    {
        link_params();
    }

    Spectrum eval(vec3 wi, vec3 wo)
    {
        return albedo / pi * glm::clamp(wo[2], 0.f, 1.f);
    }

protected:
    void link_params() { params.add("albedo", Params::Type::VEC3, &albedo); }
};

/**
 * @brief Rough Conductor BRDF class.
 */
class RoughConductor : public Brdf {
public:
    PARAMETER(Float, alpha_x, 0.5); /**< Roughness value in x-direction. */
    PARAMETER(Float, alpha_y, 0.5); /**< Roughness value in y-direction. */

    RoughConductor()
        : Brdf("RoughConductor")
    {
        link_params();
    }

    static Float D_ggx(const vec3& wh, const Float& ax, const Float& ay)
    {
        return wh[2];
    }

    static Float G1_ggx(const vec3& w, const Float& ax, const Float& ay)
    {
        return 1.;
    }

    Spectrum eval(vec3 wi, vec3 wo)
    {
        vec3 wh = glm::normalize(wi + wo);

        Float D = D_ggx(wh, alpha_x, alpha_y);
        Float G = G1_ggx(wi, alpha_x, alpha_y) * G1_ggx(wi, alpha_x, alpha_y);
        return vec3(
            D * G / (4. * glm::clamp(wi[2], 0.f, 1.f) * glm::clamp(wo[2], 0.f, 1.f)));
    }

protected:
    void link_params()
    {
        params.add("alpha_x", Params::Type::FLOAT, &alpha_x);
        params.add("alpha_y", Params::Type::FLOAT, &alpha_y);
    }
};

/**
 * @brief Test BRDF class for demonstration purposes.
 */
class TestBrdf : public Brdf {
public:
    PARAMETER(float, v1, 0.5); /**< Test parameter 1. (float) */
    PARAMETER(Spectrum, v2, 0.5); /**< Test parameter 2. (Spectrum) */
    PARAMETER(std::vector<float>, v3, 0); /**< Test parameter 3. (Array<float>) */

    TestBrdf()
        : Brdf("TestBrdf")
    {
        v3.push_back(2.);
        v3.push_back(1.);
        v3.push_back(3.);
        v3.push_back(4.);
        v3.push_back(9.);
        link_params();
    }

    Spectrum eval(vec3 wi, vec3 wo) { return Spectrum(wo.z); }

protected:
    void link_params()
    {
        params.add("float", Params::Type::FLOAT, &v1);
        params.add("vec3", Params::Type::VEC3, &v2);
        params.add("array", Params::Type::SH, &v3);
    }
};

// class Microsurface {
// public:
//
//     virtual Float D(const vec3& wh) = 0;
//     virtual Float pdf(const vec3& wh) = 0;
//     virtual vec3  sample_D(Sampler& sampler) = 0;
//
//     //virtual Float sigma(const vec3& wi) = 0;
//     /*Float D(const vec3& wh, const vec3& wi) { return 0.; }
//     vec3 sample_D(Sampler& sampler, const vec3& wi) { return vec3(0.); }
//     Float pdf(const vec3& wh, const vec3& wi) { return 0.; }
//
//     Float G_1(const vec3& wi) { return 0.; }
//     Float G_1(const vec3& wi, const vec3& wh) { return 0.; }
//     Float G_2(const vec3& wi, const vec3& wo) { return 0.; }
//     Float G_2(const vec3& wi, const vec3& wo, const vec3& wh) { return 0.;
//     }*/
// };

template <class MICROSURFACE>
class ShapeInvariantMicrosurface : public Brdf {
public:
    vec3 scale;

    ShapeInvariantMicrosurface(const std::string& type, const Float& scale_x,
        const Float& scale_y)
        : Brdf(type)
    {
        scale = vec3(scale_x, scale_y, 1.);
    }

    virtual Float sigma(const vec3& wi) = 0;
    virtual Float lambda(const vec3& wi, const float& alpha) = 0;
    virtual Float G1(const vec3& wi) = 0;

    Float scale_wi(const vec3& wi) const
    {
        Float inv_sin_theta_sqr = 1.0 / (1.0 - wi.z * wi.z);
        Float cos_phi_sqr = wi.x * wi.x * inv_sin_theta_sqr;
        Float sin_phi_sqr = wi.y * wi.y * inv_sin_theta_sqr;
        return std::sqrt(cos_phi_sqr * scale.x * scale.x + sin_phi_sqr * scale.y * scale.y);
    }

    vec3 to_stretched_space(const vec3& wi) { return glm::normalize(wi * scale); }

    vec3 to_unit_space(const vec3& wi) { return glm::normalize(wi / scale); }

    Float D(const vec3& wh)
    {
        Float det_m = 1. / std::abs(scale.x * scale.y);
        vec3 wu = to_unit_space(wh);
        return MICROSURFACE::D(wu) * det_m * std::pow(wu.z / wh.z, 4.);
    }

    Float pdf(const vec3& wh) { return D(wh) * wh.z; }

    vec3 sample_D(Sampler& sampler)
    {
        return to_stretched_space(MICROSURFACE::sample_D(sampler));
    }

    vec3 sample(const vec3& wi, Sampler& sampler)
    {
        return glm::reflect(-wi, sample_D(sampler));
    }

    Float pdf(const vec3& wi, const vec3& wo)
    {
        vec3 wh = glm::normalize(wi + wo);
        return pdf(wh) / (4. * glm::dot(wo, wh));
    }

    Spectrum eval(vec3 wi, vec3 wo)
    {
        vec3 wh = glm::normalize(wi + wo);

        Float d = D(wh);
        Float g = G1(wi) * G1(wo);
        Spectrum f = Spectrum(1);
        Spectrum brdf = d * g * f / (4.f * glm::clamp(wi[2], 0.0001f, 0.9999f) * glm::clamp(wo[2], 0.0001f, 0.9999f));
        return brdf * glm::clamp(wo[2], 0.0001f, 0.9999f);
    }
};

class SphereMicrosurface {
public:
    static Float D(const vec3& wh) { return 1. / pi; }
    static Float pdf(const vec3& wh)
    {
        return square_to_cosine_hemisphere_pdf(wh);
    }
    static vec3 sample_D(Sampler& sampler)
    {
        return square_to_cosine_hemisphere(sampler.next_float(),
            sampler.next_float());
    }
};

class GGXMicrosurface : public ShapeInvariantMicrosurface<SphereMicrosurface> {
public:
    GGXMicrosurface()
        : ShapeInvariantMicrosurface<SphereMicrosurface>("GGXMicrosurface", 0.1,
            0.1)
    {
        link_params();
    }

    GGXMicrosurface(const Float& scale_x, const Float& scale_y)
        : ShapeInvariantMicrosurface<SphereMicrosurface>("GGXMicrosurface",
            scale_x, scale_y)
    {
        link_params();
    }

    Float sigma(const vec3& wi)
    {
        Float rough_i = scale_wi(wi);
        return lambda(wi, rough_i) * wi.z;
    }

    Float lambda(const vec3& wi, const float& alpha)
    {
        Float cos_sqr = wi.z * wi.z;
        Float tan_sqr = (1. - cos_sqr) / cos_sqr;
        Float inv_a_sqr = (alpha * alpha * tan_sqr);
        return (-1. + std::sqrt(1. + inv_a_sqr)) / 2.;
    }

    Float G1(const vec3& wi)
    {
        Float rough_i = scale_wi(wi);
        return 1. / (1. + lambda(wi, rough_i));
    }

protected:
    void link_params()
    {
        params.add("roughx", Params::Type::FLOAT, &scale[0]);
        params.add("roughy", Params::Type::FLOAT, &scale[1]);
    }
};

} // namespace LT_NAMESPACE