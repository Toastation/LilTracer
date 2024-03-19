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
    virtual Spectrum eval(vec3 wi, vec3 wo);

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

    virtual bool emissive();
    virtual Spectrum emission();
};

class Emissive : public Brdf {
public:
    PARAMETER(Spectrum, intensity, 1.); /**< Intensity of emission. */

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

    Spectrum eval(vec3 wi, vec3 wo);

protected:
    void link_params() { params.add("albedo", Params::Type::VEC3, &albedo); }
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

    Spectrum eval(vec3 wi, vec3 wo);

protected:
    void link_params()
    {
        params.add("float", Params::Type::FLOAT, &v1);
        params.add("vec3", Params::Type::VEC3, &v2);
        params.add("array", Params::Type::SH, &v3);
    }
};

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

    Float scale_wi(const vec3& wi) const;
    vec3 to_transformed_space(const vec3& wi);
    vec3 to_unit_space(const vec3& wi);

    Float D(const vec3& wh);
    Float pdf(const vec3& wh);
    Float pdf(const vec3& wi, const vec3& wo);

    vec3 sample_D(Sampler& sampler);
    vec3 sample(const vec3& wi, Sampler& sampler);

    Spectrum eval(vec3 wi, vec3 wo);

    MICROSURFACE ms;
};

class SphereMicrosurface {
public:
    Float D(const vec3& wh) { return 1. / pi; }
    Float pdf(const vec3& wh)
    {
        return square_to_cosine_hemisphere_pdf(wh);
    }
    vec3 sample_D(Sampler& sampler)
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

    Float sigma(const vec3& wi);
    Float lambda(const vec3& wi, const float& alpha);
    Float G1(const vec3& wi);

protected:
    void link_params()
    {
        params.add("rough_x", Params::Type::FLOAT, &scale[0]);
        params.add("rough_y", Params::Type::FLOAT, &scale[1]);
    }
};

} // namespace LT_NAMESPACE